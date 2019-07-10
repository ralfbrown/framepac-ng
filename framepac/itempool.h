/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-10					*/
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

namespace Fr
{
   
/************************************************************************/
/************************************************************************/

// (header-only) class for allocate-only management of a smallish pool of items
//   of a given type

template <typename T>
class ItemPool
   {
   public:
      ItemPool(size_t init_cap = 64) : m_capacity(0), m_size(0) { resize(init_cap) ; }
      ~ItemPool() { delete m_items ; }

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
		  size_t new_cap = cap < 64 ? 64 : (cap < 65536 ? 2*cap : 3*cap/2) ;
		  resize(new_cap) ;
		  }
	       }
	    return idx ;
	 }
      void release(size_t /*index*/)
	 {
	    // we can release the last item allocated; if any more have been allocated since, the
	    //   request is ignored and that item simply goes to waste
	    // TODO: use CAS to ensure that the compare and decrement happen atomically

	 }
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

      // access to the allocated items
      T& operator[] (size_t N) const { return N < size() ? m_items[N] : m_items[capacity()-1] ; }
      T* item(size_t N) const { return N < size() ? &m_items[N] : &m_items[capacity()-1] ; }

      // iterator support
      T* begin() const { return m_items ; }
      const T* cbegin() const { return m_items ; }
      T* end() const { return m_items + m_size ; }
      const T* cend() const { return m_items + m_size ; }

   protected:
      void resize(size_t new_cap)
	 {
	    T* new_items = new T[new_cap] ;
	    if (new_items)
	       {
	       size_t cap = m_capacity ;
	       if (cap)
		  std::copy_n(m_items,cap,new_items) ;
	       T* old_items = m_items ;
	       m_items = new_items ;
	       m_capacity = new_cap ;
	       if (cap)
		  delete[] old_items ;
	       }
	 }
   protected:
      T*     m_items ;
      atom_size_t m_capacity ;		// size of the allocated array
      atom_size_t m_size ;		// number of items actually in use
      static std::mutex s_mutex ;	// lock for use when resizing m_items
   } ;

template <typename T>
std::mutex ItemPool<T>::s_mutex ;

} // end namespace Fr

#endif /* !_Fr_ITEMPOOL_H_INCLUDED */

// end of file itempool.h //
