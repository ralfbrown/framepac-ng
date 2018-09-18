/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-18					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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
#include "framepac/texttransforms.h"

using namespace std ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

std::locale utf8locale("en_US.UTF-8") ;

/************************************************************************/
/************************************************************************/

CharPtr dup_string(const char* s)
{
   if (!s)
      return nullptr ;
   size_t len = strlen(s) + 1 ;
   char* copy = new char[len+1] ;
   if (copy)
      std::copy(s,s+len,copy) ;
   return copy ;
}

//----------------------------------------------------------------------------

CharPtr dup_string(const char* s, size_t len)
{
   if (!s)
      return nullptr ;
   len = std::min(len,strlen(s)) ;
   char* copy = new char[len+1] ;
   if (copy)
      {
      std::copy(s,s+len,copy) ;
      copy[len] = '\0' ;
      }
   return copy ;
}

//----------------------------------------------------------------------------

CharPtr dup_string_n(const char* s, size_t len)
{
   if (!s)
      return nullptr ;
   len = std::min(len,strlen(s)) ;
   char* copy = new char[len+1] ;
   if (copy)
      {
      std::copy(s,s+len,copy) ;
      copy[len] = '\0' ;
      }
   return copy ;
}

//----------------------------------------------------------------------------

char* skip_whitespace(char* s)
{
   if (s)
      {
      while (*s && isspace(*s))
	 s++ ;
      }
   return s ;
}

//----------------------------------------------------------------------------

char* skip_whitespace(char* s, std::locale& locale)
{
   if (s)
      {
      while (*s && isspace(*s,locale))
	 s++ ;
      }
   return s ;
}

//----------------------------------------------------------------------------

const char* skip_whitespace(const char* s)
{
   if (s)
      {
      while (*s && isspace(*s))
	 s++ ;
      }
   return s ;
}

//----------------------------------------------------------------------------

char* skip_to_whitespace(char* s)
{
   if (s)
      {
      while (*s && !isspace(*s))
	 s++ ;
      }
   return s ;
}

//----------------------------------------------------------------------------

char* skip_to_whitespace(char* s, std::locale& locale)
{
   if (s)
      {
      while (*s && !isspace(*s,locale))
	 s++ ;
      }
   return s ;
}

//----------------------------------------------------------------------------

const char* skip_to_whitespace(const char* s)
{
   if (s)
      {
      while (*s && !isspace(*s))
	 s++ ;
      }
   return s ;
}

//----------------------------------------------------------------------------

char* trim_whitespace(char* s)
{
   if (s)
      {
      char* nonwhite = skip_whitespace(s) ;
      char* s_end = strchr(nonwhite,'\0') ;
      while (s_end > nonwhite && isspace(s_end[-1]))
	 s_end-- ;
      size_t len = s_end - nonwhite ;
      if (nonwhite > s)
	 std::copy(nonwhite,nonwhite+len,s) ;
      s[len] = '\0' ;
      }
   return s ;
}

//----------------------------------------------------------------------------

char* trim_whitespace(char* s, std::locale& locale)
{
   if (s)
      {
      char* nonwhite = skip_whitespace(s,locale) ;
      char* s_end = strchr(nonwhite,'\0') ;
      while (s_end > nonwhite && isspace(s_end[-1],locale))
	 s_end-- ;
      size_t len = s_end - nonwhite ;
      if (nonwhite > s)
	 std::copy(nonwhite,nonwhite+len,s) ;
      s[len] = '\0' ;
      }
   return s ;
}

//----------------------------------------------------------------------

unsigned count_words(const char *s)
{
   if (!s || !*s)
      return 0 ;
   unsigned count = 1 ;
   for ( ; *s ; s++)
      {
      if (*s == ' ')
	 count++ ;
      }
   return count ;
}

//----------------------------------------------------------------------

char *remove_quoting(char *text)
{
   text = trim_whitespace(text) ;
   size_t len = strlen(text) ;
   if (len > 1)
      {
      --len ;
      if (text[0] == '|' && text[len] == '|')
	 {
	 text[len] = '\0' ;
	 return text+1 ;
	 }
      if (text[0] == '"' && text[len] == '"')
	 {
	 size_t dest = 0 ;
	 for (size_t i = 1 ; i < len ; ++i)
	    {
	    if (text[i] == '\\')
	       ++i ;
	    text[dest++] = text[i] ;
	    }
	 text[dest] = '\0' ;
	 }
      }
   return text ;
}

//----------------------------------------------------------------------------

void lowercase_string(char* s)
{
   if (!s)
      return ;
   // see http://en.cppreference.com/w/cpp/locale/ctype/tolower
   char* last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(utf8locale).tolower(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

void lowercase_string(CharPtr& s)
{
   lowercase_string((char*)s) ;
   return ;
}

//----------------------------------------------------------------------------

void lowercase_string(char* s, std::locale& loc)
{
   if (!s)
      return ;
   // see http://en.cppreference.com/w/cpp/locale/ctype/tolower
   char* last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(loc).tolower(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

void lowercase_string(char* s, std::locale* loc)
{
   if (!s || !loc)
      return ;
   // see http://en.cppreference.com/w/cpp/locale/ctype/tolower
   char* last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(*loc).tolower(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

void lowercase_string(CharPtr& s, std::locale* loc)
{
   lowercase_string((char*)s,loc) ;
   return ;
}

//----------------------------------------------------------------------------

void uppercase_string(char* s)
{
   if (!s)
      return ;
   char* last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(utf8locale).toupper(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

void uppercase_string(CharPtr& s)
{
   uppercase_string((char*)s) ;
   return ;
}

//----------------------------------------------------------------------------

void uppercase_string(char* s, std::locale& loc)
{
   if (!s)
      return ;
   char* last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(loc).toupper(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

void uppercase_string(char* s, std::locale* loc)
{
   if (!s || !loc)
      return ;
   char* last = strchr(s,'\0') ;
   std::use_facet<std::ctype<char> >(*loc).toupper(s, last) ;
   return ;
}

//----------------------------------------------------------------------------

void uppercase_string(CharPtr& s, std::locale* loc)
{
   uppercase_string((char*)s,loc) ;
   return ;
}

//----------------------------------------------------------------------------

#if 0
//FUTURE:  not supported in G++ 4.9
//#include <codecvt> //C++11 standard
#include <bits/codecvt.h>  // needed in G++ 4.9, since no <codecvt> header
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
