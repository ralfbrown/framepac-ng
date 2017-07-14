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

#ifndef _Fr_STRINGBUILDER_H_INCLUDED
#define _Fr_STRINGBUILDER_H_INCLUDED

#include "framepac/builder.h"
#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

//----------------------------------------------------------------------------

class CFile ;

class StringBuilder : public BufferBuilder<char>
   {
   private:
   public:
      StringBuilder() {}
      StringBuilder(CFile*, size_t maxlen = (size_t)~0) ;
      ~StringBuilder() {}
      using BufferBuilder<char>::append ;
      void append(const char* s) ;
      String *string() const { return String::create(currentBuffer(),currentLength()) ; }
      char* c_str() const ;
   } ;

} // end namespace Fr

#endif /* !_Fr_STRINGBUILDER_H_INCLUDED */

// end of file stringbuilder.h //
