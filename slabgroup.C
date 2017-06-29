/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-26					*/
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

class SlabGroupColl
   {
   public:
      SlabGroupColl() = default ;
      ~SlabGroupColl() = default ;

      std::pair<SlabGroup*,size_t> accessAny() ;
      SlabGroup* access(size_t) ;
      void release(size_t) ;

      void append(SlabGroup*) ;
      void requestRemoval(size_t) ;
      bool compact() ;

      void setReleaseFunc(SlabGroupReleaseFn* fn) { m_releasefunc = fn  ; }

   protected: // constants
      // to keep things atomic and save space, we'll pack additional info into otherwise-unused bits of the
      //   SlabGroup pointer
      // the low K bits are always 0, since SlabGroups are aligned to the size of a Slab (12 bits for the
      //   default 4K, 13 for 8K, etc.), so we can use them for the reference count
      static constexpr uintptr_t COUNT_MASK = (SLAB_SIZE-1) ;
      // x86_64 currently only has 48-bit virtual addresses, so we can steal bit 63 as a flag
      static constexpr uintptr_t RELEASE_MASK = (1UL << 63) ;
      // the remaining bits are the original pointer bits
      static constexpr uintptr_t POINTER_MASK = (~0UL & ~(RELEASE_MASK|COUNT_MASK)) ;

   protected: // functions
      static SlabGroup* pointer(SlabGroup* ptr)
	 { return reinterpret_cast<SlabGroup*>(((uintptr_t)ptr) & POINTER_MASK) ; }
      static SlabGroup* pointer(uintptr_t ptr)
	 { return reinterpret_cast<SlabGroup*>(ptr & POINTER_MASK) ; }
      static unsigned refcount(SlabGroup* ptr)
	 { return unsigned(((uintptr_t)ptr) & COUNT_MASK) ; }
      static unsigned refcount(uintptr_t ptr)
	 { return unsigned(ptr & COUNT_MASK) ; }
      static bool flagged(SlabGroup* ptr)
	 { return (((uintptr_t)ptr) & RELEASE_MASK) != 0 ; }
      static bool flagged(uintptr_t ptr)
	 { return (ptr & RELEASE_MASK) != 0 ; }

      uintptr_t incrRefCount(size_t idx)
	 {
	    return m_groups[idx]++ ;
	 }
      uintptr_t decrRefCount(size_t idx)
	 {
	    return m_groups[idx]-- ;
	 }
      bool setFlag(size_t idx)
	 {
	    return (m_groups[idx].fetch_or(RELEASE_MASK) & RELEASE_MASK) != 0 ;
	 }
   protected: // data members
      Fr::Atomic<unsigned>  m_first ;		// first array element in use
      Fr::Semaphore         m_sem ;
      Fr::Atomic<uintptr_t> m_groups[COLL_SIZE+1] ;
      Fr::Atomic<unsigned>  m_lockcount ;
      SlabGroupReleaseFn*   m_releasefunc ;
      Fr::Atomic<unsigned>  m_last ;		// last+1 array element in use
   } ;

/************************************************************************/
/************************************************************************/

SlabGroup* SlabGroup::s_grouplist { nullptr } ;
SlabGroup* SlabGroup::s_freelist { nullptr } ;
SlabGroupColl SlabGroup::s_groupcoll ;
SlabGroupColl SlabGroup::s_freecoll ;

mutex SlabGroup::s_grouplist_mutex ;
mutex SlabGroup::s_freelist_mutex ;

/************************************************************************/
/*	methods for class SlabGroupColl					*/
/************************************************************************/

// find an available SlabGroup from the collection
std::pair<SlabGroup*,size_t> SlabGroupColl::accessAny()
{
   // advance the 'first' pointer past any entries which have been or are being removed
   unsigned first = m_first ;
   while (flagged(m_groups[first]) && m_first.compare_exchange_strong(first,first+1))
      {
      ++first ;
      }
   // scan a small number of active entries, looking for one which is not in use and remembering
   //   which one has the lowest reference count
   size_t minrefs = ~0 ;
   size_t bestidx = COLL_SIZE ;
   for (size_t i = 0 ; i < 8 && first+i < m_last ; ++i)
      {
      uintptr_t val = m_groups[first+i] ;
      if (flagged(val)) continue ;
      size_t refs = refcount(val) ;
      if (refs == 0)
	 {
	 bestidx = first+i ;
	 break ;
	 }
      if (refs < minrefs)
	 {
	 minrefs = refs ;
	 bestidx = first+i ;
	 }
      }
   // grab the selected entry and verify that it's still valid
   SlabGroup* grp = access(bestidx) ;
   if (!grp)
      {
      if (m_first < m_last)
	 return accessAny() ;
      bestidx = ~0 ;
      }
   return std::pair<SlabGroup*,size_t>(grp,bestidx) ;
}

//----------------------------------------------------------------------------

SlabGroup* SlabGroupColl::access(size_t idx)
{
   if (idx >= COLL_SIZE)
      return nullptr ;
   uintptr_t val = incrRefCount(idx) ;
   if (!flagged(val))
      {
      SlabGroup* grp = pointer(val) ;
      if (grp)
	 return grp ;
      }
   release(idx) ;
   return nullptr ;
}

//----------------------------------------------------------------------------

void SlabGroupColl::release(size_t idx)
{
   if (idx >= COLL_SIZE)
      return ;
   uintptr_t val = decrRefCount(idx) ;
   if ((val & (COUNT_MASK | RELEASE_MASK)) == (RELEASE_MASK | 1))
      {
      // we were the last thread accessing the entry, and it's been
      //   flagged for removal, so perform that removal
      val = m_groups[idx].exchange(RELEASE_MASK) ;
      SlabGroup* grp = pointer(val) ;
      if (grp && m_releasefunc)
	 {
	 m_releasefunc(grp) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void SlabGroupColl::append(SlabGroup* grp)
{
   if (!grp) return ;
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
   return ;
}

//----------------------------------------------------------------------------

void SlabGroupColl::requestRemoval(size_t idx)
{
   if (idx >= COLL_SIZE)
      return ;
   incrRefCount(idx) ;			// mark the entry as in-use
   setFlag(idx) ;			// set the removal flag
   release(idx) ;			// we're done; the last thread using the entry will do the removal
   return ;
}
//----------------------------------------------------------------------------

bool SlabGroupColl::compact()
{
   if (m_lockcount++ == 0)
      {
      // we're the first to grab the lock, so we'll do the actual compaction
//FIXME
      
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
   : m_mutex()
{
   // set up the linked list of free slabs
   m_slabs[0].setNextSlab(nullptr) ;
   m_slabs[0].setSlabID(0) ;
   for (size_t i = 1 ; i < lengthof(m_slabs) ; ++i)
      {
      m_slabs[i].setNextSlab(&m_slabs[i-1]) ;
      m_slabs[i].setSlabID(i) ;
      }
   m_freeslabs = &m_slabs[SLAB_GROUP_SIZE-1] ;
   // link the new group into the doubly-linked list of SlabGroups
   pushGroup() ;
   // and into the doubly-linked list of groups with free Slabs
   pushFreeGroup() ;
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
   lock_guard<mutex> _(s_grouplist_mutex) ;
   m_next = s_grouplist ;
   m_prev = &s_grouplist ;
   if (m_next)
      m_next->m_prev = &m_next ;
   s_grouplist = this ;
   return;
}

//----------------------------------------------------------------------------

void SlabGroup::unlinkGroup()
{
   unlinkFreeGroup() ;
   lock_guard<mutex> _(s_grouplist_mutex) ;
   SlabGroup** prev = m_prev ;
   if (prev)
      {
      SlabGroup* next = m_next ;
      *prev = next ;
      if (next) next->m_prev = prev ;
      }
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::unlinkFreeGroup()
{
   bool not_locked = s_freelist_mutex.try_lock() ;
   if (m_prevfree)			// is this slab on the freelist?
      {
      SlabGroup* next = m_nextfree ;
      (*m_prevfree) = next ;
      if (next)
	 next->m_prevfree = m_prevfree ;
      }
   if (not_locked)
      s_freelist_mutex.unlock() ;
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::pushFreeGroup()
{
   lock_guard<mutex> _(s_freelist_mutex) ;
   m_prevfree = &s_freelist ;
   m_nextfree = s_freelist ;
   if (m_nextfree)
      m_nextfree->m_prevfree = &this->m_nextfree ;
   s_freelist = this ;
   return ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::popFreeSlab()
{
   lock_guard<mutex> _(s_freelist_mutex) ;
   SlabGroup *sg = s_freelist ;
   if (!sg) return nullptr ;
   // allocate an available Slab from the group's freelist
   lock_guard<mutex> lock(sg->m_mutex) ;
   Slab* slb = sg->m_freeslabs ;
   sg->m_freeslabs = slb->nextSlab() ;
   if (--sg->m_numfree == 0)		// is group now completely allocated?
      {
      // remove the slabgroup from the list of groups with free slabs
      sg->unlinkFreeGroup() ;
      }
   return slb ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::allocateSlab()
{
   Slab* slb = popFreeSlab() ;
   while (!slb)
      {
      // no slabs available, so allocate a new SlabGroup
      SlabGroup* sg = new SlabGroup ;
      if (!sg)
	 return nullptr ;
      // and retry the allocation
      slb = popFreeSlab() ;
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
   sg->m_mutex.lock() ;
   // add slab to list of free slabs in its group
   slb->setNextSlab(sg->m_freeslabs) ;
   sg->m_freeslabs = slb ;
   // update statistics
   unsigned freecount = ++sg->m_numfree ;
   if (freecount == lengthof(m_slabs))
      {
      // this group is now completely unused, so return it to the operating system
      sg->unlinkGroup() ;
      sg->m_mutex.unlock() ;
      delete sg ;
      return ;
      }
   else if (freecount == 1)
      {
      // first free slab added to this group, so link it into the list of groups with free slabs
      sg->pushFreeGroup() ;
      }
   sg->m_mutex.unlock() ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slabgroup.C //
