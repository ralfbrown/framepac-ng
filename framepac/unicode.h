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

#ifndef _Fr_UNICODE_H_INCLUDED
#define _Fr_UNICODE_H_INCLUDED

/************************************************************************/
/************************************************************************/

namespace Fr
{

wchar_t UTF8_to_codepoint(char*& buffer) ;
int UTF8_to_codepoints(const char*& buffer, wchar_t* codepoints) ;

int Unicode_to_UTF8(wchar_t codepoint, char *buffer, bool &byteswap, bool use_surrogates = false) ;
int Unicode_surrogates_to_UTF8(wchar_t codepoint1, wchar_t codepoint2, char *buffer, bool &byteswap) ;
	// returns number of bytes of buffer used (1-4)
int Unicode_to_UTF8(const wchar_t* codepoints, unsigned numcodepoints, char *buffer, bool &byteswap) ;
int Unicode_to_UTF8(const wchar_t* codepoints, char *buffer, bool &byteswap) ;
	// 'codepoints' is a null-terminated array

} // end namespace Fr

//----------------------------------------------------------------------

#endif /* !_Fr_UNICODE_H_INCLUDED */

// end of file unicode.h //

