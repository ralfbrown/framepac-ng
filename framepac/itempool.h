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
#include <stdexcept>
#include "framepac/atomic.h"
#include "framepac/file.h"

namespace Fr
{
   
/************************************************************************/
/************************************************************************/

template <typename T, unsigned chunksize>
class ItemPoolIter ;			// forward declaration, as we need def of ItemPool

// (header-only) class for allocate-only management of a smallish pool
//   of items of a given type.  The pool can optionally be pointed at
//   an external area such as a memory-mapped file, in which case any
//   requests to allocate additional items will result in the contents
//   of the memory-mapped file being copied into an allocated buffer

template <typename T>
class ItemPoolFlat
   {
   public:
      typedef T* iter_type ;
      typedef const T* const_iter_type ;
      
   public:
      ItemPoolFlat(size_t init_cap = 0)
	 : m_capacity(0) { if (init_cap) resize(init_cap) ; }
      ~ItemPoolFlat() { reset() ; }

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
	    m_items = base ;
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
	    if (N == 0) return true ;		// trivially successful
	    auto start = allocBatch(N) ;
	    bool success = m_items && f.read(m_items+start,N,sizeof(T)) == N ;
	    if (success) m_size += N ;
	    return success ;
	 }
      
      bool save(CFile& f) const
	 {
	    if (size() == 0) return true ; 	// trivially successful
	    return !m_items || f.write(m_items,m_size,sizeof(T)) == m_size ;
	 }
      
      explicit operator bool () const { return capacity() > 0 ; }
      bool operator ! () const { return capacity() == 0 || !m_items ; }

      // access to the allocated items
      T& operator[] (size_t N) const
	 { if (N >= size())
	       throw std::out_of_range("ItemPoolFlat") ;
           return m_items[N] ;
	 }
      T* item(size_t N) const { return N < size() ? &m_items[N] : nullptr ; }

      // iterator support
      iter_type begin() const { return m_items ; }
      const_iter_type cbegin() const { return m_items ; }
      iter_type end() const { return m_items + m_size ; }
      const_iter_type cend() const { return m_items + m_size ; }

   protected:
      void reset()  // set pool back to initial empty state
	 {
	    if (m_extdata == 0)
	       delete[] m_items ;
	    m_items = nullptr ;
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
	       if (cap && m_items)
		  std::copy_n(m_items,cap,new_items) ;
//FIXME	       if (m_ownbuf)
		  delete[] m_items ;
	       m_items = new_items ;
	       m_capacity = new_cap ;
	       m_extdata = 0 ;
	       }
	 }
   protected:
      T*          m_items { nullptr } ;	// the buffer for the items (if chunksize==0)
      atom_size_t m_capacity ;		// size of the allocated array
      atom_size_t m_size { 0 } ;	// number of items actually in use
      size_t      m_extdata { 0 } ;	// how many items are stored in an external buffer we don't own?
      static std::mutex s_mutex ;	// lock for use when resizing m_items
   } ;

template <typename T>
std::mutex ItemPoolFlat<T>::s_mutex ;

//----------------------------------------------------------------------

template <typename T, unsigned chunksize=64>
class ItemPool
   {
   public:
      typedef ItemPoolIter<T,chunksize> iter_type ;
      typedef const iter_type const_iter_type ;
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
	    // allocate chunk list and point each at the appropriate segment of the external buffer
	    m_chunks = new Chunk*[(N/chunksize)+1] ;
	    for (size_t i = 0 ; i <= N/chunksize ; ++i)
	       {
	       m_chunks[i] = reinterpret_cast<Chunk*>(base) + i ;
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
	    if (N == 0) return true ;		// trivially successful
	    auto start = allocBatch(N) ;
	    bool success = true;
	    //assert(start == 0); //TODO: handle load into non-empty pool
	    for (size_t i = 0 ; success && i < N/chunksize ; ++i)
	       {
	       success = f.read(m_chunks[i],chunksize,sizeof(T)) == chunksize ;
	       }
	    unsigned remain = N % chunksize ;
	    if (success && remain)
	       success = f.read(m_chunks[N/chunksize],remain,sizeof(T)) == remain ;
	    if (success) m_size += N ;
	    return success ;
	 }
      
      bool save(CFile& f) const
	 {
	    if (size() == 0) return true ; 	// trivially successful
	    bool success = true ;
	    for (size_t i = 0 ; success && i < size()/chunksize ; ++i)
	       {
	       success = f.write(m_chunks[i],chunksize,sizeof(T)) == chunksize ;
	       }
	    unsigned remain = size()%chunksize ;
	    if (remain && success)
	       success = f.write(m_chunks[size()/chunksize],remain,sizeof(T)) == remain ;
	    return success ;
	 }
      
      explicit operator bool () const { return capacity() > 0 ; }
      bool operator ! () const { return capacity() == 0 || !m_chunks ; }

      // access to the allocated items
      T& operator[] (size_t N) const
	 { if (N >= size())
	       throw std::out_of_range("ItemPool") ;
           return m_chunks[N/chunksize]->m_items[N%chunksize] ;
	 }
      T* item(size_t N) const
	 { return N < size() ? &m_chunks[N/chunksize]->m_items[N%chunksize] : nullptr ; }

      // iterator support
      iter_type begin() const { return iter_type(this) ; }
      const_iter_type cbegin() const { return iter_type(this) ; }
      iter_type end() const { return iter_type(this,m_size.load()) ; }
      const_iter_type cend() const { return iter_type(this,m_size.load()) ; }

   protected:
      void reset()  // set pool back to initial empty state
	 {
	    if (m_size > m_extdata)
	       {
	       for (size_t i = m_extdata/chunksize ; i < (m_size+chunksize-1)/chunksize ; ++i)
		  {
		  delete m_chunks[i] ;
		  }
	       delete[] m_chunks ;
	       m_chunks = nullptr ;
	       }
	    m_size = 0 ;
	    m_capacity = 0 ;
	    m_extdata = 0 ;
	    return ;
	 }
      void resize(size_t new_cap)
	 {
	    Chunk** new_chunks = new Chunk*[(new_cap/chunksize)+1] ;
	    if (new_chunks)
	       {
	       size_t cap = m_capacity ;
	       if (cap && m_chunks)
		  {
//FIXME:	  std::copy_n(m_data.m_items,cap,new_items) ;
		  }
//FIXME	       if (m_ownbuf)
		  delete[] m_chunks ;
	       m_chunks = new_chunks ;
	       m_capacity = new_cap ;
	       m_extdata = 0 ;
	       }
	 }
   protected:
      Chunk**     m_chunks ;		// the list of chunks
      atom_size_t m_capacity ;		// size of the allocated array
      atom_size_t m_size { 0 } ;	// number of items actually in use
      size_t      m_extdata { 0 } ;	// how many items are stored in an external buffer we don't own?
      static std::mutex s_mutex ;	// lock for use when resizing m_items
   } ;

template <typename T, unsigned chunksize>
std::mutex ItemPool<T,chunksize>::s_mutex ;

//----------------------------------------------------------------------

template <typename T, unsigned chunksize>
class ItemPoolIter
   {
   public:
      typedef ItemPool<T,chunksize> Pool ;
   public:
      ItemPoolIter(const Pool* pool, size_t index = 0) : m_pool(pool), m_index(index) {}
      ItemPoolIter(const ItemPoolIter&) = default ;
      ~ItemPoolIter() = default ;
      ItemPoolIter& operator= (const ItemPoolIter&) = default ;

      const Pool* pool() const { return m_pool ; }
      size_t currIndex() const { return m_index ; }

      T* operator* () const { return m_pool->item(m_index) ; }
      T* operator-> () const { return m_pool->item(m_index) ; }
      //operator T* () const { return m_pool->item(m_index) ; }
      T& operator[] (size_t N) const { return *m_pool->item(m_index) ; }
      ItemPoolIter& operator++() { ++m_index ; return *this ; }
      ItemPoolIter& operator--() { --m_index ; return *this ; }
      ItemPoolIter& operator+=(size_t N) { m_index += N ; return *this ; }
      ItemPoolIter& operator-=(size_t N) { m_index -= N ; return *this ; }
      bool operator== (const ItemPoolIter& other) const
	 { return m_index == other.m_index && m_pool == other.m_pool ; }
      bool operator!= (const ItemPoolIter& other) const
	 { return m_index != other.m_index || m_pool != other.m_pool ; }
   protected:
      const Pool* m_pool ;
      size_t      m_index ;
   } ;

template <typename T, unsigned chunksize>
std::ptrdiff_t operator- (const ItemPoolIter<T,chunksize>& one, const ItemPoolIter<T,chunksize>& two)
{ return one.currIndex() - two.currIndex() ; }

} // end namespace Fr

#endif /* !_Fr_ITEMPOOL_H_INCLUDED */

// end of file itempool.h //
