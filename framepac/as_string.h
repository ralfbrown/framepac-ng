/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-09-04					*/
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

#ifndef _Fr_AS_STRING_H_INCLUDED
#define _Fr_AS_STRING_H_INCLUDED

#include <cstdint>
#include "framepac/cstring.h"

namespace Fr
{

// each of the following fills in the given buffer and returns a pointer to the char following the last one filled
char* as_string(bool value, char* buf, std::size_t buflen) ;
char* as_string(int value, char* buf, std::size_t buflen) ;
char* as_string(long value, char* buf, std::size_t buflen) ;
char* as_string(unsigned int value, char* buf, std::size_t buflen) ;
char* as_string(unsigned long value, char* buf, std::size_t buflen) ;
char* as_string(std::intmax_t value, char* buf, std::size_t buflen) ;
char* as_string(std::uintmax_t value, char* buf, std::size_t buflen) ;
char* as_string(double value, char* buf, std::size_t buflen) ;
char* as_string(long double value, char* buf, std::size_t buflen) ;
char* as_string(char value, char* buf, std::size_t buflen) ;
char* as_string(const char* value, char* buf, std::size_t buflen) ;
char* as_string(const class Object* value, char* buf, std::size_t buflen) ;
char* as_string(const void*, char* buf, std::size_t buflen) ;

// each of the following returns an allocated string
CharPtr as_string(bool value) ;
CharPtr as_string(int value) ;
CharPtr as_string(long value) ;
CharPtr as_string(unsigned int value) ;
CharPtr as_string(unsigned long value) ;
CharPtr as_string(std::intmax_t value) ;
CharPtr as_string(std::uintmax_t value) ;
CharPtr as_string(double value) ;
CharPtr as_string(long double value) ;
CharPtr as_string(char value) ;
CharPtr as_string(const char* value) ;
CharPtr as_string(const class Object* value) ;
CharPtr as_string(const void* value) ;

unsigned len_as_string(bool value) ;
unsigned len_as_string(int value) ;
unsigned len_as_string(long value) ;
unsigned len_as_string(unsigned int value) ;
unsigned len_as_string(unsigned long value) ;
unsigned len_as_string(std::intmax_t value) ;
unsigned len_as_string(std::uintmax_t value) ;
unsigned len_as_string(double value) ;
unsigned len_as_string(long double value) ;
unsigned len_as_string(char value) ;
unsigned len_as_string(const char* value) ;
unsigned len_as_string(const class Object* value) ;
unsigned len_as_string(const void* value) ;

// conversion from string to a given type
template <typename T>
T string_as(const char*& s, bool& success) ;

} // end namespace Fr

#endif /* !_Fr_AS_STRING_H_INCLUDED */

// end of file as_string.h //
