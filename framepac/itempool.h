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
// This flat-mapped version of the pool is not thread-safe, as an
//   allocation may cause all pointers to items in the pool to be
//   invalidated.

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
	    // allocate an index in the array
	    size_t idx = m_size++ ;
	    if (idx >= m_capacity)
	       {
	       // array was full, so we must resize
	       // compute the new capacity, then ask to increase to that capacity
	       size_t cap = m_capacity ;
	       size_t new_cap = cap < 32 ? 32 : (cap < 65536 ? 2*cap : 3*cap/2) ;
	       resize(new_cap) ;
	       }
	    return idx ;
	 }
      void release(size_t index)
	 {
	    // We can release the most recently allocated item.  If any others have been allocated since,
	    //   this call simply does nothing, and the item goes to waste.
	    if (index+1 == m_size)
	       --m_size ;
	 }
      bool external_buffer(T* base, size_t N)
	 {
	    if (!base) return false ;
	    reset() ;			// remove any existing items
	    m_items = base ;
	    m_extdata = m_size = m_capacity = N ;
	    return true ;
	 }
      bool external_buffer(const void* base, size_t N)
	 { return external_buffer(const_cast<T*>(reinterpret_cast<const T*>(base)),N) ; }

      size_t size() const { return m_size ; }
      size_t capacity() const { return m_capacity ; }
      void reserve(size_t new_cap)
	 {
	    if (new_cap > m_capacity)
	       {
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
	       size_t cap = std::min(m_capacity,new_cap) ;
	       if (cap && m_items)
		  std::copy_n(m_items,cap,new_items) ;
	       if (m_extdata == 0)
		  delete[] m_items ;
	       m_items = new_items ;
	       m_capacity = new_cap ;
	       m_extdata = 0 ;
	       }
	 }
   protected:
      T*     m_items { nullptr } ;	// the buffer for the items
      size_t m_capacity ;		// size of the allocated array
      size_t m_size { 0 } ;		// number of items actually in use
      size_t m_extdata { 0 } ;		// how many items are stored in an external buffer we don't own?
   } ;

//----------------------------------------------------------------------

// (header-only) class for allocate-only management of a smallish pool
//   of items of a given type.  The pool can optionally be pointed at
//   an external area such as a memory-mapped file, in which case any
//   requests to allocate additional items will result in the contents
//   of the memory-mapped file being copied into an allocated buffer
// This chunked version is thread-safe, as items do not move on a
//   reallocation (unless there is a partial final chunk in the
//   external storage).

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
	    // make sure we have allocated a chunk for the new item
	    if (m_chunks[idx/chunksize] == nullptr)
	       {
	       Chunk* chunk = new Chunk ;
	       // atomically swap the new chunk into the array
	       Chunk* expected = nullptr ;
	       if (!Atomic<Chunk*>::ref(m_chunks[idx/chunksize]).compare_exchange_strong(expected,chunk))
		  delete chunk ;
	       }
	    return idx ;
	 }
      void release(size_t index)
	 {
	    // we can release the last item allocated; if any more have been allocated since, the
	    //   request is ignored and that item simply goes to waste
	    // use CAS to atomically decrement size only if it is exactly one more than the given index.
	    size_t expected = index+1 ;
	    m_size.compare_exchange_weak(expected,index) ;
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
      bool external_buffer(const void* base, size_t N)
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
	 for (size_t i = chunk_count(currsize) ; i < chunk_count(newsize) ; ++i)
	    {
	    if (m_chunks[i] != nullptr) continue ;
	    auto chunk = new Chunk ;
	    // atomically swap the new chunk into the array
	    Chunk* expected = nullptr ;
	    if (!Atomic<Chunk*>::ref(m_chunks[i]).compare_exchange_strong(expected,chunk))
	       delete chunk ;
	    }
	 return currsize ;
	 }
      void clear() { m_size = 0 ; }
      
      bool load(CFile& f, size_t N)
	 {
	    if (N == 0) return true ;		// trivially successful
	    auto start = allocBatch(N) ;
	    bool success = true;
	    N += start ;			// adjust end to account for items already in pool
	    unsigned partial = (chunksize - (start % chunksize)) % chunksize ;
	    if (partial)
	       {
	       // read a partial chunk if the pool already had a partial last chunk
	       success = f.read(m_chunks[start/chunksize]+(start%chunksize),partial,sizeof(T)) == partial ;
	       start += partial ;
	       }
	    // read full chunks
	    for (size_t i = start ; success && i < N/chunksize ; ++i)
	       {
	       success = f.read(m_chunks[i],chunksize,sizeof(T)) == chunksize ;
	       }
	    // read any left-over in a partial last chunk
	    unsigned remain = N % chunksize ;
	    if (success && remain)
	       success = f.read(m_chunks[N/chunksize],remain,sizeof(T)) == remain ;
	    if (!success) m_size = start ; // shrink back to original size on failure
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
      static constexpr size_t chunk_count(size_t cap) { return (cap+chunksize-1) / chunksize ; }
      void reset()  // set pool back to initial empty state
	 {
	    if (m_size > m_extdata)
	       {
	       for (size_t i = chunk_count(m_extdata) ; i < chunk_count(m_size) ; ++i)
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
	    size_t new_count = chunk_count(new_cap) ;
	    size_t old_count = chunk_count(m_capacity) ;
	    new_cap = new_count * chunksize ;
	    if (new_count != old_count)
	       {			// no need to reallocate, just adjust our record of sizes
	       Chunk** new_chunks = new Chunk*[new_count] ;
	       if (!new_chunks)
		  return ; 		// don't change anything if we could not allocate memory
	       else
		  {
		  size_t min_count = std::min(old_count,new_count) ;
		  if (min_count && m_chunks)
		     {
		     std::copy_n(m_chunks,min_count,new_chunks) ;
		     }
		  if (new_count > old_count)
		     {
		     // zero out any new chunk pointers
		     std::fill_n(new_chunks + old_count, new_count - old_count, nullptr) ;
		     }
		  else if (new_count < old_count)
		     {
		     // free up any chunks which are no longer used
		     for (size_t i = new_count ; i < old_count ; ++i)
			delete m_chunks[i] ;
		     }
		  delete[] m_chunks ;
		  m_extdata = 0 ;
		  m_chunks = new_chunks ;
		  }
	       }
	    m_capacity = new_cap ;
	    if (new_cap < m_size) m_size = new_cap ;
	    return ;
	 }
   protected:
      Chunk**     m_chunks { nullptr } ;// the list of chunks
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
