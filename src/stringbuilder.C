/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-13					*/
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

#include <memory.h>
#include "framepac/file.h"
#include "framepac/stringbuilder.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class StringBuilder					*/
/************************************************************************/

// constructor that builds a line by reading the given file
StringBuilder::StringBuilder(CFile* file, size_t maxline)
{
   if (!file || file->eof())
      return ;
   size_t count = 0 ;
   int c ;
   for ( c = file->getc() ; c != EOF && c != '\r' && c != '\n' && count < maxline ; c = file->getc())
      {
      append((char)c) ;
      }
   if (c == '\r')
      {
      // check for CRLF sequence
      c = file->getc() ;
      if (c != EOF && c != '\n')
	 file->ungetc(c) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void StringBuilder::append(const char *s)
{
   if (s)
      {
      while (*s)
	 append(*s++) ;
      }
   return ;
}

//----------------------------------------------------------------------------

CharPtr StringBuilder::c_str() const
{
   size_t len = currentLength() ;
   CharPtr s(len+1,currentBuffer(),len) ;
   s[len] = '\0' ;
   return s ;
}

} // end namespace Fr

// end of file stringbuilder.C //
