/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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
#include "framepac/nonobject.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class SmallAlloc					*/
/************************************************************************/

SmallAlloc::SmallAlloc()
{
   return  ;
}

//----------------------------------------------------------------------------

SmallAlloc::~SmallAlloc()
{
   return ;
}

//----------------------------------------------------------------------------

SmallAlloc* SmallAlloc::create(size_t objsize)
{
   (void)objsize ;
   return nullptr ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file smallalloc.C //
