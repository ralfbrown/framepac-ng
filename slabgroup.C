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

#include <cstdlib>
#include "framepac/memory.h"
using namespace std ;

namespace FramepaC
{

/************************************************************************/
/************************************************************************/

SlabGroup* SlabGroup::s_grouplist_fwd = 0 ;
SlabGroup* SlabGroup::s_grouplist_rev = 0 ;
mutex SlabGroup::s_mutex ;

/************************************************************************/
/************************************************************************/

SlabGroup::SlabGroup()
{
   lock_guard<mutex> lock(s_mutex) ;
   m_next = s_grouplist_fwd ;
   s_grouplist_fwd = this ;
   if (!s_grouplist_rev)
      s_grouplist_rev = this ;
   return ;
}

//----------------------------------------------------------------------------

void* SlabGroup::operator new(size_t sz)
{
   void* alloc ;
   return (posix_memalign(&alloc,sizeof(Slab),sz) ) ? alloc : nullptr ;
}

//----------------------------------------------------------------------------

void SlabGroup::operator delete(void* grp)
{
   free(grp) ; 
   return ;
}

//----------------------------------------------------------------------------

void SlabGroup::unlink()
{
   lock_guard<mutex> lock(s_mutex) ;
   if (m_prev)
      m_prev->m_next = m_next ;
   else
      s_grouplist_fwd = m_next ;
   if (m_next)
      m_next->m_prev = m_prev ;
   else
      s_grouplist_rev = m_prev ;
   return ;
}

//----------------------------------------------------------------------------

Slab* SlabGroup::allocateSlab()
{
   //!!!SlabGroup* sg = new SlabGroup ;
   //!!!m_numfree-- ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

void SlabGroup::releaseSlab(Slab* )
{
   //!!!m_numfree++ ;
   //FIXME

   return ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

// end of file slabgroup.C //
