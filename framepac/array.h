/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
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

#include <vector>
#include "framepac/object.h"

namespace Fr {

//----------------------------------------------------------------------------

class ArrayIter
   {
   private:
      Object **m_array ;
   public:
      ArrayIter(Object **o) : m_array(o) {}
      ArrayIter(const ArrayIter &) = default ;
      ~ArrayIter() = default ;

      Object* operator* () const { return *m_array ; }
      Object* operator-> () const { return *m_array ; }
      ArrayIter& operator++ () { ++m_array ; return *this ; }
      Object*& operator [] (size_t n) const { return m_array[n] ; }

      bool operator== (const ArrayIter& other) { return m_array == other.m_array ; }
      bool operator!= (const ArrayIter& other) { return m_array != other.m_array ; }
   } ;

//----------------------------------------------------------------------------

class Array : public Object
   {
   public:
      static Array *create(size_t initial_size = 0) ;
      static Array *create(const Object *) ;
      static Array *create(const Array *) ;

      bool reserve(size_t n) ;

      // *** standard info functions ***
      size_t size() const { return m_size ; }
      size_t empty() const { return size() == 0 ; }

      // *** standard access functions ***
      Object *front() const { return m_array[0] ; }
      Array *subseq(ArrayIter start, ArrayIter stop, bool shallow = false) const ;

      // *** iterator support ***
      ArrayIter begin() { return ArrayIter(m_array) ; }
      ArrayIter cbegin() const { return ArrayIter(m_array) ; }
      ArrayIter end() { return ArrayIter(m_array + size()) ; }
      ArrayIter cend() const { return ArrayIter(m_array + size()) ; }

   private: // static members
      static Allocator<Array> s_allocator ;
   private:
      Object **m_array ;
      size_t   m_size ;
      size_t   m_alloc ;
   protected: // construction/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      Array(size_t initial_size) ;
      Array(const Array*) ;
      Array(const Array& a) : Array(&a) {}
      Array(const Object *, size_t repeat = 1) ;
      ~Array() ;
      void operator= (const Array&) = delete ;
   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Array> ;

      // type determination predicates
      static bool isArray_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "Array" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete (Array*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete (Array*)obj ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object *, char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *obj) { return static_cast<const Array*>(obj)->m_size ; }
      static bool empty_(const Object *obj) { return static_cast<const Array*>(obj)->m_size == 0 ; }

      // *** standard access functions ***
      static Object *front_(Object* obj) { return static_cast<const Array*>(obj)->front() ; }
      static const Object *front_(const Object* obj) { return static_cast<const Array*>(obj)->front() ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object *) ;
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object*) { return nullptr ; }
      static ObjectIter& next_(const Object*, ObjectIter& it) { it.incrIndex() ; return it ; }
   } ;

//----------------------------------------------------------------------------

extern template class Allocator<Array> ;

// end of namespace Fr
} ;

// end of file array.h //
