/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
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

#include "framepac/charget.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

int CharGetter::peekNonWhite()
{
   int nextch ;
   while ((nextch = peek()) != EOF && isspace(nextch))
      {
      // consume the character
      get() ;
      }
   return nextch ;
}

//----------------------------------------------------------------------------

int CharGetter::getNonWhite()
{
   int nextch ;
   while ((nextch = get()) != EOF && isspace(nextch))
      {
      // consume the character
      }
   return nextch ;
}

/************************************************************************/
/************************************************************************/

int CharGetterFILE::peekNonWhite()
{
   int c ;
   while ((c = fgetc(m_stream)) != EOF && isspace(c))
      {
      // consume the character
      }
   if (c != EOF)
      (void)ungetc(c,m_stream) ;
   return c ;
}

//----------------------------------------------------------------------------

int CharGetterFILE::peek()
{
   int c = fgetc(m_stream) ;
   if (c != EOF)
      (void)ungetc(c,m_stream) ;
   return c ;
}

/************************************************************************/
/************************************************************************/

} // end namespace Fr

// end of file charget.C //
