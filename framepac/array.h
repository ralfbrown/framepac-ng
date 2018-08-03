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

#ifndef __FrARRAY_H_INCLUDED
#define __FrARRAY_H_INCLUDED

#include <vector>
#include "framepac/object.h"

namespace Fr {

//----------------------------------------------------------------------------

typedef Object** ArrayIter ;

class RevArrayIter
   {
   private:
      Object** m_array ;
   public:
      RevArrayIter(Object** o) : m_array(o) {}
      RevArrayIter(const RevArrayIter&) = default ;
      ~RevArrayIter() = default ;

      Object* operator* () const { return *m_array ; }
      Object* operator-> () const { return *m_array ; }
      RevArrayIter& operator++ () { --m_array ; return *this ; }
      Object*& operator [] (size_t n) const { return m_array[-n] ; }

      bool operator== (const RevArrayIter& other) { return m_array == other.m_array ; }
      bool operator!= (const RevArrayIter& other) { return m_array != other.m_array ; }
   } ;

//----------------------------------------------------------------------------

class RefArray ;

class Array : public Object
   {
   public:
      typedef Object super ;
   public:
      static Array* create(size_t initial_size = 0) { return new Array(initial_size) ; }
      static Array* create(const Object*) ;
      static Array* create(const Array*) ;

      bool append(const Object*) ;
      bool appendNoCopy(Object*) ;
      Object* getNth(size_t N) const { return N < m_size ? m_array[N] : nullptr ; }
      void setNth(size_t N, const Object* val) ;
      bool elide(size_t N) ;

      void reverse() ;

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
      ArrayIter begin() const { return ArrayIter(m_array) ; }
      ArrayIter cbegin() const { return ArrayIter(m_array) ; }
      ArrayIter end() const { return ArrayIter(m_array + size()) ; }
      ArrayIter cend() const { return ArrayIter(m_array + size()) ; }

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

   private: // static members
      static Allocator s_allocator ;
   protected:
      Object** m_array ;
      size_t   m_size ;
      size_t   m_alloc ;
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
      static const char *typeName_(const Object*) { return "Array" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object *shallowCopy_(const Object*obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) { delete (Array*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { delete (Array*)obj ; }

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
      void setNth(size_t N, Object* val) { if (N < m_size) m_array[N] = val ; }
      bool elide(size_t N) ;

      void clearArray(bool free_objects = false) ;

      // STL compatibility
      void pop_back() ;  	// remove last element of array

   private: // static members
      static Allocator s_allocator ;
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

      // type determination predicates
      static const char *typeName_(const Object*) { return "RefArray" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object *shallowCopy_(const Object*obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<RefArray*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { delete static_cast<RefArray*>(obj) ; }

   } ;

} ; // end namespace Fr

#endif /* !__FrARRAY_H_INCLUDED */

// end of file array.h //
