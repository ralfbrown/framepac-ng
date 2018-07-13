/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-12					*/
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

#include "framepac/builder.h"
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

//----------------------------------------------------------------------------

char* CharGetter::getLine()
{
   BufferBuilder<char> buf ;

   int nextch ;
   while ((nextch = get()) != EOF && nextch != '\n')
      {
      buf += (char)nextch ;
      }
   return buf.move() ;
}


/************************************************************************/
/*	Methods for class CharGetterStream				*/
/************************************************************************/

bool CharGetterStream::rewind()
{
   // clear eof flag, otherwise the seek will fail
   m_stream.clear(m_stream.eofbit) ;
   // set the stream 'get' pointer back to zero
   m_stream.seekg(0) ;
   // let caller know whether the seek was successful
   return !m_stream.fail() ;
}

/************************************************************************/
/*	Methods for class CharGetterFILE				*/
/************************************************************************/

bool CharGetterFILE::rewind()
{
   return fseek(m_stream,0,SEEK_SET) == 0 ;
}

//----------------------------------------------------------------------------

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
