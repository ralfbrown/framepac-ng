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

template <typename ObjT>
thread_local Slab* Allocator<ObjT>::m_currslab ;

/************************************************************************/
/*	Methods for class Allocator					*/
/************************************************************************/

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file allocator.C //
