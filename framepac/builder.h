/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-04-18					*/
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

#ifndef _Fr_BUILDER_H_INCLUDED
#define _Fr_BUILDER_H_INCLUDED

#include <array>
#include <mutex>
#include "framepac/config.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

class CFile ;

template <typename T, size_t minsize = 0>
class BufferBuilder
   {
   public:
      BufferBuilder() ;
      BufferBuilder(const BufferBuilder&) = delete ;
      ~BufferBuilder() ;
      void operator= (const BufferBuilder&) = delete ;

      bool preallocate(size_t newsize) ;
      bool reserve(size_t newsize) { return newsize > capacity() ? preallocate(newsize) : true ; }
      void clear() ;

      bool load(CFile&, const char* filename) ;
      bool loadFromMmap(const void* mmap_base, size_t mmap_len) ;
      bool save(CFile&) const ;

      bool read(const char*&) ;
      bool read(char*& input) { return read(const_cast<char*&>(input)) ; }
      void append(T value) ;
      void append(const BufferBuilder& value) ;
      void append(T value, size_t count) ;
      void remove() { if (m_currsize > 0) --m_currsize ; } // remove last-added item
      void reverse() ;

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
      T* localBuffer() { return minsize ? m_localbuf.begin() : nullptr ; }
      const T* localBuffer() const { return minsize ? m_localbuf.begin() : nullptr ; }
      void freeBuffer() ;

   protected:
      T        *m_buffer ;
      size_t	m_currsize { 0 } ;
      size_t	m_alloc ;
      bool	m_external_buffer { false } ;
      std::array<T,minsize> m_localbuf ;

      // magic values for serializing
      static constexpr auto signature = "\x7F""BufBuild" ;
      static constexpr unsigned file_format = 1 ;
      static constexpr unsigned min_file_format = 1 ;
   } ;

//----------------------------------------------------------------------------

template <typename T, size_t minsize = 200>
class ParallelBufferBuilder : public BufferBuilder<T,minsize>
   {
   public: // types
      typedef BufferBuilder<T,minsize> super ;
   public: // methods
      ParallelBufferBuilder() : super() {}
      ParallelBufferBuilder(const ParallelBufferBuilder&) = delete ;
      ~ParallelBufferBuilder() {}
      void operator= (const ParallelBufferBuilder&) = delete ;

      // new functions to support parallel construction of the buffer
      size_t reserveElements(size_t count) ;
      void setElement(size_t N, T& value) ;

      // operator overloads
      T *operator * () const { return this->m_buffer ; }
      T operator [] (size_t N) const { return this->m_buffer[N] ; }
      ParallelBufferBuilder &operator += (T value) { this->append(value) ; return *this ; }
      ParallelBufferBuilder &operator += (const ParallelBufferBuilder& buf) { this->append(buf) ; return *this ; }
      ParallelBufferBuilder &operator += (const super& buf) { this->append(buf) ; return *this ; }

   protected:      
#if __cplusplus > 201400
      std::shared_timed_mutex m_buffer_lock ;
#else
      std::mutex m_buffer_lock ;
#endif /* C++14 or later */
   } ;

//----------------------------------------------------------------------------

extern template class BufferBuilder<char> ;

} // end namespace Fr

#endif /* !_Fr_BUILDER_H_INCLUDED */

// end of file builder.h //
