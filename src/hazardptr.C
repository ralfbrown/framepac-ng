/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.01, last edit 2017-04-05					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/*  File atomic.h		atomic operations on simple variables	*/
/*									*/
/*  (c) Copyright 2015,2016,2017 Carnegie Mellon University		*/
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

#include "framepac/atomic.h"

namespace Fr
{

bool HazardPointerList::registerPointer(void*)
{
   //TODO
   return true ;
}

//----------------------------------------------------------------------------

bool HazardPointerList::unregisterPointer(void*)
{
   //TODO
   return true ;
}

//----------------------------------------------------------------------------

bool HazardPointerList::hasHazard(void* /*object*/) const
{
   //TODO
   return false ;
}


} // end namespace Fr

// end of file hazardptr.C //
