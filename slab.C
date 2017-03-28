/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
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

#include "framepac/memory.h"

namespace FramepaC
{

/************************************************************************/
/************************************************************************/

Slab::Slab()
{

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
   alloc_size_t free = m_footer.f.m_firstfree ;
   if (!free)
      return nullptr ;
   alloc_size_t* obj = (alloc_size_t*)(((char*)this) + free) ;
   m_footer.f.m_numfree-- ;
   m_footer.f.m_firstfree.store(*obj) ;
   return obj ;
}

//----------------------------------------------------------------------------

void Slab::releaseObject(void* obj)
{
   if (!obj)
      return ;
   alloc_size_t freeobj = (alloc_size_t)(((uintptr_t)obj) & (SLAB_SIZE - 1)) ;
   alloc_size_t* nextptr = ((alloc_size_t*)obj) ;
   *nextptr = m_footer.f.m_firstfree.load() ;
   m_footer.f.m_numfree++ ;
   m_footer.f.m_firstfree.store(freeobj) ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slab.C //
