/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-05					*/
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
#include <cstring>
#include <cwchar>
#include <algorithm>
#include <functional>
#include <locale>
#include "framepac/texttransforms.h"

using namespace std ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

static std::locale utf8locale("en_US.UTF-8") ;

/************************************************************************/
/************************************************************************/

void lowercase_string(char* s)
{
   if (!s)
      return ;
   // see http://en.cppreference.com/w/cpp/locale/ctype/tolower
   char *last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(utf8locale).tolower(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

void uppercase_string(char* s)
{
   if (!s)
      return ;
   char *last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(utf8locale).toupper(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

#if 0
//FUTURE:  not supported in G++ 4.9
//#include <codecvt> //C++11 standard
#include <bits/codecvt.h>  // needed in G++ 4.9, since on <codecvt> header
// for a codepoint-wise case conversion, we need to convert to a wide-char strings,
//   apply toupper to each of the wide characters, then convert back to UTF8. And
//   the result is likely to differ in size, so we can't do it in place...
std::string lowercase_utf8_string(char* s)
{
   std::string utf8(s) ;
   std::wstring_convert<std::codecvt_utf8<char16_t> > converter ;
   std::wstring widestr = converter.from_bytes(utf8) ;
   for (auto& c : widestr)
      {
      c = std::toupper(c, utf8locale) ;
      }
   return converter.to_bytes(widestr) ;
}
#endif

} // end namespace Fr

// end of file texttransforms.C //
