/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-04					*/
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

void* Slab::allocObject()
{
   if (m_header.m_freelist)
      {
      m_header.m_freecount-- ;
      void* obj = ((char*)this) + m_header.m_freelist ;
      m_header.m_freelist = *((alloc_size_t*)obj) ;
      return obj ;
      }
   // our local freelist was empty, so grab any objects on the remote-free list
   alloc_size_t free = m_footer.m_firstfree.exchange(0) ;
   if (!free)
      return nullptr ;
   // point the local freelist at the objects we've just reclaimed
   m_header.m_freelist = free ;
   m_header.m_freecount = 1 ;
   while ((free = *((alloc_size_t*)(((char*)this)+free))) != 0)
      {
      m_header.m_freecount++ ;
      }
   return allocObject() ;
}

//----------------------------------------------------------------------------

void Slab::releaseObject(void* obj)
{
   if (!obj)
      return ;
   alloc_size_t freeobj = (alloc_size_t)(((uintptr_t)obj) & (SLAB_SIZE - 1)) ;
   if (freeobj < offsetof(Slab,m_buffer))
      return ;
   //if (our-slab) {
   *((alloc_size_t*)(this + freeobj)) = m_header.m_freelist ;
   m_header.m_freelist = freeobj ;
   m_header.m_freecount++ ;
   if (m_header.m_freecount == m_info.m_objcount) { /*release slab*/ }
#if 0
   //} else {
   alloc_size_t* nextptr = ((alloc_size_t*)obj) ;
   *nextptr = m_footer.m_firstfree.load() ;
   m_footer.m_firstfree = freeobj ;
#endif
   return ;
}

//----------------------------------------------------------------------------

void Slab::linkSlab(Slab*& /*listhead*/)
{
   //TODO
   return ;
}

//----------------------------------------------------------------------------

void Slab::unlinkSlab(Slab*& /*listhead*/)
{
   //TODO
   return ;
}

//----------------------------------------------------------------------------

void* Slab::initFreelist(unsigned /*listhead*/)
{
   //TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slab.C //
