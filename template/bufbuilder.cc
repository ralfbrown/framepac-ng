/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-31					*/
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

#include "framepac/builder.h"
#include "framepac/convert.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename T, size_t minsize>
BufferBuilder<T,minsize>::~BufferBuilder()
{
   if (m_buffer != m_localbuf)
      delete[] m_buffer ;
   m_currsize = 0 ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
bool BufferBuilder<T,minsize>::preallocate(size_t newsize)
{
   T* newbuf { new T[newsize] };
   if (newbuf)
      {
      for (size_t i = 0 ; i < m_currsize ; i++)
	 {
	 newbuf[i] = m_buffer[i] ;
	 }
      if (m_buffer != m_localbuf)
	 {
	 delete[] m_buffer ;
	 }
      m_buffer = newbuf ;
      m_alloc = newsize ;
      return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::clear()
{
   // reset the buffer to be the initial buffer
   if (m_buffer != m_localbuf)
      {
      delete[] m_buffer ;
      }
   m_buffer = m_localbuf ;
   m_alloc = minsize ;
   m_currsize = 0 ;
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
   if (buf == m_localbuf)
      {
      // we need to make a copy of the buffer
      buf = new T[this->capacity()] ;
      for (size_t i = 0 ; i < m_currsize ; ++i)
	 {
	 buf[i] = m_buffer[i] ;
	 }
      }
   // reset our buffer
   m_buffer = m_localbuf ;
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
      preallocate(2*m_currsize) ;
      }
   m_buffer[m_currsize++] = value ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::append(const BufferBuilder<T,minsize>& addbuf)
{
   size_t grow = addbuf.size() ;
   if (m_currsize + grow > m_alloc)
      {
      size_t newalloc = (grow > m_currsize) ? 2 * (m_currsize + grow) : (2 * m_currsize) ;
      preallocate(newalloc) ;
      }
   for (size_t i = 0 ; i < grow ; ++i)
      {
      m_buffer[m_currsize++] = addbuf[i] ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
size_t BufferBuilder<T,minsize>::reserveElements(size_t count)
{
#if __cplusplus > 201400
   // get a write lock, since we may change the buffer address
   std::lock_guard<std::shared_timed_mutex> lock(m_buffer_lock) ;
#else
   std::lock_guard<std::mutex> lock(m_buffer_lock) ;
#endif /* C++14 */
   size_t currsize = size() ;
   if (currsize + count > m_alloc)
      {
      // reallocate the buffer
      size_t newalloc = (count > m_currsize) ? 2 * (m_currsize + count) : (2 * m_currsize) ;
      preallocate(newalloc) ;
      }
   m_currsize += count ;
   return currsize ;			// index of first reserved element
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
void BufferBuilder<T,minsize>::setElement(size_t N, T& value)
{
   // rwlock on buffer to prevent a concurrent reserveElements from moving it while we're writing
#if __cplusplus > 201400
   // get a read lock, since we won't modify the buffer address, even though we are modifying its contents
   std::shared_lock<std::shared_timed_mutex> lock(m_buffer_lock) ;
#else
   std::lock_guard<std::mutex> lock(m_buffer_lock) ;
#endif /* C++14 */
   m_buffer[N] = value ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
T *BufferBuilder<T,minsize>::finalize() const
{
   T *finalbuf { new T[m_currsize] };
   for (size_t i = 0 ; i < m_currsize ; i++)
      {
      finalbuf[i] = m_buffer[i] ;
      }
   return finalbuf ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !Fr_BUFBUILDER_CC_INCLUDED */

// end of file template/bufbuilder.cc //
