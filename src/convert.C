/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-14					*/
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
#include "framepac/convert.h"

namespace Fr
{

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, char& value)
{
   if (!input || !*input)
      return false ;
   value = *input++ ;
   return true ;
}

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, std::int32_t& value)
{
   if (!input || !*input)
      return false ;
   char* past_end { nullptr } ;
   long v = strtol(input,&past_end,0) ;
   if (past_end != input)
      {
      if (sizeof(unsigned long) == sizeof(uint32_t) || (v >= INT32_MIN && v <= INT32_MAX))
	 {
	 value = v ;
	 input = past_end ;
	 return true ;
	 }
      }
   value = 0 ;
   return false ;
}

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, uint32_t& value)
{
   if (!input || !*input)
      return false ;
   char* past_end { nullptr } ;
   unsigned long v = strtoul(input,&past_end,0) ;
   if (past_end != input)
      {
      if (sizeof(unsigned long) == sizeof(uint32_t) || v <= UINT32_MAX)
	 {
	 value = v ;
	 input = past_end ;
	 return true ;
	 }
      }
   value = 0 ;
   return false ;
}

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, size_t& value)
{
   if (!input || !*input)
      return false ;
   char* past_end { nullptr } ;
   size_t v = strtoul(input,&past_end,0) ;
   if (past_end != input)
      {
      value = v ;
      input = past_end ;
      return true ;
      }
   value = 0 ;
   return false ;
}

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, long& value)
{
   if (!input || !*input)
      return false ;
   char* past_end { nullptr } ;
   long v = strtol(input,&past_end,0) ;
   if (past_end != input)
      {
      value = v ;
      input = past_end ;
      return true ;
      }
   value = 0 ;
   return false ;
}

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, float& value)
{
   if (!input || !*input)
      return false ;
   char* past_end { nullptr } ;
   float v = strtof(input,&past_end) ;
   if (past_end != input)
      {
      value = v ;
      input = past_end ;
      return true ;
      }
   value = 0.0f ;
   return false ;
}

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, double& value)
{
   if (!input || !*input)
      return false ;
   char* past_end { nullptr } ;
   double v = strtod(input,&past_end) ;
   if (past_end != input)
      {
      value = v ;
      input = past_end ;
      return true ;
      }
   value = 0.0 ;
   return false ;
}

//----------------------------------------------------------------------------

template <>
bool convert_string(const char*&input, long double& value)
{
   if (!input || !*input)
      return false ;
   char* past_end { nullptr } ;
   long double v = strtold(input,&past_end) ;
   if (past_end != input)
      {
      value = v ;
      input = past_end ;
      return true ;
      }
   value = 0.0 ;
   return false ;
}

//----------------------------------------------------------------------------



} //end namespace Fr

// end of file convert.C //
