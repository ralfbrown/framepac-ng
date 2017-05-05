/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-05					*/
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

#ifndef _Fr_TEXTTRANSFORMS_H_INCLUDED
#define _Fr_TEXTTRANSFORMS_H_INCLUDED

#include <locale>

namespace Fr
{

void lowercase_string(char*) ;
void uppercase_string(char*) ;

void lowercase_string(char*, std::locale&) ;
void uppercase_string(char*, std::locale&) ;

[[gnu::format(gnu_printf,1,0)]]
char* vaprintf(const char *fmt, va_list args) ;

[[gnu::format(gnu_printf,1,2)]]
char* aprintf(const char* fmt, ...) ;

} // end namespace Fr



#endif /* !_Fr_TEXTTRANSFORMS_H_INCLUDED */

// end of file texttransforms.h

