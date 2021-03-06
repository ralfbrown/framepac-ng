/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-19					*/
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

#ifndef Fr_BUFBUILDER_CC_INCLUDED
#define Fr_BUFBUILDER_CC_INCLUDED

#include <algorithm>
#include "framepac/builder.h"
#include "framepac/convert.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename T, size_t minsize>
BufferBuilder<T,minsize>::BufferBuilder()
{
   m_currsize = 0 ;
#ifdef __SANITIZE_ADDRESS__
   // don't use the local buffer under ASAN, so that it can properly track the memory
   m_alloc = 0 ;
   m_buffer = nullptr ;
#else
   m_alloc = minsize ;
   m_buffer = localBuffer() ;
#endif
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
BufferBuilder<T,minsize>::~BufferBuilder()
{
   freeBuffer() ;
   m_currsize = 0 ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::freeBuffer()
{
   if (m_buffer != localBuffer())
      {
      if (!m_external_buffer && m_buffer)
	 {
	 ASAN(__sanitizer_annotate_contiguous_container(m_buffer,m_buffer+capacity(),m_buffer+size(), \
	       m_buffer+capacity())) ;
	 delete[] m_buffer ;
	 }
      else
	 m_external_buffer = false ;
      m_currsize = 0 ;
#ifdef __SANITIZE_ADDRESS__
      m_buffer = nullptr ;
      m_alloc = 0 ;
#else
      m_buffer = localBuffer() ;
      m_alloc = minsize ;
#endif /* __SANITIZE_ADDRESS__ */
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
bool BufferBuilder<T,minsize>::preallocate(size_t newsize)
{
   T* newbuf { new T[newsize] };
   if (newbuf)
      {
      size_t sz = m_currsize ;
      std::copy(m_buffer,m_buffer+sz,newbuf) ;
      freeBuffer() ;
      m_buffer = newbuf ;
      m_currsize = sz ;
      ASAN(__sanitizer_annotate_contiguous_container(m_buffer,m_buffer+newsize,m_buffer+sz,m_buffer+sz)) ;
      m_alloc = newsize ;
      return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::clear()
{
   freeBuffer() ;
   m_buffer = localBuffer() ;
   m_alloc = minsize ;
   m_currsize = 0 ;
   m_external_buffer = false ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
bool BufferBuilder<T,minsize>::read(const char*& input)
{
   if (!input || !*input) return false ;
   T value ;
   if (!convert_string(input,value))
      return false ;
   this->append(value) ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
T* BufferBuilder<T,minsize>::move()
{
   T* buf { m_buffer } ;
   if (buf == localBuffer() && minsize > 0)
      {
      // we need to make a copy of the buffer
      buf = new T[this->capacity()] ;
      std::copy(m_buffer,m_buffer+m_currsize,buf) ;
      }
   // reset our buffer
   m_buffer = localBuffer() ;
   m_alloc = minsize ;
   m_currsize = 0 ;
   // and return the buffer that had been built
   return buf ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::append(T value)
{
   if (m_currsize >= m_alloc)
      {
      size_t newalloc = m_currsize > 200000000 ? 5*m_currsize/4 : (m_currsize > 1000000 ? 3*m_currsize/2 : 2*m_currsize+50) ;
      preallocate(newalloc) ;
      }
   ASAN(__sanitizer_annotate_contiguous_container(m_buffer,m_buffer+capacity(),m_buffer+size(),m_buffer+size()+1)) ;
   m_buffer[m_currsize++] = value ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::append(T value, size_t count)
{
   if (!count)
      return ;
   if (m_currsize + count  >= m_alloc)
      {
      size_t newalloc = m_currsize > 200000000 ? 5*m_currsize/4 : (m_currsize > 1000000 ? 3*m_currsize/2 : 2*m_currsize+50) ;
      if (newalloc < m_currsize + count)
	 newalloc = 5*(m_currsize + count)/4 ;
      preallocate(newalloc) ;
      }
   ASAN(__sanitizer_annotate_contiguous_container(m_buffer,m_buffer+capacity(),m_buffer+size(),m_buffer+size()+count));
   for (size_t i = 0 ; i < count ; ++i)
      {
      m_buffer[m_currsize++] = value ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::append(const BufferBuilder<T,minsize>& addbuf)
{
   size_t grow = addbuf.size() ;
   if (m_currsize + grow > m_alloc)
      {
      size_t newalloc = (grow > m_currsize) ? m_currsize + grow : m_currsize ;
      newalloc = newalloc > 200000000 ? 5*newalloc/4 : ((newalloc > 1000000) ? 3*newalloc/2 : 2*newalloc) ;
      preallocate(newalloc) ;
      }
   ASAN(__sanitizer_annotate_contiguous_container(m_buffer,m_buffer+capacity(),m_buffer+size(),m_buffer+size()+grow)) ;
   std::copy(addbuf.m_buffer,addbuf.m_buffer+grow,m_buffer+m_currsize) ;
   m_currsize += grow ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::reverse() 
{
   std::reverse(m_buffer,m_buffer+size()) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
T *BufferBuilder<T,minsize>::finalize() const
{
   T *finalbuf { new T[m_currsize] };
   std::copy(m_buffer,m_buffer+m_currsize,finalbuf) ;
   return finalbuf ;
}

/************************************************************************/
/*	Methods for template class ParallelBufferBuilder		*/
/************************************************************************/

template <typename T, size_t minsize>
size_t ParallelBufferBuilder<T,minsize>::reserveElements(size_t count)
{
#if __cplusplus > 201400
   // get a write lock, since we may change the buffer address
   std::lock_guard<std::shared_timed_mutex> lock(m_buffer_lock) ;
#else
   std::lock_guard<std::mutex> lock(m_buffer_lock) ;
#endif /* C++14 */
   size_t currsize = this->size() ;
   if (currsize + count > this->m_alloc)
      {
      // reallocate the buffer
      size_t newalloc = (count > currsize) ? 2 * (currsize + count) : (2 * currsize) ;
      this->preallocate(newalloc) ;
      }
   this->m_currsize += count ;
   ASAN(__sanitizer_annotate_contiguous_container(this->m_buffer,this->m_buffer+this->capacity(), \
	 this->m_buffer+currsize, this->m_buffer+this->size())) ;
   return currsize ;			// index of first reserved element
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void ParallelBufferBuilder<T,minsize>::setElement(size_t N, T& value)
{
   // rwlock on buffer to prevent a concurrent reserveElements from moving it while we're writing
#if __cplusplus > 201400
   // get a read lock, since we won't modify the buffer address, even though we are modifying its contents
   std::shared_lock<std::shared_timed_mutex> lock(m_buffer_lock) ;
#else
   std::lock_guard<std::mutex> lock(m_buffer_lock) ;
#endif /* C++14 */
   this->m_buffer[N] = value ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !Fr_BUFBUILDER_CC_INCLUDED */

// end of file template/bufbuilder.cc //
