/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.11, last edit 2018-09-10					*/
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

#include "framepac/cstring.h"

using namespace std ;

namespace Fr
{

bool is_number(const char* token)
{
   if (!token)
      return false ;
   if (token[0] == '+' || token[0] == '-') // skip leading plus or minus sign
      token++ ;
   if (!token[0])
      return false ;
   while (*token && isdigit(*token))
      token++ ;
   if (!token[0])			// end of string
      return true ;			// we have an integer
   if (token[0] != '.')			// if there's anything but a decimal point here,
      return false ;			//   the token is not a number
   token++ ;
   while (*token && isdigit(*token))
      token++ ;
   return !token[0] ;
}

//----------------------------------------------------------------------

bool is_punct(const char *token)
{
   if (!token)
      return false ;
   return ispunct(token[0]) && (isspace(token[1]) || token[1] == '\0') ;
}



} // end namespace Fr

// end of file is_number.C //
