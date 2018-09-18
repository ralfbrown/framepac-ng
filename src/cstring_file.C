/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#include <cstring>
#include "framepac/cstring.h"
#include "framepac/file.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

bool CFile::readStringArray(class Fr::CString*& strings, size_t& count)
{
   count = 0 ;
   uint32_t total_length ;
   uint32_t num_strings ;
   if (!readValue(&total_length) || !readValue(&num_strings))
      return false ;
   char* buf = new char[total_length] ;
   if (!buf || !read(buf,total_length))
      {
      delete[] buf ;
      return false ;
      }
   strings = new CString[num_strings] ;
   if (!strings)
      {
      delete[] buf ;
      return false ;
      }
   for (size_t i = 0 ; i < num_strings ; ++i)
      {
      strings[i] = buf ;
      buf = strchr(buf,'\0') + 1 ;
      }
   count = num_strings ;
   return true ;
}

//----------------------------------------------------------------------------

bool CFile::writeStringArray(const class Fr::CString* strings, size_t count)
{
   uint64_t total_length { 0 } ;
   uint64_t num_strings { (uint64_t)count } ;
   for (size_t i = 0 ; i < count ; ++i)
      {
      ++total_length ;			// account for terminating NUL character
      if (strings[i].str())
	 total_length += strlen(strings[i].str()) ;
      }
   if (!writeValue(total_length) || !writeValue(num_strings))
      return false ;
   for (size_t i = 0 ; i < count ; ++i)
      {
      const char* s = strings[i].str() ? strings[i].str() : "" ;
      if (!write(s,strlen(s)+1))
	 return false ;
      }
   return true ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file cstring_file.C //
