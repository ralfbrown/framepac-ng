/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.12, last edit 2018-09-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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

#include <iostream>

#include <cstdlib>
#include "framepac/memory.h"
#include "framepac/semaphore.h"
#include "framepac/critsect.h"
using namespace std ;

namespace FramepaC
{

#if defined(__SANITIZE_ADDRESS__) ||  defined(__SANITIZE_THREAD__)
#define COLL_SIZE (512*1024)
#else
// with the default 4K per slab and 4K slabs per group, this collection size
//   permits just under 1TB of total memory allocations
#define COLL_SIZE 65536
#endif /* __SANITIZE_ADDRESS__ || __SANITIZE_THREAD__ */

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

// this implements a Vyukov-style bounded MPMC queue
//  see http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

// TODO: replace with linked-list queue as described by Chris M. Thomason at
//    https://software.intel.com/en-us/forums/intel-moderncode-for-parallel-architectures/topic/295836
// advantage: can grow queue as needed, so no arbitrary limit on total allocations

class SlabGroupQueue
   {
   public:
      SlabGroupQueue()
	 {
	 static_assert((COLL_SIZE & (COLL_SIZE-1)) == 0,"COLL_SIZE must be a power of two") ;
	 for (size_t i = 0 ; i < COLL_SIZE ; ++i)
	    m_entries[i].m_seqnum.store(i) ;
	 m_headseq.store(0) ;
	 m_tailseq.store(0) ;
	 }
      ~SlabGroupQueue() {}

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
      static constexpr uint64_t m_mask = COLL_SIZE - 1 ;
   } ;

/************************************************************************/
/*	Static member variables						*/
/************************************************************************/

SlabGroupQueue SlabGroup::s_freequeue ;

/************************************************************************/
/*	methods for class SlabGroupQueue					*/
/************************************************************************/

static Fr::CriticalSection app_cs ;

bool SlabGroupQueue::append(SlabGroup* grp)
{
   uint64_t pos = m_headseq.load_relax() ; 
   for ( ; ; )
      {
      Entry* entry = &m_entries[pos & m_mask] ;
      uint64_t prevseq = entry->m_seqnum.load() ;
      if (prevseq == pos)
	 {
	 // although it seems like a redundant NOP, the below lock
	 //   around the compare_exchange is required to prevent the
	 //   queue from losing entries under high contention, which
	 //   results in a memory leak
	 app_cs.lock() ;
	 bool updated = m_headseq.compare_exchange_weak(pos, pos+1) ;
	 app_cs.unlock() ;
	 if (updated)
	    {
	    entry->m_group = grp ;
	    entry->m_seqnum.store(pos+1) ;
	    return true ;
	    }
	 }
      else if (prevseq < pos)
	 {
	 return false ;
	 }
      else
	 {
	 pos = m_headseq.load_relax() ;
	 }
      }
}

//----------------------------------------------------------------------------

bool SlabGroupQueue::pop(SlabGroup*& grp)
{
   for ( ; ; )
      {
      uint64_t pos = m_tailseq.load_relax() ;
      Entry* entry = &m_entries[pos & m_mask] ;
      uint64_t prevseq = entry->m_seqnum.load() ;
      if (prevseq == pos + 1)
	 {
	 if (m_tailseq.compare_exchange_weak(pos, pos+1))
	    {
	    grp = entry->m_group ;
	    entry->m_seqnum.store(pos + COLL_SIZE) ;
	    return true ;
	    }
	 }
      else if (prevseq < pos + 1)
	 {
	 grp = nullptr ;
	 return false ;
	 }
      }
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
      ASAN(ASAN_UNPOISON_MEMORY_REGION(&m_slabs[i].m_info,sizeof(m_slabs[i].m_info))) ;
      m_slabs[i].setVMT(nullptr) ;
      m_slabs[i].setOwningAllocator(~0) ;
      m_slabs[i].setSlabID(i) ;
      m_slabs[i].setNextFreeSlab(next) ;
      next = &m_slabs[i] ;
      }
   m_freeslabs = next ;
   m_numfree = SLAB_GROUP_SIZE ;
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
   if (!s_freequeue.pop(sg)) return nullptr ;
   // is the group we got completely unused?
   if (sg->freeSlabs() == SLAB_GROUP_SIZE)
      {
      // check whether there are additional groups with free slabs; if so, return the completely free group
      //   to the operating system
      SlabGroup* sg2 ;
      if (s_freequeue.pop(sg2))
	 {
	 sg->_delete() ;
	 sg = sg2 ;
	 }
      }
   // allocate an available Slab from the group's freelist
   // although we have exclusive 'pop' access, another thread could sneak in and free a slab, so we have to
   //   properly synchronize anyway
   sg->m_numfree-- ;		     // update free count
   Slab* slb = sg->m_freeslabs ;
   Slab* next ;
   do {
      next = slb->nextFreeSlab() ;
      } while (!sg->m_freeslabs.compare_exchange_weak(slb,next)) ;
   if (next)
      {
      // there are still free slabs in the group, so put it back on the queue of groups with free slabs
      s_freequeue.append(sg) ;
      }
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
	 s_freequeue.append(sg) ;
	 }
      }      
   ASAN(ASAN_POISON_MEMORY_REGION(slb->bufferStart(),slb->bufferSize())) ;
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
   Slab* freelist = sg->m_freeslabs ;
   do {
      slb->setNextFreeSlab(freelist) ;
      } while (!sg->m_freeslabs.compare_exchange_weak(freelist,slb)) ;
   ASAN(ASAN_POISON_MEMORY_REGION(slb->bufferStart(),slb->bufferSize())) ;
   // update statistics
   sg->m_numfree++ ;
   if (!freelist)
      {
      // first free slab added to this group, so link it into the list of groups with free slabs
      s_freequeue.append(sg) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::reclaim()
{
   SlabGroup* sg ;
   if (!s_freequeue.pop(sg)) return ;
   s_freequeue.append(sg) ;
   SlabGroup* sg2 ;
   s_freequeue.pop(sg2) ;
   while (sg2 != sg)
      {
      if (sg2->freeSlabs() == SLAB_GROUP_SIZE)
	 {
	 sg2->_delete() ;
	 }
      s_freequeue.pop(sg2) ;
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slabgroup.C //
