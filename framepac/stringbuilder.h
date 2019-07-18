/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-17					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018,2019 Carnegie Mellon University		*/
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
#include "framepac/smartptr.h"
#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

//----------------------------------------------------------------------------

class CFile ;

class StringBuilder : public BufferBuilder<char>
   {
   public: // types
      typedef BufferBuilder<char> super ;
   public:
      StringBuilder() {}
      StringBuilder(CFile*, size_t maxlen = (size_t)~0) ;
      ~StringBuilder() {}
      using BufferBuilder<char>::append ;
      void append(const char* s) ;
      using BufferBuilder<char>::operator+= ;
      StringBuilder& operator += (const char* s) { append(s) ; return *this ; }
      StringPtr string() const { return String::create(currentBuffer(),currentLength()) ; }
      CharPtr c_str() const ;

      // iterator support
      char* begin() const { return currentBuffer() ; }
      char* end() const { return currentBuffer() + size() ; }
      const char* cbegin() const { return begin() ; }
      const char* cend() const { return end() ; }
   } ;

} // end namespace Fr

#endif /* !_Fr_STRINGBUILDER_H_INCLUDED */

// end of file stringbuilder.h //
