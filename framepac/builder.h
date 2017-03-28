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

#ifndef _Fr_BUILDER_H_INCLUDED
#define _Fr_BUILDER_H_INCLUDED

#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

template <typename T, size_t minsize = 200>
class BufferBuilder
   {
   private:
      T        *m_buffer = m_localbuf ;
      size_t	m_alloc = minsize ;
      size_t	m_currsize = 0 ;
      T		m_localbuf[minsize] ;
   public:
      BufferBuilder() {}
      BufferBuilder(const BufferBuilder&) = delete ;
      ~BufferBuilder() ;
      void operator= (const BufferBuilder&) = delete ;

      bool preallocate(size_t newsize) ;
      void clear() ;

      void append(T value) ;

      size_t currentLength() const { return m_currsize ; }
      T *currentBuffer() const { return m_buffer ; }
      T *finalize() const ;

      // operator overloads
      T *operator * () const { return m_buffer ; }
      BufferBuilder &operator += (T value) { append(value) ; return *this ; }
   } ;

extern template class BufferBuilder<char> ;

//----------------------------------------------------------------------------

class StringBuilder : public BufferBuilder<char>
   {
   private:
   public:
      StringBuilder() {}
      ~StringBuilder() {}
      String *string() const { return String::create(currentBuffer(),currentLength()) ; }
   } ;

} // end namespace Fr

#endif /* !_Fr_BUILDER_H_INCLUDED */

// end of file builder.h //
