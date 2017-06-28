/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-26					*/
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

#include <cstdlib>
#include "framepac/memory.h"
using namespace std ;

namespace FramepaC
{

/************************************************************************/
/************************************************************************/

SlabGroup* SlabGroup::s_grouplist { nullptr } ;
SlabGroup* SlabGroup::s_freelist { nullptr } ;
mutex SlabGroup::s_grouplist_mutex ;
mutex SlabGroup::s_freelist_mutex ;

/************************************************************************/
/************************************************************************/

// -Weffc++ warns about not setting m_next in the initializer list, but we can't safely
//   set it until we've locked the mutex in the ctor's body....
#pragma GCC diagnostic ignored "-Weffc++"

SlabGroup::SlabGroup()
   : m_mutex()
{
   // set up the linked list of free slabs
   m_slabs[0].setNextSlab(nullptr) ;
   m_slabs[0].setSlabID(0) ;
   for (size_t i = 1 ; i < lengthof(m_slabs) ; ++i)
      {
      m_slabs[i].setNextSlab(&m_slabs[i-1]) ;
      m_slabs[i].setSlabID(i) ;
      }
   m_freeslabs = &m_slabs[SLAB_GROUP_SIZE-1] ;
   // link the new group into the doubly-linked list of SlabGroups
   pushGroup() ;
   // and into the doubly-linked list of groups with free Slabs
   pushFreeGroup() ;
   return ;
}

//----------------------------------------------------------------------------

void* SlabGroup::operator new(size_t sz)
{
   void* alloc ;
   return (posix_memalign(&alloc,sizeof(Slab),sz) ) ? nullptr : alloc ;
}

//----------------------------------------------------------------------------

void SlabGroup::operator delete(void* grp)
{
   free(grp) ; 
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::pushGroup()
{
   lock_guard<mutex> _(s_grouplist_mutex) ;
   m_next = s_grouplist ;
   m_prev = &s_grouplist ;
   if (m_next)
      m_next->m_prev = &m_next ;
   s_grouplist = this ;
   return;
}

//----------------------------------------------------------------------------

void SlabGroup::unlinkGroup()
{
   unlinkFreeGroup() ;
   lock_guard<mutex> _(s_grouplist_mutex) ;
   SlabGroup** prev = m_prev ;
   if (prev)
      {
      SlabGroup* next = m_next ;
      *prev = next ;
      if (next) next->m_prev = prev ;
      }
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::unlinkFreeGroup()
{
   bool not_locked = s_freelist_mutex.try_lock() ;
   if (m_prevfree)			// is this slab on the freelist?
      {
      SlabGroup* next = m_nextfree ;
      (*m_prevfree) = next ;
      if (next)
	 next->m_prevfree = m_prevfree ;
      }
   if (not_locked)
      s_freelist_mutex.unlock() ;
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::pushFreeGroup()
{
   lock_guard<mutex> _(s_freelist_mutex) ;
   m_prevfree = &s_freelist ;
   m_nextfree = s_freelist ;
   if (m_nextfree)
      m_nextfree->m_prevfree = &this->m_nextfree ;
   s_freelist = this ;
   return ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::popFreeSlab()
{
   lock_guard<mutex> _(s_freelist_mutex) ;
   SlabGroup *sg = s_freelist ;
   if (!sg) return nullptr ;
   // allocate an available Slab from the group's freelist
   lock_guard<mutex> lock(sg->m_mutex) ;
   Slab* slb = sg->m_freeslabs ;
   sg->m_freeslabs = slb->nextSlab() ;
   if (--sg->m_numfree == 0)		// is group now completely allocated?
      {
      // remove the slabgroup from the list of groups with free slabs
      sg->unlinkFreeGroup() ;
      }
   return slb ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::allocateSlab()
{
   Slab* slb = popFreeSlab() ;
   while (!slb)
      {
      // no slabs available, so allocate a new SlabGroup
      SlabGroup* sg = new SlabGroup ;
      if (!sg)
	 return nullptr ;
      // and retry the allocation
      slb = popFreeSlab() ;
      }
   return slb ;
}

//----------------------------------------------------------------------------

void SlabGroup::releaseSlab(Slab* slb)
{
   SlabGroup* sg = slb->containingGroup() ;
#ifndef FrSINGLE_THREADED
   slb->clearOwner() ;
#endif /* !FrSINGLE_THREADED */
   sg->m_mutex.lock() ;
   // add slab to list of free slabs in its group
   slb->setNextSlab(sg->m_freeslabs) ;
   sg->m_freeslabs = slb ;
   // update statistics
   unsigned freecount = ++sg->m_numfree ;
   if (freecount == lengthof(m_slabs))
      {
      // this group is now completely unused, so return it to the operating system
      sg->unlinkGroup() ;
      sg->m_mutex.unlock() ;
      delete sg ;
      return ;
      }
   else if (freecount == 1)
      {
      // first free slab added to this group, so link it into the list of groups with free slabs
      sg->pushFreeGroup() ;
      }
   sg->m_mutex.unlock() ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slabgroup.C //
