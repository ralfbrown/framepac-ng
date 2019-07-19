/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-12					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018,2019 Carnegie Mellon University			*/
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

#ifndef _Fr_SMARTPTR_H_INCLUDED
#define _Fr_SMARTPTR_H_INCLUDED

#include <algorithm>
#include <cstdint>

namespace Fr
{

//----------------------------------------------------------------------------
// smart pointer to a single item

template <typename T>
class Owned
   {
   public:
      typedef T* pointer ;
      typedef T element_type ;
   public:
      Owned() { this->m_item = new T ; }
      Owned(nullptr_t) { this->m_item = nullptr ; }
      Owned(T* s) { this->m_item = s ; }
      template <typename ...Args>
      Owned(Args ...args) { this->m_item = new T(args...) ; }
      Owned(const Owned&) = delete ;
      Owned(Owned& orig) { this->m_item = orig.move() ; }
      Owned(Owned&& orig) { this->m_item = orig.move() ; }
      ~Owned() { delete m_item ; }

      Owned& operator= (const Owned&) = delete ;
      Owned& operator= (Owned& orig) { this->reset(orig.move()) ; return *this ; }
      Owned& operator= (Owned&& orig) { this->reset(orig.move()) ; return *this ; }
      Owned& operator= (T* new_s) { this->reset(new_s) ; return *this ; }

      T* get() const { return this->m_item ; }
      T* move() { T* s = m_item ; m_item = nullptr ; return s ; }
      void reset(T* ptr) { T* old = m_item ; m_item = ptr ; delete old ; }
      
      T* operator-> () { return this->m_item ; }
      const T* operator-> () const { return this->m_item ; }
      T& operator* () { return *this->m_item ; }
      const T& operator* () const { return *this->m_item ; }
      T* operator& () { return this->m_item ; }
      const T* operator& () const { return this->m_item ; }
      operator T* () { return this->m_item ; }
      operator const T* () const { return this->m_item ; }
      explicit operator bool () const { return this->m_item != nullptr ; }
      bool operator ! () const { return this->m_item == nullptr ; }
   private:
      T* m_item ;
} ;

//----------------------------------------------------------------------------
// smart pointer to an array of items

template <typename T>
class NewPtr
   {
   public:
      typedef T* pointer ;
      typedef T element_type ;
   public:
      NewPtr() { m_items = nullptr ; }
      NewPtr(unsigned N) { m_items = new T[N] ; }
      NewPtr(unsigned N, const T* s, unsigned copyN)
	 { m_items = new T[N] ; std::copy(s,s+copyN,m_items) ; }
      NewPtr(T* s) { m_items = s ; }
      NewPtr(const NewPtr&) = delete ;
      NewPtr(NewPtr& orig) { m_items = orig.move() ; }
      NewPtr(NewPtr&& orig) { m_items = orig.move() ; }
      ~NewPtr() { reset(nullptr) ; }
      NewPtr& operator= (const NewPtr&) = delete ;
      NewPtr& operator= (NewPtr& orig) { reset(orig.release()) ; return *this ; }
      NewPtr& operator= (NewPtr&& orig) { reset(orig.release()) ; return *this ; }
      NewPtr& operator= (T* new_s) { reset(new_s) ; return *this ; }

      T* get() const noexcept { return m_items ; }
      T* move() { T* s = m_items ; clear() ; return s ; }
      void clear() { m_items = nullptr ; }
      void reset(T* ptr) { T* old = m_items ; m_items = ptr ; delete[] old ; }
      T* release() { T* ptr = m_items ; clear() ; return ptr ; }
      bool reallocate(size_t prevsize, size_t newsize)
	 {
	    T* new_items = new T[newsize] ;
	    if (new_items)
	       {
	       if (newsize < prevsize) prevsize = newsize ;
	       std::move(m_items,m_items+prevsize,new_items) ;
	       delete[] m_items ;
	       m_items = new_items ;
	       return true ;
	       }
	    return false ;
	 }
      
      T* operator-> () { return m_items ; }
      const T* operator-> () const { return m_items ; }
      T* operator* () { return m_items ; }
      const T* operator* () const { return m_items ; }
      explicit operator T* () const { return m_items ; }
      operator const T* () const { return m_items ; }
      T& operator[] (size_t N) const { return m_items[N] ; }
      explicit operator bool () const { return m_items != nullptr ; }
      bool operator ! () const { return m_items == nullptr ; }
      T* begin() const { return m_items ; }
   protected:
      T* m_items ;
   } ;

//----------------------------------------------------------------------
// pre-declare pointers to standard types

typedef NewPtr<char> CharPtr ;
typedef NewPtr<uint8_t> UInt8Ptr ;
typedef NewPtr<short> ShortPtr ;
typedef NewPtr<unsigned short> UShortPtr ;
typedef NewPtr<int> IntPtr ;
typedef NewPtr<unsigned> UIntPtr ;
typedef NewPtr<float> FloatPtr ;
typedef NewPtr<double> DoublePtr ;

} // end of namespace Fr



#endif /* !_Fr_SMARTPTR_H_INCLUDED */

// end of file smartptr.h //
