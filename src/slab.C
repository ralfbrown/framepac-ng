/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-19					*/
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

#include <cassert>
#include <cstddef> // for offsetof()
#include "framepac/memory.h"

namespace FramepaC
{

/************************************************************************/
/************************************************************************/

Slab::Slab()
   : m_info(), m_header(), m_footer()
{
   static_assert(offsetof(Slab,m_header) == 64, "wrong alignment") ;
   return ;
}

//----------------------------------------------------------------------------

Slab::~Slab()
{

   return ;
}

//----------------------------------------------------------------------------

void* Slab::reclaimForeignFrees()
{
   // grab any objects on the remote-free list
   auto free = m_footer.grabList() ;
   if (free.second == 0)  // if count == 0
      return nullptr ;
   // point the local freelist at the objects we've just reclaimed
   m_header.m_freelist = free.first ;
   m_header.m_usedcount -= free.second ;
   m_header.m_usedcount++ ;
   void* obj = ((char*)this) + m_header.m_freelist ;
   m_header.m_freelist = *((alloc_size_t*)obj) ;
   return obj ;
}

//----------------------------------------------------------------------------

   
void Slab::releaseObject(void* obj, Slab*& freelist)
{
#ifndef FrSINGLE_THREADED
   if (this_thread::get_id() != m_info.m_owner)
      {
      m_footer.link(obj) ;
      //FIXME: deal with slab-list on foreign frees
      }
   else
#endif /* !FrSINGLE_THREADED */
      {
      alloc_size_t old_freelist = m_header.m_freelist ;
      *((alloc_size_t*)obj) = old_freelist ;
      ASAN(ASAN_POISON_MEMORY_REGION(obj,m_info.m_objsize)) ;
      m_header.m_freelist = slabOffset(obj) ;
      m_header.m_usedcount-- ;
      if (old_freelist == 0)
	 {
	 // this was the first object freed, so add the slab to the list of slabs with available objects
	 pushFreeSlab(freelist) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void Slab::pushSlab(Slab*& listhead)
{
#ifndef FrSINGLE_THREADED
   // claim ownership of the slab by the current thread
   m_info.m_owner = this_thread::get_id() ;
#endif /* !FrSINGLE_THREADED */
   setNextSlab(listhead) ;
   setPrevSlabPtr(&listhead) ;
   if (listhead) listhead->setPrevSlabPtr(&(this->m_info.m_nextslab)) ;
   listhead = this ;
   return ;
}

//----------------------------------------------------------------------------

void Slab::clearOwner()
{
   m_info.m_owner = (thread::id)0 ;
   return ;
}

//----------------------------------------------------------------------------

#if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ < 9
// GCC 4.8 doesn't have std::align !
} // end namespace FramepaC

namespace std {
void* align( std::size_t alignment, std::size_t size, void*& ptr, std::size_t& space)
{
   if (alignment == 0 || (alignment & (alignment-1)) != 0) return nullptr ; // not a power of two
   if (space < size) return nullptr;
   std::intptr_t p = (((std::intptr_t)ptr) + (alignment-1)) & ~(alignment-1) ;
   std::intptr_t adj = p - (std::intptr_t)ptr ;
   if (size + adj > space) return nullptr ;
   space -= adj ;
   ptr = (void*)p ;
   return (void*)p ;
}

} // end namespace std

namespace FramepaC {
#endif /* GCC 4.8 */

//----------------------------------------------------------------------------

alloc_size_t Slab::makeFreeList(unsigned objsize, unsigned align)
{
   ASAN(ASAN_UNPOISON_MEMORY_REGION(m_buffer,sizeof(m_buffer))) ;
   // round up the object size to the next higher multiple of 'align'
   ASAN(if (align < 8) align = 8) ;
   unsigned multiple { (objsize + align - 1) / align } ;
   objsize = align * multiple ;
   // store the adjusted object size
   m_info.m_objsize = objsize ;
   m_header.m_usedcount = 0 ;
   // align the start of the buffer
   void* buf = m_buffer ;
   size_t space = sizeof(m_buffer) ;
   if (!std::align(align,objsize,buf,space))
      {
      // error: unable to fit an object of 'objsize' bytes with alignment 'align' into m_buffer
      m_info.m_objcount = 0 ;
//FIXME
      return 0 ;
      }
   // compute the number of objects the slab can hold
   size_t objcount { space / objsize } ;
   m_info.m_objcount = objcount ;
   alloc_size_t prev = 0 ;
   assert(objcount > 1) ;
   if (objcount > 1)			// require at least two objects so we don't have to special-case
      {		      			//   the 'full slab' condition during allocation
      //  link together all of the objects
      for (size_t i = 0 ; i < objcount ; ++i)
	 {
         char* currobj = ((char*)buf) + i*objsize ;
	 alloc_size_t curr = slabOffset(currobj) ;
	 *((alloc_size_t*)currobj) = prev ;
	 prev = curr ;
         }
      }
   // set the start of the freelist to the last object added to the linked list
   m_header.m_freelist = prev ;
   ASAN(ASAN_POISON_MEMORY_REGION(m_buffer,sizeof(m_buffer))) ;
   return prev ;
}

//----------------------------------------------------------------------------

void* Slab::initFreelist(unsigned objsize, unsigned align)
{
   alloc_size_t first = makeFreeList(objsize, align) ;
   // pop the first entry off the free list
   return first ? allocObject() : nullptr ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slab.C //
