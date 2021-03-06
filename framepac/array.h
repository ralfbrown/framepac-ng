/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-24					*/
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

#ifndef __FrARRAY_H_INCLUDED
#define __FrARRAY_H_INCLUDED

#include <vector>
#include "framepac/object.h"

namespace Fr {

class Array ;

//----------------------------------------------------------------------------

typedef Object** ArrayIter ;
typedef const Object** ConstArrayIter ;
typedef std::reverse_iterator<ArrayIter> RevArrayIter ;
typedef std::reverse_iterator<ConstArrayIter> ConstRevArrayIter ;
typedef Ptr<Array> ArrayPtr ;

//----------------------------------------------------------------------------

class RefArray ;

class Array : public Object
   {
   public:
      typedef Object super ;
      typedef ArrayIter iterator ;
      typedef ConstArrayIter const_iterator ;
   public:
      static Array* create(size_t initial_size = 0) { return new Array(initial_size) ; }
      static Array* create(const Object* o, size_t rpt = 1) { return new Array(o,rpt) ; }
      static Array* create(const Array* a) { return new Array(a) ; }

      bool append(const Object*) ;
      bool appendNoCopy(Object*) ;
      Object* getNth(size_t N) const { return N < m_size ? m_array[N] : nullptr ; }
      void setNth(size_t N, const Object* val) ;
      void setNthNoCopy(size_t N, Object* val) ;
      void clearNth(size_t N) { if (N < m_size) m_array[N] = nullptr ; }

      bool elide(size_t N) ;
      bool removeNulls() ; // compact elements, skipping any null ptrs; returns whether or not anything changed

      void reverse() ;
      void sort(ObjectOrderingFn*) ;

      // return a reference array containing the given number of elements sampled at random
      //   if size < 1, use the given proportion; if size >= 1, use that number of elements
      RefArray* randomSample(double size) const ;

      // *** standard info functions ***
      size_t size() const { return m_size ; }
      size_t empty() const { return size() == 0 ; }

      // *** standard access functions ***
      Object* front() const { return m_array[0] ; }
      Array* subseq(ArrayIter start, ArrayIter stop, bool shallow = false) const ;

      // *** iterator support ***
      iterator begin() const { return iterator(m_array) ; }
      const_iterator cbegin() const { return const_iterator(m_array) ; }
      iterator end() const { return iterator(m_array + size()) ; }
      const_iterator cend() const { return const_iterator(m_array + size()) ; }

      RevArrayIter rbegin() { return RevArrayIter(m_array + size() - 1) ; }
      RevArrayIter crbegin() const { return RevArrayIter(m_array + size() - 1) ; }
      RevArrayIter rend() { return RevArrayIter(m_array - 1) ; }
      RevArrayIter crend() const { return RevArrayIter(m_array - 1) ; }

      // STL compatibility
      bool reserve(size_t n) ;
      void resize(size_t N) ;
      void resize(size_t N, Object*) ;
      void shrink_to_fit() ;
      void push_back(Object* obj) { (void)append(obj) ; } // append item to array
      void pop_back() ;  	// remove last element of array
      void insert(size_t pos, Object*) ;
      void erase(size_t pos) ;
      void erase(size_t pos1, size_t pos2) ;

      Object* at(size_t pos) const { return (pos < size()) ? m_array[pos] : nullptr ; }
      Object*& operator[] (size_t pos) ;

      Object* back() const ;	// get last element in array
      Object** data() const { return m_array ; }

      size_t capacity() const { return m_alloc ; }

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Array() : m_array(nullptr), m_size(0), m_alloc(0) {}
      Array(size_t initial_size) ;
      Array(const Array*) ;
      Array(const Array& a) : Array(&a) {}
      Array(const Object *, size_t repeat = 1) ;
      ~Array() ;
      void operator= (const Array&) = delete ;
   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Array> ;

      // type determination predicates
      static bool isArray_(const Object*) { return true ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object *shallowCopy_(const Object*obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<Array*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { Array::free_(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*, char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return static_cast<const Array*>(obj)->m_size ; }
      static bool empty_(const Object* obj) { return static_cast<const Array*>(obj)->m_size == 0 ; }

      // *** standard access functions ***
      static Object* front_(Object* obj) { return static_cast<const Array*>(obj)->front() ; }
      static const Object* front_(const Object* obj) { return static_cast<const Array*>(obj)->front() ; }
      static long nthInt_(const Object* obj, size_t N)
	 { const Object* val = static_cast<const Array*>(obj)->getNth(N) ;
           return val ? val->intValue() : 0 ; }
      static double nthFloat_(const Object* obj, size_t N)
	 { const Object* val = static_cast<const Array*>(obj)->getNth(N) ;
           return val ? val->floatValue() : 0 ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

      // *** iterator support ***
      static Object* next_(const Object*) { return nullptr ; }
      static ObjectIter& next_(const Object*, ObjectIter& it) { it.incrIndex() ; return it ; }

   private: // static members
      static Allocator s_allocator ;
      static const char s_typename[] ;
   protected:
      Object** m_array ;
      size_t   m_size ;
      size_t   m_alloc ;
   } ;

//----------------------------------------------------------------------------
// exactly the same as Array, except that it doesn't copy inserted objects and
//   doesn't delete the contained objects when it is deleted

class RefArray : public Array
   {
   public:
      typedef Array super ;
   public:
      static RefArray* create(size_t initial_size = 0) { return new RefArray(initial_size) ; }
      static RefArray* create(const Object*) ;
      static RefArray* create(const Array* a) { return new RefArray(a) ; }

      bool append(Object*) ;
      void setNth(size_t N, Object* val) ;
      void setNthNoCopy(size_t N, Object* val) { setNth(N,val) ; }
      bool elide(size_t N) ;

      void clearArray(bool free_objects = false) ;

      // STL compatibility
      void pop_back() ;  	// remove last element of array

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      RefArray(size_t initial_size) : super(initial_size) {}
      RefArray(const Array*) ;
      RefArray(const Array& a) : RefArray(&a) {}
      RefArray(const Object *, size_t repeat = 1) ;
      ~RefArray() ;
      void operator= (const RefArray&) = delete ;
   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<RefArray> ;

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object *shallowCopy_(const Object*obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<RefArray*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { RefArray::free_(obj) ; }

   private: // static members
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

} ; // end namespace Fr

#endif /* !__FrARRAY_H_INCLUDED */

// end of file array.h //
