/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-05					*/
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

#include <cassert>

#include <iostream>
#include <cstdlib>
#include "framepac/memory.h"
#include "framepac/semaphore.h"
using namespace std ;

namespace FramepaC
{

// with the default 4K per slab and 4K slabs per group, this collection size
//   permits just under 1TB of allocations before we need to do a compaction
//#define COLL_SIZE 65535
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
   Slab* next = nullptr ;
   for (size_t i = 0 ; i < SLAB_GROUP_SIZE ; ++i)
      {
      m_slabs[i].setVMT(nullptr) ;
      m_slabs[i].setNextFreeSlab(next) ;
      m_slabs[i].setPrevFreeSlabPtr(nullptr) ;
      m_slabs[i].setSlabID(i) ;
      next = &m_slabs[i] ;
      }
   m_freeslabs = next ;
   m_numfree = SLAB_GROUP_SIZE ;
//   dummy.setVMT((ObjectVMT*)0xDEADBEEF) ;
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
   if (sg->m_numfree == SLAB_GROUP_SIZE)
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
   sg->m_numfree-- ;		     // update free count
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
	 // pop the first element off the freelist
	 slb = sg->m_freeslabs ;
	 sg->m_freeslabs = slb->nextFreeSlab() ;
	 sg->m_numfree-- ;
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
   slb->setPrevFreeSlabPtr(nullptr) ;
   Slab* freelist = sg->m_freeslabs ;
   do {
      slb->setNextFreeSlab(freelist) ;
      } while (!sg->m_freeslabs.compare_exchange_weak(freelist,slb)) ;
   // update statistics
   if (sg->m_numfree++ == 0)
      {
      // first free slab added to this group, so link it into the list of groups with free slabs
      s_freecoll.append(sg) ;
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slabgroup.C //
