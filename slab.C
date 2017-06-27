/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
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

#include <cstddef> //TMP
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
      m_header.m_freelist = slabOffset(obj) ;
      if (--m_header.m_usedcount == 0)
	 {
	 Fr::Allocator::releaseSlab(this) ;
	 }
      else if (old_freelist == 0)
	 {
	 // this was the first object freed, so add the slab to the list of slabs with available objects
	 setNextFreeSlab(freelist) ;
	 freelist = this ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void Slab::linkSlab(Slab*& listhead)
{
   if (listhead == nullptr)
      listhead = this ;
   else
      {
      // link in the current slab following the item at the head of the list
      Slab* next = listhead->nextSlab() ;
      m_info.m_prevslab = listhead ;
      m_info.m_nextslab = next ;
      next->m_info.m_prevslab = this ;
      listhead->m_info.m_nextslab = this ;
      }
   return ;
}

//----------------------------------------------------------------------------

void Slab::unlinkSlab(Slab*& listhead)
{
   Slab* prev = prevSlab() ;
   Slab* next = nextSlab() ;
   if (prev == next)
      {
      // if there's only the current slab, set the list to NULL
      listhead = nullptr ;
      }
   else
      {
      // unlink the slab from the list
      prev->m_info.m_nextslab = next ;
      next->m_info.m_prevslab = prev ;
      if (listhead == this)
	 listhead = next ;
      }
   //TODO: return slab to SlabGroup's freelist

   return ;
}

//----------------------------------------------------------------------------

void* Slab::initFreelist(unsigned objsize, unsigned align)
{
   // round up the object size to the next higher multiple of 'align'
   unsigned multiple { (objsize + align - 1) / align } ;
   objsize = align * multiple ;
   // align the start of the buffer
   void* buf = m_buffer ;
   size_t space = sizeof(m_buffer) ;
   if (!std::align(align,objsize,buf,space))
      {
      // error: unable to fit an object of 'objsize' bytes with alignment 'align' into m_buffer
//FIXME
      return nullptr ;
      }
   // compute the number of objects the slab can hold
   size_t objcount { space / objsize } ;
   m_info.m_objsize = objsize ;
   m_info.m_objcount = objcount ;
   // and link together all but one of the objects
   alloc_size_t prev = 0 ;
   for (size_t i = 1 ; i < objcount ; ++i)
      {
      char* currobj = ((char*)buf) + i*objsize ;
      alloc_size_t curr = slabOffset(currobj) ;
      *((alloc_size_t*)currobj) = prev ;
      prev = curr ;
      }
   // set the start of the freelist to the last object added to the linked list
   m_header.m_freelist = prev ;
   m_header.m_usedcount = 1 ;		// the object we're returning is in use
   return buf ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slab.C //
