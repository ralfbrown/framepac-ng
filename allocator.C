/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-05					*/
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

thread_local SlabFreelist* AllocatorBase::m_freelist ;
thread_local SlabFreelist* AllocatorBase::m_pending ;
thread_local Slab* AllocatorBase::m_hazard ;
HazardPointerList AllocatorBase::m_hazardlist ;
Slab* AllocatorBase::m_slabs ;

//----------------------------------------------------------------------------

AllocatorBase::~AllocatorBase()
{
   return ;
}

//----------------------------------------------------------------------------

void *AllocatorBase::allocate_more()
{
   assert(m_freelist == nullptr) ; // call only when freelist is empty
   SlabFreelist *item = reclaimPending(m_pending) ;
   if (item)
      return item ;
#ifndef FrSINGLE_THREADED
   // we didn't have any pending frees on our own thread's list, so try stealing
   //   the pending list from another thread
   item = stealPendingFrees() ;
   if (item)
      return item ;
#endif /* !FrSINGLE_THREADED */
   // we weren't able to reclaim any pending frees, so we need to allocate another slab
   Slab *new_slab = SlabGroup::allocateSlab() ;
   new_slab->linkSlab(m_slabs) ;
   item = reinterpret_cast<SlabFreelist*>(new_slab->initFreelist(m_objsize)) ;
   m_freelist = item->next() ;
   return (void*)item ;
}

//----------------------------------------------------------------------------

SlabFreelist *reclaimPending(Atomic<SlabFreelist*> &pending)
{
   // atomically grab the entire list of pending frees
   SlabFreelist *items = pending.exchange(nullptr) ;
  // pop off the first item to reallocate, and redistribute any
   // remaining items back to their slabs
   SlabFreelist *item = items ;
   items = items->next() ;
//FIXME
   return item ;
}

//----------------------------------------------------------------------------

SlabFreelist *stealPendingFrees()
{
   // scan the list of per-thread pointers to m_pending, looking for one that is non-empty
#if 0
   for ( ... )
      {
      SlabFreelist **pending = ??? ;
      SlabFreelist *item = reclaim(pending) ;
      if (item)
	 return item ;
      }
#endif
   return nullptr ;
}

//----------------------------------------------------------------------------

SlabFreelist* AllocatorBase::reclaimPending(SlabFreelist*& )
{
   //TODO
   // convert to atomic with Atomic<SlabFreelist*>::ref(x)
   return nullptr ;
}

//----------------------------------------------------------------------------

SlabFreelist* AllocatorBase::stealPendingFrees()
{
   //TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

bool AllocatorBase::reclaim_foreign_frees()
{
   //FIXME
   return false ;
}

//----------------------------------------------------------------------------

void AllocatorBase::threadInit()
{
   m_hazardlist.registerPointer(&m_hazard) ;
   return ;
}

//----------------------------------------------------------------------------

void AllocatorBase::threadCleanup()
{
   m_hazardlist.unregisterPointer(&m_hazard) ;
   return ;
}

/************************************************************************/
/*	Methods for class Allocator					*/
/************************************************************************/

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file allocator.C //
