/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-07					*/
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
#include <cstring>
#include "framepac/as_string.h"
#include "framepac/object.h"
#include "framepac/texttransforms.h"

using namespace std ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

unsigned len_as_string(long value)
{
   return snprintf(nullptr,0,"%ld",value) ;
}

//----------------------------------------------------------------------------

CharPtr as_string(long value)
{
   return Fr::aprintf("%ld",value) ;
}

//----------------------------------------------------------------------------

char* as_string(long value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%ld",value) ;
}

//----------------------------------------------------------------------------

unsigned len_as_string(unsigned int value)
{
   return snprintf(nullptr,0,"%u",value) ;
}

//----------------------------------------------------------------------------

CharPtr as_string(unsigned int value)
{
   return Fr::aprintf("%u",value) ;
}

//----------------------------------------------------------------------------

char* as_string(unsigned int value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%u",value) ;
}

//----------------------------------------------------------------------------

unsigned len_as_string(unsigned long value)
{
   return snprintf(nullptr,0,"%lu",value) ;
}

//----------------------------------------------------------------------------

CharPtr as_string(unsigned long value)
{
   return Fr::aprintf("%lu",value) ;
}

//----------------------------------------------------------------------------

char* as_string(unsigned long value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%lu",value) ;
}

//----------------------------------------------------------------------------

#if 0 //FIXME: figure out how to only instantiate if intmax_t is bigger than long
unsigned len_as_string(std::intmax_t value)
{
   return snprintf(nullptr,0,"%jd",value) ;
}

//----------------------------------------------------------------------------

CharPtr as_string(std::intmax_t value)
{
   return Fr::aprintf("%jd",value) ;
}

//----------------------------------------------------------------------------

char* as_string(std::intmax_t value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%jd",value) ;
}
#endif /* 0 intmax_t */

//----------------------------------------------------------------------------

#if 0 //FIXME: figure out how to only instantiate if uintmax_t is bigger than unsigned long
unsigned len_as_string(std::uintmax_t value)
{
   return snprintf(nullptr,0,"%ju",value) ;
}

//----------------------------------------------------------------------------

CharPtr as_string(std::uintmax_t value)
{
   return Fr::aprintf("%ju",value) ;
}

//----------------------------------------------------------------------------

char* as_string(std::uintmax_t value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%ju",value) ;
}
#endif /* 0 uintmax_t */

//----------------------------------------------------------------------------

unsigned len_as_string(double value)
{
   return snprintf(nullptr,0,"%g",value) ;
}

//----------------------------------------------------------------------------

CharPtr as_string(double value)
{
   return Fr::aprintf("%g",value) ;
}

//----------------------------------------------------------------------------

char* as_string(double value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%g",value) ;
}

//----------------------------------------------------------------------------

unsigned len_as_string(long double value)
{
   return snprintf(nullptr,0,"%Lg",value) ;
}

//----------------------------------------------------------------------------

CharPtr as_string(long double value)
{
   return Fr::aprintf("%Lg",value) ;
}

//----------------------------------------------------------------------------

char* as_string(long double value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%Lg",value) ;
}

//----------------------------------------------------------------------------

unsigned len_as_string(char /*value*/)
{
   return 1 ;
}

//----------------------------------------------------------------------------

CharPtr as_string(char value)
{
   return Fr::aprintf("%c",value) ;
}

//----------------------------------------------------------------------------

char* as_string(char value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%c",value) ;
}

//----------------------------------------------------------------------------

unsigned len_as_string(const char* value)
{
   return value ? strlen(value) : 6 ;
}

//----------------------------------------------------------------------------

CharPtr as_string(const char* value)
{
   return dup_string(value ? value : "(null)") ;
}

//----------------------------------------------------------------------------

char* as_string(const char* value, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%s",value?value:"(null)") ;
}

//----------------------------------------------------------------------------

unsigned len_as_string(const Object* value)
{
   return value ? (int)value->cStringLength() : 4 ;
}

//----------------------------------------------------------------------------

CharPtr as_string(const Object* value)
{
   return value ? value->cString() : dup_string("#N<>") ;
}

//----------------------------------------------------------------------------

char* as_string(const Object* /*value*/, char* buf, size_t /*buflen*/)
{
//FIXME   return buf + snprintf(buf,buflen,"%ld",value) ;
   return buf ;
}

//----------------------------------------------------------------------------

unsigned len_as_string(const void* /*value*/)
{
   return 3 ; // will print "???"
}

//----------------------------------------------------------------------------

CharPtr as_string(const void* /*value*/)
{
   return dup_string("???") ;
}

//----------------------------------------------------------------------------

char* as_string(const void* /*value*/, char* buf, size_t buflen)
{
   return buf + snprintf(buf,buflen,"%s","???") ;
}

/************************************************************************/
/*	Conversion FROM string to number/object				*/
/************************************************************************/

template <>
int string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   int value = (int)strtol(s,&endptr,0) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
unsigned string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   unsigned value = (unsigned)strtoul(s,&endptr,0) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
long string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   long value = strtol(s,&endptr,0) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
unsigned long string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   unsigned long value = strtoul(s,&endptr,0) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
long long string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   long long value = strtoll(s,&endptr,0) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
unsigned long long string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   unsigned long long value = strtoull(s,&endptr,0) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
float string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   float value = strtof(s,&endptr) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
double string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   double value = strtod(s,&endptr) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
long double string_as(const char*& s, bool& success)
{
   char* endptr = const_cast<char*>(s) ;
   long double value = strtold(s,&endptr) ;
   success = endptr != s ;
   s = endptr ;
   return value ;
}

//----------------------------------------------------------------------------

template <>
const char* string_as(const char*& s, bool& success)
{
   success = true ;
   const char* result = s ;
   s = s ? strchr(s,'\0') : s ;
   return result ;
}

//----------------------------------------------------------------------------


} // end namespace Fr

// end of file as_string.C //
