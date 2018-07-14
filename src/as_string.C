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

#include <cstdlib>
#include "framepac/as_string.h"
#include "framepac/object.h"
#include "framepac/texttransforms.h"

using namespace std ;

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

template <>
int string_as(const char* s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   int value = (int)strtol(s,&endptr,0) ;
   success = endptr != s ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
unsigned string_as(const char* s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   unsigned value = (unsigned)strtoul(s,&endptr,0) ;
   success = endptr != s ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
long string_as(const char* s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   long value = strtol(s,&endptr,0) ;
   success = endptr != s ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
unsigned long string_as(const char* s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   unsigned long value = strtoul(s,&endptr,0) ;
   success = endptr != s ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
float string_as(const char* s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   float value = strtof(s,&endptr) ;
   success = endptr != s ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
double string_as(const char* s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   double value = strtod(s,&endptr) ;
   success = endptr != s ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
long double string_as(const char* s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   long double value = strtold(s,&endptr) ;
   success = endptr != s ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
const char* string_as(const char* s, bool& success)
{
   success = true ;
   return s ;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------


} // end namespace Fr

// end of file as_string.C //
