/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-26					*/
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

#ifndef _Fr_TEXTTRANSFORMS_H_INCLUDED
#define _Fr_TEXTTRANSFORMS_H_INCLUDED

#include <locale>
#include "framepac/cstring.h"

namespace Fr
{

CharPtr dup_string(const char*) ;
CharPtr dup_string(const char*, size_t len) ;
CharPtr dup_string_n(const char*, size_t maxlength) ;

char* skip_whitespace(char*) ;
const char* skip_whitespace(const char*) ;
char* skip_whitespace(char*, std::locale&) ;

char* skip_to_whitespace(char*) ;
const char* skip_to_whitespace(const char*) ;
char* skip_to_whitespace(char*, std::locale&) ;

char* trim_whitespace(char*) ;
char* trim_whitespace(char*, std::locale&) ;

void lowercase_string(char*) ;
void uppercase_string(char*) ;

void lowercase_string(char*, std::locale&) ;
void lowercase_string(char*, std::locale*) ;
void uppercase_string(char*, std::locale&) ;
void uppercase_string(char*, std::locale*) ;

//std::string lowercase_utf8_string(char*) ;
//std::string uppercase_utf8_string(char*) ;

CharPtr canonicalize_sentence(const char*, bool force_uppercase = false, const char* delim = nullptr,
	  		      char const* const* abbrevs = nullptr) ;
CharPtr canonicalize_sentence(const char*, std::locale&, bool force_uppercase = false,
			      const char* delim = nullptr, char const* const* abbrevs = nullptr) ;

[[gnu::format(gnu_printf,1,0)]]
CharPtr vaprintf(const char *fmt, va_list args) ;

[[gnu::format(gnu_printf,1,2)]]
CharPtr aprintf(const char* fmt, ...) ;

} // end namespace Fr



#endif /* !_Fr_TEXTTRANSFORMS_H_INCLUDED */

// end of file texttransforms.h

