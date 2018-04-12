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

#include "framepac/atomic.h"
#include "framepac/memory.h"

using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

#define EAGER_RECLAIM

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/*	Global variables						*/
/************************************************************************/

// DO NOT provide explicit initialization to zero, as that init will occur after some
//   allocators have already been assigned IDs, which will then be re-used and cause a crash
static Atomic<unsigned> num_allocators ;

/************************************************************************/
/*	static variables for class Allocator				*/
/************************************************************************/

Allocator::ThreadInitializer Allocator::s_threadinit ;
Allocator::SharedInfo Allocator::s_shared[FramepaC::MAX_ALLOCATOR_TYPES] ;
thread_local Allocator::TLS Allocator::s_tls[FramepaC::MAX_ALLOCATOR_TYPES] ;
thread_local FramepaC::Slab* Allocator::s_local_free_slabs = nullptr ;
thread_local unsigned Allocator::s_local_free_count = 0 ;

/************************************************************************/
/************************************************************************/

class too_many_allocators : public std::bad_alloc
   {
   public:
      too_many_allocators() : bad_alloc() {}
      virtual ~too_many_allocators() = default ;
      virtual const char* what() const noexcept { return "too many instances of Fr::Allocator" ; }
   } ;

/************************************************************************/
/*	Methods for class Allocator					*/
/************************************************************************/

static unsigned find_match(Allocator::SharedInfo* info, const FramepaC::ObjectVMT* vmt,
   unsigned objsize, unsigned align)
{
   unsigned existing { num_allocators } ;
   for (unsigned i = 0 ; i < existing ; ++i)
      {
      if (info[i].m_vmt == vmt && info[i].m_objsize == objsize && info[i].m_alignment == align)
	 return i ;
      }
   // if we get here, there was no matching allocator, so generate a new one
   unsigned idx { num_allocators++ } ;
   if (idx < MAX_ALLOCATOR_TYPES)
      {
      new (&info[idx]) Allocator::SharedInfo(vmt,objsize,align) ;
      return idx ;
      }
   // uh oh, we've filled up the array of allocator types...
   throw new too_many_allocators() ;
   return ~0 ;
}

//----------------------------------------------------------------------------

Allocator::Allocator(const FramepaC::ObjectVMT* vmt, unsigned objsize)
{
   unsigned align { alignof(double) } ;
   if (objsize < align)
      align = objsize ;
   m_type = find_match(s_shared,vmt,objsize,align) ;
   return ;
}

//----------------------------------------------------------------------------

Allocator::Allocator(const FramepaC::ObjectVMT* vmt, unsigned objsize, unsigned align)
{
   m_type = find_match(s_shared,vmt,objsize,align) ;
   return ;
}

//----------------------------------------------------------------------------

void Allocator::releaseSlab(FramepaC::Slab* slb)
{
   // unlink the slab from the doubly-linked list of allocated slabs
   slb->unlinkSlab() ;
   // put the slab into the thread-local cache to avoid global allocations
   slb->setNextSlab(s_local_free_slabs) ;
   s_local_free_slabs = slb ;
   ++s_local_free_count ;
   // trim the thread-local cache if necessary
   if (s_local_free_count > FramepaC::LOCAL_SLABCACHE_HIGHWATER)
      {
      static_assert(FramepaC::LOCAL_SLABCACHE_HIGHWATER > FramepaC::LOCAL_SLABCACHE_LOWWATER,
	            "high-water must be greater than low-water for slab cache") ;
      while (s_local_free_slabs && s_local_free_count > FramepaC::LOCAL_SLABCACHE_LOWWATER)
	 {
	 slb = s_local_free_slabs ;
	 s_local_free_slabs = slb->nextSlab() ;
	 --s_local_free_count ;
	 SlabGroup::releaseSlab(slb) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void Allocator::popFreelist() 
{
   Slab* next = s_tls[m_type].m_freelist->nextFreeSlab() ;
   s_tls[m_type].m_freelist = next ;
   if (!next) return ;
   next->reclaimForeignFrees() ;
#ifdef EAGER_RECLAIM
   while (next->objectsInUse() == 0 && next->nextFreeSlab())
      {
      Slab* nextnext = next->nextFreeSlab() ;
      s_tls[m_type].m_freelist = nextnext ;
      releaseSlab(next) ;
      next = nextnext ;
      if (!next) break ;
      next->reclaimForeignFrees() ;
      }
#else
   if (next->objectsInUse() == 0 && next->nextFreeSlab())
      {
      s_tls[m_type].m_freelist = next->nextFreeSlab() ;
      releaseSlab(next) ;
      }
#endif
   return ;
}

//----------------------------------------------------------------------------

void Allocator::reclaim(uint16_t alloc_id, bool keep_one)
{
   // reclaim all foreign frees
   // first, atomically grab the list of slabs with foreign-freed objects
   Slab* freelist { Atomic<Slab*>::ref(s_tls[alloc_id].m_foreignfree).exchange(nullptr) } ;
   // then iterate through the list, accumulating the number of slabs and min/max available objects per slab
   unsigned min_used { ~0U } ;
   unsigned max_used { 0 } ;
#ifdef EAGER_RECLAIM
   // when eagerly reclaiming, reclaim even if there weren't any foreign frees
   if (!freelist) min_used = 0 ;
#endif /* EAGER_RECLAIM */
   while (freelist)
      {
      Slab* slb { freelist } ;
      freelist = freelist->nextFreeSlab() ;
      slb->reclaimForeignFrees() ;
      size_t in_use { slb->objectsInUse() } ;
      if (in_use < min_used) min_used = in_use ;
      if (in_use > max_used) max_used = in_use ;
      slb->pushFreeSlab(s_tls[alloc_id].m_freelist) ;
      }
   // if we have multiple Slabs on the freelist, release any which are completely unused back to the general pool
   Slab* slb { s_tls[alloc_id].m_freelist } ;
   if (slb && min_used == 0)
      {
      Slab* prev { nullptr } ;
      if (max_used == 0 && keep_one)
	 {
	 // leave one slab on the freelist if all are completely unused
	 prev = slb ;
	 slb = slb->nextFreeSlab() ;
	 }
      while (slb)
	 {
	 Slab* currslab { slb } ;
	 slb = slb->nextFreeSlab() ;
	 if (currslab->objectsInUse() == 0)
	    {
	    // release back to general pool
	    if (prev)
	       prev->setNextFreeSlab(slb) ;
	    else
	       s_tls[alloc_id].m_freelist = slb ;
	    releaseSlab(currslab) ;
	    }
	 else
	    prev = currslab ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void Allocator::reclaim(bool keep_one)
{
   reclaim(m_type,keep_one) ;
}

//----------------------------------------------------------------------------

void* Allocator::allocate_more()
{
   reclaim(m_type,true) ;
   if (s_tls[m_type].m_freelist)
      {
      return s_tls[m_type].m_freelist->allocObject() ;
      }
   // no unallocated object found, so allocate a new Slab
   Slab* new_slab ;
   if (s_local_free_slabs)
      {
      new_slab = s_local_free_slabs ;
      s_local_free_slabs = new_slab->nextSlab() ;
      --s_local_free_count ;
      }
   else
      {
      // adopt an orphaned slab
      new_slab = s_shared[m_type].m_orphans ;
      Slab* next ;
      do {
         if (!new_slab)
	    break ;
         next = new_slab->nextSlab() ;
	 } while (!s_shared[m_type].m_orphans.compare_exchange_weak(new_slab,next)) ;
      }
   if (!new_slab)
      {
      // we were unable to grab a reclaimed or an orphaned slab, so allocate a new one
      new_slab = FramepaC::SlabGroup::allocateSlab() ;
      }
   void *item = new_slab->initFreelist(s_shared[m_type].m_objsize,s_shared[m_type].m_alignment) ;
   // and insert it on both our list of owned slabs and list of slabs with available objects
   new_slab->setVMT(s_shared[m_type].m_vmt) ;
   new_slab->setOwningAllocator(m_type) ;
   new_slab->pushSlab(s_tls[m_type].m_allocslabs) ;
   if (new_slab->objectsAvailable())
      {
      new_slab->pushFreeSlab(s_tls[m_type].m_freelist) ;
      }
   return item ;
}

//----------------------------------------------------------------------------

void Allocator::gc()
{
   for (size_t i = 0 ; i < num_allocators ; ++i)
      {
      reclaim(i,false) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void Allocator::threadCleanup()
{
   // we are about to orphan all of the slabs owned by the terminating thread, so make sure
   //   some other thread adopts them....
   // cycle through all allocators and move the slabs owned by the current thread onto the non-TLS list m_orphans
   for (unsigned i = 0 ; i < num_allocators ; ++i)
      {
      Slab* slabs { s_tls[i].m_allocslabs } ;
      if (!slabs)
	 continue ;
      // find the last slab in the list
      Slab* tail { slabs } ;
      while (tail->nextSlab())
	 {
	 tail = tail->nextSlab() ;
	 }
      // link the entire list into the orphans list in one step
      Slab* orphans ;
      orphans = s_shared[i].m_orphans ;
      do {
         tail->setNextSlab(orphans) ;
         } while (!s_shared[i].m_orphans.compare_exchange_weak(orphans,slabs)) ;
      s_tls[i].m_allocslabs = nullptr ;
      }
   // return the cache of unused slabs to the global pool
   while (s_local_free_slabs)
      {
      Slab* slb = s_local_free_slabs ;
      s_local_free_slabs = slb->nextSlab() ;
      slb->unlinkSlab() ;
      SlabGroup::releaseSlab(slb) ;
      --s_local_free_count ;
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

#include "framepac/nonobject.h"

// end of file allocator.C //
