/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-07					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#include <cstdint>
#include "framepac/unicode.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------

int UTF8_codepoint_size(const char* utf8_string)
{
   auto utf8 = reinterpret_cast<const uint8_t*>(utf8_string) ;
   auto cp = utf8[0] ;
   if (cp < 0x80) return 1 ;		// 7-bit ASCII character
   unsigned bytes ;
   if ((cp & 0xE0) == 0xC0)
      bytes = 2 ;
   else if ((cp & 0xF0) == 0xE0)
      bytes = 3 ;
   else if ((cp & 0xF8) == 0xF0)
      bytes = 4 ;
   else if ((cp & 0xFC) == 0xF8)
      bytes = 5 ;
   else if ((cp & 0xFE) == 0xFE)
      bytes = 6 ;
   else
      return -1 ;			// invalid UTF8
   for (unsigned i = 1 ; i < bytes ; ++i)
      {
      if ((cp & 0xC0) != 0x80)
	 return -1 ;			// invaliid UTF8 - not a continuation byte
      }
   return bytes ;
}

//----------------------------------------------------------------------

wchar_t UTF8_to_codepoint(const char*& buffer)
{
   if (!buffer)
      return 0 ;
   auto utf8 = reinterpret_cast<const uint8_t*>(buffer) ;
   auto cp = utf8[0] ;
   if (cp < 0x80)
      {
      ++buffer ;
      return (wchar_t)cp ;
      }
   wchar_t code ;
   unsigned bytes ;
   if ((cp & 0xE0) == 0xC0)
      {
      bytes = 2 ;
      code = (cp & 0x1F) ;
      }
   else if ((cp & 0xF0) == 0xE0)
      {
      bytes = 3 ;
      code = (cp & 0x0F) ;
      }
   else if ((cp & 0xF8) == 0xF0)
      {
      bytes = 4 ;
      code = (cp & 0x07) ;
      }
   else if ((cp & 0xFC) == 0xF8)
      {
      bytes = 5 ;
      code = (cp & 0x03) ;
      }
   else if ((cp & 0xFE) == 0xFC)
      {
      bytes = 6 ;
      code = (cp & 0x01) ;
      }
   else
      return (wchar_t)-1 ;
   for (unsigned i = 1 ; i < bytes ; ++i)
      {
      if ((utf8[i] & 0xC0) != 0x80)
	 return (wchar_t)-1 ;
      code = (code << 6) | (utf8[i] & 0x3F) ;
      }
   buffer += bytes ;
   return code ;
}

//----------------------------------------------------------------------

int UTF8_to_codepoints(const char*& buffer, wchar_t* codepoints)
{
   if (!buffer)
      return -1 ;
   int count = 0 ;
   while (*buffer)
      {
      int cnt = UTF8_codepoint_size(buffer) ;
      if (cnt < 0)
	 return -1 ;			// invalid UTF8 encountered
      ++count ;
      if (codepoints)
	 {
	 // convert and store the codepoint in the output buffer
	 *codepoints++ = UTF8_to_codepoint(buffer) ;
	 }
      else
	 {
	 // just advance the buffer pointer, don't store anything
	 buffer += cnt ;
	 }
      }
   return count ;
}

//----------------------------------------------------------------------

int Unicode_to_UTF8(wchar_t cp, char *buffer, bool &byteswap, bool use_surrogates)
{
   if (!buffer)
      {
      // count the required number of bytes, but don't store anything
      if (cp < 0x80) return 1 ;
      if (cp < 0x800) return 2 ;
      if (cp < 0x10000) return 3 ;
      if (use_surrogates) return cp < (wchar_t)(0x10000 + (1U<<24)) ? 6 : -1 ;
      if (cp < 0x200000) return 4 ;
      if (cp < 0x40000000) return 5 ;
      return 6 ;
      }
   if (cp >= 0xD800 && cp < 0xE000)
      {
      // this is a surrogate; need to get a second 16-bit codepoint and call
      // the two-arg version of this function
      return -1 ;
      }
   if (cp < 0x80)
      {
      // 7-bit ASCII stays unchanged
      *buffer = (char)cp ;
      return cp ? 1 : 0 ;
      }
   if (cp < 0x800)
      {
      // encode as two bytes
      buffer[0] = (unsigned char)(0xC0 | ((cp & 0x07C0) >> 6)) ;
      buffer[1] = (unsigned char)(0x80 | (cp & 0x003F)) ;
      return 2 ;
      }
   if (cp < 0x10000)
      {
      if (cp == 0xFFFE)
	 {
	 byteswap = !byteswap ;
	 cp = 0xFEFF ;
	 }
      // encode as three bytes
      buffer[0] = (unsigned char)(0xE0 | ((cp & 0xF000) >> 12)) ;
      buffer[1] = (unsigned char)(0x80 | ((cp & 0x0FC0) >> 6)) ;
      buffer[2] = (unsigned char)(0x80 | (cp & 0x003F)) ;
      return 3 ;
      }
   if (use_surrogates)
      {
      if (cp >= (wchar_t)(0x10000 + (1U<<24)))
	 return -1 ;			// can't encode that high with surrogates
      //TODO: encode as two 16-bit surrogate codepoints instead of one 32-bit codepoint
      }
   if (cp < 0x200000)
      {
      // encode as four bytes
      buffer[0] = (unsigned char)(0xF0 | ((cp & 0x1C0000) >> 18)) ;
      buffer[1] = (unsigned char)(0x80 | ((cp & 0x03F000) >> 12)) ;
      buffer[2] = (unsigned char)(0x80 | ((cp & 0x000FC0) >> 6)) ;
      buffer[3] = (unsigned char)(0x80 | (cp & 0x003F)) ;
      return 4 ;
      }
   if (cp < 0x4000000)
      {
      // encode as five bytes
      buffer[0] = (unsigned char)(0xF8 | ((cp & 0x3000000) >> 24)) ;
      buffer[1] = (unsigned char)(0x80 | ((cp & 0x0FC0000) >> 18)) ;
      buffer[2] = (unsigned char)(0x80 | ((cp & 0x003F000) >> 12)) ;
      buffer[3] = (unsigned char)(0x80 | ((cp & 0x0000FC0) >> 6)) ;
      buffer[4] = (unsigned char)(0x80 | (cp & 0x003F)) ;
      return 5 ;
      }
   else
      {
      // encode as six bytes
      buffer[0] = (unsigned char)(0xFC | ((cp & 0x80000000) >> 30)) ;
      buffer[1] = (unsigned char)(0x80 | ((cp & 0x7F000000) >> 24)) ;
      buffer[2] = (unsigned char)(0x80 | ((cp & 0x00FC0000) >> 18)) ;
      buffer[3] = (unsigned char)(0x80 | ((cp & 0x0003F000) >> 12)) ;
      buffer[4] = (unsigned char)(0x80 | ((cp & 0x00000FC0) >> 6)) ;
      buffer[5] = (unsigned char)(0x80 | (cp & 0x003F)) ;
      return 6 ;
      }
}

//----------------------------------------------------------------------

int Unicode_surrogates_to_UTF8(wchar_t cp1, wchar_t cp2, char *buffer, bool &byteswap)
{
   if (cp1 >= 0xD800 && cp1 < 0xE000)
      {
      // we have a surrogate pair
      wchar_t cp ;
      if (cp1 >= 0xDC00)
	 {
	 // low surrogate, so assume cp2 is high surrogate
	 cp = 0x10000 + (((cp2 & 0x03FF) << 12) | (cp1 & 0x03FF)) ;
	 }
      else
	 {
	 // high surrogate, so assume cp2 is low surrogate
	 cp = 0x10000 + (((cp1 & 0x03FF) << 12) | (cp2 & 0x03FF)) ;
	 }
      return Unicode_to_UTF8(cp,buffer,byteswap) ;
      }
   if (cp2 >= 0xD800 && cp2 < 0xE000)
      {
      // second codepoint is a surrogate, but first wasn't --> error
      return -1 ;
      }
   // we have two non-surrogate codepoints, so encode both of them normally
   int len1 = Unicode_to_UTF8(cp1,buffer,byteswap) ;
   return len1 + Unicode_to_UTF8(cp2,buffer+len1,byteswap) ;
}

//----------------------------------------------------------------------

int Unicode_to_UTF8(const wchar_t* cps, unsigned num_cps, char *buffer, bool &byteswap)
{
   int count = 0 ;
   while (num_cps > 0)
      {
      --num_cps ;
      int cnt = Unicode_to_UTF8(*cps,buffer,byteswap) ;
      if (cnt == -1)
	 {
	 if (num_cps > 0)
	    cnt = Unicode_surrogates_to_UTF8(cps[0],cps[1],buffer,byteswap) ;
	 if (cnt == -1)
	    return -1 ;
	 }
      count += cnt ;
      if (buffer)			// don't advance if NULL buffer (we only want count)
	 buffer += cnt ;
      }
   return count ;
}

//----------------------------------------------------------------------

int Unicode_to_UTF8(const wchar_t* cps, char *buffer, bool &byteswap)
{
   if (!cps)
      return 0 ;
   unsigned count = 0 ;
   while (cps[count]) ++count ;
   return Unicode_to_UTF8(cps,count,buffer,byteswap) ;
}

//----------------------------------------------------------------------



} // end namespace Fr

// end of file unicode.C //
