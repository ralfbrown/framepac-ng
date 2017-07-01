/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-30					*/
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

#include <iostream>
#include <cstdlib>
#include "framepac/memory.h"
#include "framepac/semaphore.h"
using namespace std ;

namespace FramepaC
{

// with the default 4K per slab and 4K slabs per group, this collection size
//   permits just under 1TB of allocations before we need to do a compaction
#define COLL_SIZE 65535

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
typedef void SlabGroupIndexFn(SlabGroup*, size_t index) ;

class SlabGroupColl
   {
   public:
      SlabGroupColl() = default ;
      SlabGroupColl(SlabGroupIndexFn* idx, SlabGroupReleaseFn* rel)
	 : m_setindexfunc(idx), m_releasefunc(rel)
	 {}
      ~SlabGroupColl() = default ;

      std::pair<SlabGroup*,size_t> accessAny(unsigned hint = 0) ;
      SlabGroup* access(size_t) ;
      bool release(size_t) ;

      size_t append(SlabGroup*) ;
      bool markReleaseable(size_t idx)
	 {
	    return (m_groups[idx].fetch_or(RELEASE_MASK) & RELEASE_MASK) != 0 ;
	 }
      void requestRemoval(size_t) ;
      bool compact() ;

      void setReleaseFunc(SlabGroupReleaseFn* fn) { m_releasefunc = fn  ; }
      void setIndexFunc(SlabGroupIndexFn* fn) { m_setindexfunc = fn  ; }

      bool lockedEntry(size_t index) { return index < COLL_SIZE ? no_more_updates(m_groups[index]) : true ; }

   protected: // constants
      // to keep things atomic and save space, we'll pack additional info into otherwise-unused bits of the
      //   SlabGroup pointer
      // the low K bits are always 0, since SlabGroups are aligned to the size of a Slab (12 bits for the
      //   default 4K, 13 for 8K, etc.), so we can use them for the reference count
      static constexpr uintptr_t COUNT_MASK = (SLAB_SIZE-1) ;
      // x86_64 currently only has 48-bit virtual addresses, so we can steal bits 62 and 63 as flags
      static constexpr uintptr_t RELEASE_MASK = (1UL << 63) ;
      static constexpr uintptr_t COMPACT_MASK = (1UL << 62) ;
      // the remaining bits are the original pointer bits
      static constexpr uintptr_t POINTER_MASK = ~(RELEASE_MASK|COMPACT_MASK|COUNT_MASK) ;

   protected: // functions
      static SlabGroup* pointer(SlabGroup* ptr)
	 { return reinterpret_cast<SlabGroup*>(((uintptr_t)ptr) & POINTER_MASK) ; }
      static SlabGroup* pointer(uintptr_t ptr)
	 { return reinterpret_cast<SlabGroup*>(ptr & POINTER_MASK) ; }
      static unsigned refcount(SlabGroup* ptr)
	 { return unsigned(((uintptr_t)ptr) & COUNT_MASK) ; }
      static unsigned refcount(uintptr_t ptr)
	 { return unsigned(ptr & COUNT_MASK) ; }
      static bool no_more_updates(uintptr_t ptr)
	 { return (ptr & (RELEASE_MASK|COMPACT_MASK)) != 0 ; }

      uintptr_t incrRefCount(size_t idx)
	 {
	    return m_groups[idx]++ ;
	 }
      uintptr_t decrRefCount(size_t idx)
	 {
	    return m_groups[idx]-- ;
	 }
      bool markCompacting(size_t idx)
	 {
	    return (m_groups[idx].fetch_or(COMPACT_MASK) & COMPACT_MASK) != 0 ;
	 }
      unsigned leastBusy(unsigned first) const ;
      
   protected: // data members
      Fr::Atomic<unsigned>  m_first ;		// first array element in use
      Fr::Semaphore         m_sem ;
      Fr::Atomic<uintptr_t> m_groups[COLL_SIZE+1] ;
      Fr::Atomic<unsigned>  m_lockcount ;
      SlabGroupIndexFn*     m_setindexfunc ;
      SlabGroupReleaseFn*   m_releasefunc ;
      Fr::Atomic<unsigned>  m_last ;		// last+1 array element in use
   } ;

/************************************************************************/
/*	methods for class SlabGroupColl					*/
/************************************************************************/

unsigned SlabGroupColl::leastBusy(unsigned first) const
{
   // scan a small number of active entries, looking for one which is not in use and remembering
   //   which one has the lowest reference count
   size_t minrefs = ~0 ;
   size_t bestidx = COLL_SIZE ;
   for (size_t i = 0 ; (i < 160000 || bestidx == COLL_SIZE) && first+i < m_last ; ++i)
      {
      uintptr_t val = m_groups[first+i] ;
      if (no_more_updates(val)) continue ;
      size_t refs = refcount(val) ;
      if (refs == 0)
 	 {
	 return first+i ;
	 }
      if (refs < minrefs)
	 {
	 minrefs = refs ;
	 bestidx = first+i ;
	 }
      }
   return bestidx ;
}

//----------------------------------------------------------------------------

// find an available SlabGroup from the collection
std::pair<SlabGroup*,size_t> SlabGroupColl::accessAny(unsigned hint)
{
   for (unsigned tries = 0 ; tries < 3 && m_first < m_last ; ++tries)
      {
      // advance the 'first' pointer past any entries which have been or are being moved/removed
      unsigned first = m_first ;
      while (no_more_updates(m_groups[first]) && m_first.compare_exchange_strong(first,first+1))
	 {
	 ++first ;
	 }
      if (hint > first) first = hint ;
      unsigned bestidx = leastBusy(first) ;
      while (bestidx < COLL_SIZE)
	 {
	 // grab the selected entry and verify that it's still valid
	 SlabGroup* grp = access(bestidx) ;
	 if (grp) return std::pair<SlabGroup*,size_t>(grp,bestidx) ;
	 bestidx = leastBusy(first + (bestidx-first)/2) ;
	 }
      }
   return std::pair<SlabGroup*,size_t>(nullptr,~0U) ;
}

//----------------------------------------------------------------------------

SlabGroup* SlabGroupColl::access(size_t idx)
{
   if (idx >= COLL_SIZE)
      return nullptr ;
   uintptr_t val = incrRefCount(idx) ;
   if (!no_more_updates(val))
      {
      SlabGroup* grp = pointer(val) ;
      if (grp)
	 return grp ;
      }
   release(idx) ;
   return nullptr ;
}

//----------------------------------------------------------------------------

bool SlabGroupColl::release(size_t idx)
{
   if (idx >= COLL_SIZE)
      return false ;
   uintptr_t val = decrRefCount(idx) ;
   if ((val & (COUNT_MASK | RELEASE_MASK)) != (RELEASE_MASK | 1))
      return false ;
   // we were the last thread accessing the entry, and it's been
   //   flagged for removal, so perform that removal
   val = m_groups[idx].exchange(RELEASE_MASK) ;
   if ((val & POINTER_MASK) == POINTER_MASK){cerr<<"release fail\n"<<flush; return false ;}//!!!
   SlabGroup* grp = pointer(val) ;
   if (grp && m_releasefunc)
      {
      m_releasefunc(grp) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

size_t SlabGroupColl::append(SlabGroup* grp)
{
   if (!grp) return ~0UL ;
   size_t idx = m_last++ ;		// reserve a slot
   if (idx >= COLL_SIZE)
      {
      if (compact())			// defragment the collection
	 return append(grp) ;		// and retry
      // if we get here, the compaction failed because every entry was still active, which means we can't
      //   append, so we'll have to throw an error
      throw new alloc_capacity_error ;
      }
   m_groups[idx] = uintptr_t(grp) ;
   return idx ;
}

//----------------------------------------------------------------------------

void SlabGroupColl::requestRemoval(size_t idx)
{
   if (idx >= COLL_SIZE)
      return ;
   incrRefCount(idx) ;			// mark the entry as in-use
   markReleaseable(idx) ;		// set the removal flag
   release(idx) ;			// we're done; the last thread using the entry will do the removal
   return ;
}
//----------------------------------------------------------------------------

bool SlabGroupColl::compact()
{
   abort() ; //FIXME!!!  this function needs completion and debugging
   if (m_lockcount++ == 0)
      {
      // we're the first to grab the lock, so we'll do the actual compaction
      // every entry prior to m_first has at least started the removal process; zero out
      //   all that have actually completed
      size_t first = m_first ;
      size_t last = m_last ;
      for (size_t i = 0 ; i < first ; ++i)
	 {
	 if (m_groups[i] == RELEASE_MASK)
	    m_groups[i] = 0 ;
	 }
      size_t dest = 0 ;
      size_t highest_used = ~0UL ;
      for (size_t i = m_first ; i < last ; ++i)
	 {
	 if (i >= m_first) m_first = i+1 ; // advance m_first if nobody else has yet
	 // check the state of the current entry
	 uintptr_t val = m_groups[i] ;
	 if (val == RELEASE_MASK)
	    {
	    // fully-released entry, so just zero it out
	    m_groups[i] = 0 ;
	    }
	 else if ((val & (RELEASE_MASK | COUNT_MASK)) != 0)
	    {
	    // busy (either in use or flagged for release but release still in progress): leave it alone for now
	    highest_used = i ;
	    }
	 else // if ((val & (RELEASE_MASK | COUNT_MASK)) == 0)
	    {
	    // nobody is using the entry, so we can move it
	    // first, we need to show that it's in use and set a flag to prevent any further accesses
	    incrRefCount(i) ;
	    markCompacting(i) ;
	    if (refcount(decrRefCount(i)) == 1)
	       {
	       // we're the only thread accessing the entry, so it's safe to move now
	       m_groups[dest] = val ;
	       m_setindexfunc(pointer(val),dest) ;
	       m_groups[i] = 0 ;
	       ++dest ;
	       }
	    }
	 }
      // second pass to clean up any entries which became busy while we were trying to move them
      for (size_t i = 0 ; i < last ; ++i)
	 {
//FIXME
	 }
      // update the first/last pointers to encompass the new range of entries which are in use
      m_last = std::max(dest,highest_used+1) ;
      m_first = 0 ;
      }
   else
      {
      // compaction was already in progress, so wait
      m_sem.wait() ;
      }
   bool compacted = m_last < COLL_SIZE ;
   // done, so release the lock
   if (m_lockcount-- > 1)
      {
      // others were waiting, so wake up the next waiter
      m_sem.post() ;
      }
   return compacted ;
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
   for (size_t i = 0 ; i < SLAB_GROUP_SIZE-1 ; ++i)
      {
      m_slabs[i].setNextFreeSlab(next) ;
      m_slabs[i].setPrevFreeSlabPtr(nullptr) ;
      m_slabs[i].setSlabID(i) ;
      next = &m_slabs[i] ;
      }
   m_freeslabs = next ;
   m_numfree = SLAB_GROUP_SIZE-1 ;
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

void SlabGroup::pushGroup()
{
#ifdef FrMEMALLOC_STATS
   m_groupindex = s_groupcoll.append(this) ;
#endif /* FrMEMALLOC_STATS */
   return;
}

//----------------------------------------------------------------------------

void SlabGroup::unlinkGroup()
{
#ifdef FrMEMALLOC_STATS
   s_groupcoll.requestRemoval(m_groupindex) ;
#endif /* FrMEMALLOC_STATS */
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::unlinkFreeGroup()
{
   s_freecoll.requestRemoval(m_freeindex) ;
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::pushFreeGroup()
{
   m_freeindex = s_freecoll.append(this) ;
   return ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::popFreeSlab(unsigned& hint)
{
   auto sg_info = s_freecoll.accessAny(hint) ;
   auto sg = sg_info.first ;
   auto index = sg_info.second ;
   if (!sg) return nullptr ;
   hint = index ;
   // allocate an available Slab from the group's freelist
   Slab* slb = sg->m_freeslabs.load() ;
   Slab* next ;
   do {
      if (!slb) break ;
      next = slb->nextFreeSlab() ;
      } while (!sg->m_freeslabs.compare_exchange_weak(slb,next)) ;
   if (slb)
      {
      sg->m_numfree-- ;		     // update free count
      }
   if (!sg->m_freeslabs)
      {
      // request removal of the slabgroup from the list of groups with free slabs
      s_freecoll.markReleaseable(index) ;
      }
   s_freecoll.release(index) ;
   return slb ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::allocateSlab()
{
   unsigned hint = 0 ;
   Slab* slb = popFreeSlab(hint) ;
   if (!slb)
      slb = popFreeSlab(hint) ; // retry once just in case another thread has already allocated a SlabGroup
   if (!slb)
      {
      // we were unable to get any existing free slab, so allocate a
      //   new group, and reserve one of its slabs for ourself before
      //   adding it to the pool
      SlabGroup* sg = new SlabGroup ;
      if (sg)
	 {
	 // pop the first element of the freelist
	 slb = sg->m_freeslabs.load() ;
	 sg->m_freeslabs = slb->nextFreeSlab() ;
	 sg->m_numfree-- ;
	 // add the new group to the collection of all SlabGroups
	 sg->pushGroup() ;
	 // add the group to the collection of groups with free Slabs
	 sg->pushFreeGroup() ;
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
   unsigned freecount = ++sg->m_numfree ;
   if (freecount == lengthof(m_slabs))
      {
      // this group is now completely unused, so return it to the operating system
      sg->unlinkGroup() ;
      sg->unlinkFreeGroup() ;
      return ;
      }
   else if (freecount == 1)
      {
      // first free slab added to this group, so link it into the list of groups with free slabs
      sg->pushFreeGroup() ;
      }
   return ;
}

/************************************************************************/
/*	Utility functions for SlabGroupColl				*/
/************************************************************************/

#ifdef FrMEMALLOC_STATS
static void group_setindex(SlabGroup* sg, size_t idx)
{
   sg->setGroupIndex(idx) ;
   return ;
}
#endif /* FrMEMALLOC_STATS */

//----------------------------------------------------------------------------

static void freelist_setindex(SlabGroup* sg, size_t idx)
{
   sg->setFreecollIndex(idx) ;
   return ;
}

//----------------------------------------------------------------------------

static void freelist_release(SlabGroup* sg)
{
   // we get called when a scheduled removal from s_freecoll becomes final; the removal was
   //   scheduled because at that time the slab was either entirely used or entirely free
   // Three cases can happen now:
   // 1. the group is completely used: remove from free list (already done), but do nothing else
   // 2. the group is completely free: return to OS
   // 3. the group is partially used: somebody modified it since the removal was scheduled, so put it back
   //    on the freelist
   size_t numfree = sg->freeSlabs() ;
   if (numfree == 0)
      {
      // case 1: do nothing
      }
   else if (numfree == SLAB_GROUP_SIZE)
      {
      // case 2: return to OS
      sg->_delete() ;
      }
   else
      {
      // case 3: put back on the free list
      sg->pushFreeGroup() ;
      }
   return ;
}

/************************************************************************/
/*	Static member variables						*/
/************************************************************************/

SlabGroup* SlabGroup::s_grouplist { nullptr } ;
SlabGroup* SlabGroup::s_freelist { nullptr } ;
SlabGroupColl SlabGroup::s_freecoll(freelist_setindex,freelist_release) ;
#ifdef FrMEMALLOC_STATS
SlabGroupColl SlabGroup::s_groupcoll(group_setindex,nullptr) ;
#endif /* FrMEMALLOC_STATS */

mutex SlabGroup::s_grouplist_mutex ;
mutex SlabGroup::s_freelist_mutex ;

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slabgroup.C //
