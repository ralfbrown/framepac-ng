/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-06					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
/*	This program may be redistributed and/or modified under the	*/
/*	terms of the GNU General Public License, version 3, or an	*/
/*	alternative license agreement as detailed in the accompanying	*/
/*	file LICENSE.  You should also have received a copy of the	*/
/*	GPL (file COPYING) along with this program.  If not, see	*/
/*	http://www.gnu.org/licenses/					*/
/*									*/
/*	This program is distributed in the hope that it will be		*/
/*	useful, but WITHOUT ANY WARRANTY; without even the implied	*/
/*	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR		*/
/*	PURPOSE.  See the GNU General Public License for more details.	*/
/*									*/
/************************************************************************/

#include <cstdlib>
#include "framepac/memory.h"
#include "framepac/semaphore.h"
using namespace std ;

#define LOCKFREE

namespace FramepaC
{

// with the default 4K per slab and 4K slabs per group, this collection size
//   permits just under 1TB of total memory allocations
#define COLL_SIZE 65536

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

class alloc_capacity_error : public std::bad_alloc
   {
   public:
      alloc_capacity_error() = default ;
      virtual const char* what() const noexcept { return "Allocator memory capacity exceeded" ; }
   } ;

//----------------------------------------------------------------------------

typedef void SlabGroupReleaseFn(SlabGroup*) ;

// implement a Vyukov-style bounded MPMC queue
//  see http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

class SlabGroupColl
   {
   public:
      SlabGroupColl()
	 {
	 static_assert((COLL_SIZE & (COLL_SIZE-1)) == 0,"COLL_SIZE must be a power of two") ;
	 for (size_t i = 0 ; i < COLL_SIZE ; ++i)
	    m_entries[i].m_seqnum.store(i) ;
	 m_headseq.store(0) ;
	 m_tailseq.store(0) ;
	 m_mask = COLL_SIZE - 1 ;
	 }
      ~SlabGroupColl() {}

      bool append(SlabGroup* grp) ;
      bool pop(SlabGroup*& grp) ;

   protected: // subtypes
      class Entry
	 {
	 public:
	    Fr::Atomic<uint64_t> m_seqnum ;
	    SlabGroup*           m_group ;
	 } ;

   protected: // data members
      Entry                m_entries[COLL_SIZE] ;
      alignas(64)
      Fr::Atomic<uint64_t> m_headseq ;
      alignas(64)
      Fr::Atomic<uint64_t> m_tailseq ;
      alignas(64)
      uint64_t             m_mask ;
      SlabGroupReleaseFn*   m_releasefunc ;
   } ;

/************************************************************************/
/*	Static member variables						*/
/************************************************************************/

SlabGroupColl SlabGroup::s_freecoll ;

std::mutex slab_mutex ;

/************************************************************************/
/*	methods for class SlabGroupColl					*/
/************************************************************************/

bool SlabGroupColl::append(SlabGroup* grp)
{
   uint64_t pos ;
   size_t maskedpos ;
   for (pos = m_headseq.load_relax() ; ; pos = m_headseq.load_relax())
      {
      maskedpos = pos & m_mask ;
      uint64_t prevseq = m_entries[maskedpos].m_seqnum.load() ;
      if (pos == prevseq)
	 {
	 if (m_headseq.compare_exchange_weak(pos, pos+1))
	    break ;
	 }
      else if (pos > prevseq)
	 return false ;
      }
   m_entries[maskedpos].m_group = grp ;
   m_entries[maskedpos].m_seqnum.store(pos+1) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SlabGroupColl::pop(SlabGroup*& grp)
{
   uint64_t pos ;
   size_t maskedpos ;
   for (pos = m_tailseq.load_relax() ; ; pos = m_tailseq.load_relax())
      {
      maskedpos = pos & m_mask ;
      uint64_t prevseq = m_entries[maskedpos].m_seqnum.load() ;
      if (prevseq == pos + 1)
	 {
	 if (m_tailseq.compare_exchange_weak(pos, pos+1))
	    break ;
	 }
      else if (prevseq < pos + 1)
	 return false ;
      }
   grp = m_entries[maskedpos].m_group ;
   m_entries[maskedpos].m_seqnum.store(pos + m_mask + 1) ;
   return true ;
}

/************************************************************************/
/************************************************************************/

// -Weffc++ warns about not setting m_next in the initializer list, but we can't safely
//   set it until we've locked the mutex in the ctor's body....
#pragma GCC diagnostic ignored "-Weffc++"

SlabGroup::SlabGroup()
{
   // set up the linked list of free slabs
#ifdef LOCKFREE
   uint16_t next = NULLPTR ;
#else
   Slab* next = nullptr ;
#endif
   for (size_t i = 0 ; i < SLAB_GROUP_SIZE ; ++i)
      {
      m_slabs[i].setVMT(nullptr) ;
      m_slabs[i].setSlabID(i) ;
#ifdef LOCKFREE
      m_slabs[i].setNextFreeSlab(nullptr) ;
      m_slabs[i].m_info.m_nextfree_id = next ;
      next = i ;
#else
      m_slabs[i].setNextFreeSlab(next) ;
      next = &m_slabs[i] ;
#endif
      }
#ifdef LOCKFREE
   FreeInfo freeinfo ;
   freeinfo.m_index = next ;
   freeinfo.m_numfree = SLAB_GROUP_SIZE ;
   m_freeinfo.store(freeinfo) ;
   m_numfree = 0 ;
#else
   m_freeslabs = next ;
   m_numfree = SLAB_GROUP_SIZE ;
#endif
   return ;
}

//----------------------------------------------------------------------------

void* SlabGroup::operator new(size_t sz)
{
   void* alloc ;
   return (posix_memalign(&alloc,sizeof(Slab),sz) ) ? nullptr : alloc ;
}

//----------------------------------------------------------------------------

void SlabGroup::operator delete(void* grp)
{
   free(grp) ; 
   return ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::popFreeSlab()
{
   SlabGroup* sg ;
   if (!s_freecoll.pop(sg)) return nullptr ;
   // is the group we got completely unused?
#ifdef LOCKFREE
   if (sg->m_freeinfo.load().m_numfree == SLAB_GROUP_SIZE)
#else
   if (sg->freeSlabs() == SLAB_GROUP_SIZE)
#endif
      {
      // check whether there are additional groups with free slabs; if so, return the completely free group
      //   to the operating system
      SlabGroup* sg2 ;
      if (s_freecoll.pop(sg2))
	 {
	 sg->_delete() ;
	 sg = sg2 ;
	 }
      }
   // allocate an available Slab from the group's freelist
   // although we have exclusive 'pop' access, another thread could sneak in and free a slab, so we have to
   //   properly synchronize anyway
#ifdef LOCKFREE
   FreeInfo freeinfo { sg->m_freeinfo.load() } ;
   FreeInfo nextinfo ;
   Slab* slb ;
   slab_mutex.lock() ;
   do {
      slb = &sg->m_slabs[freeinfo.m_index] ;
      nextinfo.m_numfree = freeinfo.m_numfree - 1 ;
      nextinfo.m_index = slb->m_info.m_nextfree_id ;
      } while (!sg->m_freeinfo.compare_exchange_weak(freeinfo,nextinfo)) ;
   if (freeinfo.m_numfree > 1)
      {
      // there are still free slabs in the group, so put it back on the queue of groups with free slabs
      s_freecoll.append(sg) ;
      }
   slab_mutex.unlock() ;
#else
   slab_mutex.lock() ;
   sg->m_numfree-- ;		     // update free count
   Slab* slb = sg->m_freeslabs ;
   Slab* next ;
   do {
      next = slb->nextFreeSlab() ;
      } while (!sg->m_freeslabs.compare_exchange_weak(slb,next)) ;
   if (next)
      {
      // there are still free slabs in the group, so put it back on the queue of groups with free slabs
      s_freecoll.append(sg) ;
      }
   slab_mutex.unlock() ;
#endif
   return slb ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::allocateSlab()
{
   Slab* slb = popFreeSlab() ;
   if (!slb)
      slb = popFreeSlab() ; // retry once just in case another thread has already allocated a SlabGroup
   if (!slb)
      {
      // we were unable to get any existing free slab, so allocate a
      //   new group, and reserve one of its slabs for ourself before
      //   adding it to the pool
      SlabGroup* sg = new SlabGroup ;
      if (sg)
	 {
#ifdef LOCKFREE
	 FreeInfo freeinfo { sg->m_freeinfo.load() } ;
	 FreeInfo nextinfo ;
	 slb = &sg->m_slabs[freeinfo.m_index] ;
	 nextinfo.m_numfree = freeinfo.m_numfree - 1 ;
	 nextinfo.m_index = slb->m_info.m_nextfree_id ;
	 sg->m_freeinfo.store(nextinfo) ;
#else
	 // pop the first element off the freelist
	 slb = sg->m_freeslabs ;
	 sg->m_freeslabs = slb->nextFreeSlab() ;
	 sg->m_numfree-- ;
#endif
	 // add the group to the collection of groups with free Slabs
	 s_freecoll.append(sg) ;
	 }
      }      
   return slb ;
}

//----------------------------------------------------------------------------

void SlabGroup::releaseSlab(Slab* slb)
{
   SlabGroup* sg = slb->containingGroup() ;
#ifndef FrSINGLE_THREADED
   slb->clearOwner() ;
#endif /* !FrSINGLE_THREADED */
   // add slab to list of free slabs in its group
#ifdef LOCKFREE
   FreeInfo freeinfo { sg->m_freeinfo.load() } ;
   FreeInfo nextinfo ;
   nextinfo.m_index = slb->m_info.m_slab_id ;
   slab_mutex.lock() ;
   do {
      slb->m_info.m_nextfree_id = freeinfo.m_index ;
      nextinfo.m_numfree = freeinfo.m_numfree + 1 ;
      } while (!sg->m_freeinfo.compare_exchange_weak(freeinfo,nextinfo)) ;
   if (freeinfo.m_numfree == 0)
      {
      // first free slab added to this group, so link it into the list of groups with free slabs
      s_freecoll.append(sg) ;
      }
   slab_mutex.unlock() ;
#else
   slab_mutex.lock() ;
   Slab* freelist = sg->m_freeslabs ;
   do {
      slb->setNextFreeSlab(freelist) ;
      } while (!sg->m_freeslabs.compare_exchange_weak(freelist,slb)) ;
   // update statistics
   sg->m_numfree++ ;
   if (!freelist)
      {
      // first free slab added to this group, so link it into the list of groups with free slabs
      s_freecoll.append(sg) ;
      }
   slab_mutex.unlock() ;
#endif
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::reclaim()
{
   SlabGroup* sg ;
   if (!s_freecoll.pop(sg)) return ;
   s_freecoll.append(sg) ;
   SlabGroup* sg2 ;
   s_freecoll.pop(sg2) ;
   while (sg2 != sg)
      {
#ifdef LOCKFREE
      if (sg2->m_freeinfo.load().m_numfree == SLAB_GROUP_SIZE)
#else
      if (sg2->freeSlabs() == SLAB_GROUP_SIZE)
#endif
	 {
	 sg2->_delete() ;
	 }
      s_freecoll.pop(sg2) ;
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slabgroup.C //
