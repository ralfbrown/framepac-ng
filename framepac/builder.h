/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#ifndef _Fr_BUILDER_H_INCLUDED
#define _Fr_BUILDER_H_INCLUDED

#include "framepac/config.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

template <typename T, size_t minsize = 200>
class BufferBuilder
   {
   public:
      BufferBuilder() {}
      BufferBuilder(const BufferBuilder&) = delete ;
      ~BufferBuilder() ;
      void operator= (const BufferBuilder&) = delete ;

      bool preallocate(size_t newsize) ;
      void clear() ;

      bool read(const char*&) ;
      bool read(char*& input) { return read(const_cast<char*&>(input)) ; }
      void append(T value) ;
      void append(const BufferBuilder& value) ;
      void remove() { if (m_currsize > 0) --m_currsize ; } // remove last-added item

      size_t currentLength() const { return m_currsize ; }
      size_t size() const { return m_currsize ; }
      size_t capacity() const { return m_alloc ; }
      T* currentBuffer() const { return m_buffer ; }
      T* finalize() const ;
      T* move() ;

      // iterator support
      T* begin() const { return currentBuffer() ; }
      T* end() const { return currentBuffer() + size() ; }
      const T* cbegin() const { return begin() ; }
      const T* cend() const { return end() ; }

      // operator overloads
      T *operator * () const { return m_buffer ; }
      T operator [] (size_t N) const { return m_buffer[N] ; }
      BufferBuilder &operator += (T value) { append(value) ; return *this ; }
      BufferBuilder &operator += (const BufferBuilder& buf) { append(buf) ; return *this ; }

   protected:
      T        *m_buffer = m_localbuf ;
      size_t	m_alloc = minsize ;
      size_t	m_currsize = 0 ;
      T		m_localbuf[minsize] ;
   } ;

extern template class BufferBuilder<char> ;

} // end namespace Fr

#endif /* !_Fr_BUILDER_H_INCLUDED */

// end of file builder.h //
