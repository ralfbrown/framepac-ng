/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-07					*/
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
   unsigned existing = num_allocators ;
   for (unsigned i = 0 ; i < existing ; ++i)
      {
      if (info[i].m_vmt == vmt && info[i].m_objsize == objsize && info[i].m_alignment == align)
	 return i ;
      }
   // if we get here, there was no matching allocator, so generate a new one
   unsigned idx = num_allocators++ ;
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
   unsigned align = alignof(double) ;
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
   unsigned index = slb->owningAllocator() ;
   if (s_tls[index].m_currslab == slb)
      {
//FIXME
      }
   return ;
}

//----------------------------------------------------------------------------

static void orphan_slabs(FramepaC::Slab*& currlist, FramepaC::Slab*& orphans)
{
   Slab* slabs = currlist ;
   if (slabs)
      {
      currlist = nullptr ;
      Slab* tail = slabs ;
      while (tail->nextSlab())
	 {
	 tail = tail->nextSlab() ;
	 }
      tail->setNextSlab(orphans) ;
      orphans = slabs ;
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
      orphan_slabs(s_tls[i].m_currslab,s_shared[i].m_orphans) ;
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
