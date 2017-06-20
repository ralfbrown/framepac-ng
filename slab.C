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

void Slab::releaseObject(void* obj)
{
#ifndef FrSINGLE_THREADED
   if (this_thread::get_id() != m_info.m_owner)
      {
      m_footer.link(obj) ;
      }
   else
#endif /* !FrSINGLE_THREADED */
      {
      *((alloc_size_t*)obj) = m_header.m_freelist ;
      m_header.m_freelist = slabOffset(obj) ;
      if (--m_header.m_usedcount == 0) { VMT()->releaseSlab_(this) ; }
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

void* Slab::initFreelist(unsigned /*listhead*/)
{
   //TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slab.C //
