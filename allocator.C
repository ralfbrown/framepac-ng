/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-25					*/
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

#include <assert.h>
#include "framepac/atomic.h"
#include "framepac/memory.h"

using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/*	Global variables						*/
/************************************************************************/

static atom_uint num_allocators = 0 ;

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
   // unlink the slab from the doubly-linked list
   Slab* nextslab { slb->nextSlab() } ;
   Slab* prevslab { slb->prevSlab() } ;
   prevslab->setNextSlab(nextslab) ;
   nextslab->setPrevSlab(prevslab) ;
   if (nextslab == slb)			// only slab remaining?
      nextslab = nullptr ;
   unsigned index { slb->owningAllocator() } ;
   if (s_tls[index].m_allocslabs == slb)
      {
      s_tls[index].m_allocslabs = nextslab ;
      }
   // cache a small number of freed slabs to avoid global allocations
   if (s_local_free_count < FramepaC::LOCAL_SLABCACHE_HIGHWATER)
      {
      slb->setNextSlab(s_local_free_slabs) ;
      s_local_free_slabs = slb ;
      ++s_local_free_count ;
      }
   else
      {
      // return the slab to the containing group
      SlabGroup::releaseSlab(slb) ;
      //TODO: return a batch of slabs so that we get down to the low-water mark on locally-cached slabs
//!!!      while (s_local_free_count > FramepaC::LOCAL_SLABCACHE_LOWWATER)
	 {

	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void* Allocator::allocate_more()
{
   // reclaim all foreign frees
   // first, atomically grab the list of slabs with foreign-freed objects
   Slab* freelist { Atomic<Slab*>::ref(s_tls[m_type].m_foreignfree).exchange(nullptr) } ;
   // then iterate through the list, accumulating the number of slabs and min/max available objects per slab
   size_t count { 0 } ;
   unsigned min_used { ~0U } ;
   unsigned max_used { 0 } ;
   while (freelist)
      {
      count++ ;
      Slab* slb { freelist } ;
      freelist = freelist->nextFreeSlab() ;
      slb->reclaimForeignFrees() ;
      size_t in_use { slb->objectsInUse() } ;
      if (in_use < min_used) min_used = in_use ;
      if (in_use > max_used) max_used = in_use ;
      slb->setNextFreeSlab(s_tls[m_type].m_freelist) ;
      s_tls[m_type].m_freelist = slb ;
      }
   // if we have multiple Slabs, release any which are completely unused back to the general pool
   if (count > 1 && min_used == 0)
      {
      Slab* slb { s_tls[m_type].m_freelist } ;
      freelist = nullptr ;
      if (max_used == 0)
	 {
	 Slab* currslab { slb } ;
	 slb = slb->nextFreeSlab() ;
	 currslab->setNextFreeSlab(nullptr) ;
	 freelist = currslab ;
	 }
      while (slb)
	 {
	 Slab* currslab { slb } ;
	 slb = slb->nextFreeSlab() ;
	 if (currslab->objectsInUse() == 0)
	    {
	    // release back to general pool
	    SlabGroup::releaseSlab(currslab) ;
	    }
	 else
	    {
	    currslab->setNextFreeSlab(freelist) ;
	    freelist = currslab ;
	    }
	 }
      s_tls[m_type].m_freelist = freelist ;
      }
   void* item ;
   if (s_tls[m_type].m_freelist)
      {
      (void)s_tls[m_type].m_freelist->allocObject(item) ;
      return item ;
      }
   // no unallocated object found, so allocate a new Slab
   Slab* new_slab ;
   if (s_local_free_slabs)
      {
      new_slab = s_local_free_slabs ;
      s_local_free_slabs = new_slab->nextSlab() ;
      --s_local_free_count ;
      }
   else if (s_shared[m_type].m_orphans)
      {
      // adopt an orphaned slab
      new_slab = s_shared[m_type].m_orphans ;
      Slab* next ;
      do {
         if (!new_slab)
	    break ;
         next = new_slab->nextSlab() ;
	 } while (!s_shared[m_type].m_orphans.compare_exchange_weak(new_slab,next)) ;
      if (new_slab)
	 {
	 // adopt the slab
	 }
      else
	 {
	 // we were unable to grab an orphaned slab, so allocate a new one
	 new_slab = FramepaC::SlabGroup::allocateSlab() ;
	 }
      }
   else
      {
      new_slab = FramepaC::SlabGroup::allocateSlab() ;
      }
   item = new_slab->initFreelist(s_shared[m_type].m_objsize) ;
   // and insert it on our list of owned slabs
   new_slab->linkSlab(s_tls[m_type].m_allocslabs) ;
   return item ;
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
      }
   // return the cache of unused slabs to the global pool
   while (s_local_free_slabs)
      {
      Slab* slb = s_local_free_slabs ;
      s_local_free_slabs = slb->nextSlab() ;
      SlabGroup::releaseSlab(slb) ;
      --s_local_free_count ;
      }
   return ;
}

//----------------------------------------------------------------------------

size_t Allocator::reclaim()
{
   //FIXME
   return 0 ;
}

} // end namespace Fr

// end of file allocator.C //
