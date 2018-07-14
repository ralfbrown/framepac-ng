/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-13					*/
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

namespace Fr
{

// each of the following returns an allocated string which must be released with delete[]
char* as_string(long value) ;
char* as_string(unsigned long value) ;
char* as_string(double value) ;
char* as_string(char value) ;
char* as_string(const char* value) ;
char* as_string(const class Object* value) ;

} // end namespace Fr

#endif /* !_Fr_AS_STRING_H_INCLUDED */

// end of file as_string.h //