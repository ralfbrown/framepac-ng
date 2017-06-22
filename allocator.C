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
#include "framepac/memory.h"
using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

ThreadInitializer<Allocator> Allocator::s_threadinit ;
AllocatorSharedInfo Allocator::s_shared[FramepaC::MAX_ALLOCATOR_TYPES] ;
thread_local AllocatorTLS Allocator::s_tls[FramepaC::MAX_ALLOCATOR_TYPES] ;
thread_local FramepaC::Slab* Allocator::s_local_free_slabs = nullptr ;
thread_local unsigned Allocator::s_local_free_count = 0 ;

/************************************************************************/
/*	Methods for class Allocator					*/
/************************************************************************/

static unsigned find_match(AllocatorSharedInfo* info, const FramepaC::Object_VMT_Base* vmt,
   unsigned objsize, unsigned align)
{
   size_t existing = FramepaC::MAX_ALLOCATOR_TYPES ; //FIXME
   for (size_t i = 0 ; i < existing ; ++i)
      {
      if (info[i].m_vmt == vmt && info[i].m_objsize == objsize && info[i].m_alignment == align)
	 return i ;
      }
   // if we get here, there was no matching allocator, so generate a new one
   //FIXME

   // uh oh, we've filled up the array of allocator types...
   return ~0 ;
}

//----------------------------------------------------------------------------

Allocator::Allocator(const FramepaC::Object_VMT_Base* vmt, unsigned objsize)
{
   unsigned align = alignof(double) ;
   if (objsize < align)
      align = objsize ;
   m_type = find_match(s_shared,vmt,objsize,align) ;
   return ;
}

//----------------------------------------------------------------------------

Allocator::Allocator(const FramepaC::Object_VMT_Base* vmt, unsigned objsize, unsigned align)
{
   m_type = find_match(s_shared,vmt,objsize,align) ;
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
