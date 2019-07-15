/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#ifndef _Fr_ITEMPOOL_H_INCLUDED
#define _Fr_ITEMPOOL_H_INCLUDED

#include<algorithm>
#include <mutex>
#include "framepac/atomic.h"
#include "framepac/file.h"

namespace Fr
{
   
/************************************************************************/
/************************************************************************/

// (header-only) class for allocate-only management of a smallish pool
//   of items of a given type.  The pool can optionally be pointed at
//   an external area such as a memory-mapped file, in which case any
//   requests to allocate additional items will result in the contents
//   of the memory-mapped file being copied into an allocated buffer

template <typename T, unsigned chunksize=64>
class ItemPool
   {
   public:
      typedef T* iter_type ;
      typedef const T* const_iter_type ;
      class Chunk
	 {
	 public:
	    T m_items[chunksize] ;
	 } ;
      
   public:
      ItemPool(size_t init_cap = 0)
	 : m_capacity(0) { if (init_cap) resize(init_cap) ; }
      ~ItemPool() { reset() ; }

      size_t alloc()
	 {
	    // atomically allocate an index in the array
	    size_t idx = m_size++ ;
	    if (idx >= m_capacity)
	       {
	       // array was full, so we must resize
	       // start by grabbing the lock
	       std::lock_guard<std::mutex> _(s_mutex) ;
	       // check whether another thread beat us to the resize
	       if (m_size >= m_capacity)
		  {
		  // still need to resize, so do it
		  // compute the new capacity, then ask to increase to that capacity
		  size_t cap = m_capacity ;
		  size_t new_cap = cap < 32 ? 32 : (cap < 65536 ? 2*cap : 3*cap/2) ;
		  resize(new_cap) ;
		  }
	       }
	    return idx ;
	 }
      void release(size_t /*index*/)
	 {
	    // we can release the last item allocated; if any more have been allocated since, the
	    //   request is ignored and that item simply goes to waste
	    //if (???)
	       {
	       // TODO: use CAS to ensure that the compare and decrement happen atomically

	       }
	 }
      bool external_buffer(T* base, size_t N)
	 {
	    if (!base) return false ;
	    reset() ;			// remove any existing items
	    if (chunksize == 0)
	       m_data.m_items = base ;
	    else
	       {
	       // allocate chunk list and point each at the appropriate segment of the external buffer
	       m_data.m_chunks = new Chunk*[(N/chunksize)+1] ;
	       for (size_t i = 0 ; i <= N/chunksize ; ++i)
		  {
		  m_data.m_chunks[i] = reinterpret_cast<Chunk*>(base) + i ;
		  }
	       }
	    m_extdata = m_size = m_capacity = N ;
	    return true ;
	 }
      bool external_buffer(const char* base, size_t N)
	 { return external_buffer(const_cast<T*>(reinterpret_cast<const T*>(base)),N) ; }

      size_t size() const { return m_size ; }
      size_t capacity() const { return m_capacity ; }
      void reserve(size_t new_cap)
	 {
	    if (new_cap > m_capacity)
	       {
	       std::lock_guard<std::mutex> _(s_mutex) ;
	       resize(new_cap) ;
	       }
	 }
      void shrink(size_t newsize)  // forget about any elements past 'newsize'
	 {
	    if (newsize < m_size) m_size = newsize ;
	 }
      size_t allocBatch(size_t incr)
	 {
	 size_t currsize = size() ;
	 size_t newsize = currsize + incr ;
	 reserve(newsize) ;
	 m_size = newsize ;
	 return currsize ;
	 }
      void clear() { m_size = 0 ; }
      
      bool load(CFile& f, size_t N)
	 {
	    if (N == 0) return true ;
	    auto start = allocBatch(N) ;
	    bool success = m_data.m_items && f.read(m_data.m_items+start,N,sizeof(T)) == N ;
	    if (success) m_size += N ;
	    return success ;
	 }
      
      bool save(CFile& f) const
	 {
	    return !size() || !m_data.m_items || f.write(m_data.m_items,m_size,sizeof(T)) == m_size ;
	 }
      
      explicit operator bool () const { return capacity() > 0 ; }
      bool operator ! () const { return capacity() == 0 || !m_data.m_items ; }

      // access to the allocated items
      T& operator[] (size_t N) const { return N < size() ? m_data.m_items[N] : m_data.m_items[capacity()-1] ; }
      T* item(size_t N) const { return N < size() ? &m_data.m_items[N] : nullptr ; }

      // iterator support
      iter_type begin() const { return m_data.m_items ; }
      const_iter_type cbegin() const { return m_data.m_items ; }
      iter_type end() const { return m_data.m_items + m_size ; }
      const_iter_type cend() const { return m_data.m_items + m_size ; }

   protected:
      void reset()  // set pool back to initial empty state
	 {
	    if (chunksize == 0)
	       {
	       if (m_extdata == 0)
		  delete[] m_data.m_items ;
	       m_data.m_items = nullptr ;
	       }
	    else if (m_size > m_extdata)
	       {
	       for (size_t i = m_extdata/chunksize ; i < (m_size+chunksize-1)/chunksize ; ++i)
		  {
		  delete m_data.m_chunks[i] ;
		  }
	       delete[] m_data.m_chunks ;
	       m_data.m_chunks = nullptr ;
	       }
	    m_size = 0 ;
	    m_capacity = 0 ;
	    m_extdata = 0 ;
	    return ;
	 }
      void resize(size_t new_cap)
	 {
	    T* new_items = new T[new_cap] ;
	    if (new_items)
	       {
	       size_t cap = m_capacity ;
	       if (cap && m_data.m_items)
		  std::copy_n(m_data.m_items,cap,new_items) ;
	       if (m_ownbuf)
		  delete[] m_data.m_items ;
	       m_data.m_items = new_items ;
	       m_capacity = new_cap ;
	       m_ownbuf = true ;
	       }
	 }
   protected:
      union {
	 Chunk**  m_chunks ;		// the list of chunks (if chunksize > 0)
	 T*       m_items { nullptr } ;	// the buffer for the items (if chunksize==0)
	 } m_data ;
      atom_size_t m_capacity ;		// size of the allocated array
      atom_size_t m_size { 0 } ;	// number of items actually in use
      size_t      m_extdata { 0 } ;	// how many items are stored in an external buffer we don't own?
      bool        m_ownbuf { false } ;	// have we allocated the buffer ourselves?
      static std::mutex s_mutex ;	// lock for use when resizing m_items
   } ;

template <typename T, unsigned chunksize>
std::mutex ItemPool<T,chunksize>::s_mutex ;

} // end namespace Fr

#endif /* !_Fr_ITEMPOOL_H_INCLUDED */

// end of file itempool.h //
