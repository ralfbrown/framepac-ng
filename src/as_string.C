/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-13					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#include "framepac/as_string.h"
#include "framepac/object.h"
#include "framepac/texttransforms.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

char* as_string(long value)
{
   return Fr::aprintf("%ld",value) ;
}

//----------------------------------------------------------------------------

char* as_string(unsigned long value)
{
   return Fr::aprintf("%lu",value) ;
}

//----------------------------------------------------------------------------

char* as_string(double value)
{
   return Fr::aprintf("%g",value) ;
}

//----------------------------------------------------------------------------

char* as_string(char value)
{
   return Fr::aprintf("%c",value) ;
}

//----------------------------------------------------------------------------

char* as_string(const char* value)
{
   return dup_string(value ? value : "(null)") ;
}

//----------------------------------------------------------------------------

char* as_string(const Object* value)
{
   return value ? value->cString() : dup_string("#N<>") ;
}

//----------------------------------------------------------------------------


} // end namespace Fr

// end of file as_string.C //
