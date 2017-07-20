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

#ifndef __FrVECTOR_H_INCLUDED
#define __FrVECTOR_H_INCLUDED

#include "framepac/object.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

template <typename ValT>
class Vector : public Object
   {
   public:
      // export the template type parameter for use in other templates that may not have
      //   an explicit parameter giving this type because they inferred the vector type
      typedef ValT value_type ;

      static Vector* create(size_t numelts) ;

      double length() const { return m_length >= 0.0 ? m_length : vectorLength() ; }

      // support for iterating through elements for e.g. vector similarity functions
      size_t numElements() const { return m_size ; }
      ValT elementValue(size_t N) const { return m_values[N] ; }
      size_t elementIndex(size_t N) const { return N ; }

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Vector() ;
      Vector(const Vector&) ;
      ~Vector() { delete [] m_values ; m_size = 0 ; }
      Vector& operator= (const Vector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Vector> ;

      double vectorLength() const ;

      // type determination predicates
      static bool isVector_(const Object *) { return true ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<Vector*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete static_cast<Vector*>(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object *,char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *obj) { return static_cast<const Vector*>(obj)->size() ; }
      static bool empty_(const Object *obj) { return static_cast<const Vector*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) { return obj ; }
      static const Object *front_const(const Object *obj) { return obj ; }
      static const char *stringValue_(const Object *) { return nullptr ; }

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

   private:
      static Allocator s_allocator ;
   protected:
      ValT*  m_values ;
      mutable double m_length ; // cached vector length (L2-norm)
      size_t m_size ;	  	// number of elements in vector
   } ;

//----------------------------------------------------------------------------
//   we have a bunch of template functions that define a
//   more-efficient specialization for the case of two dense vectors
//   and a generic version for the other combinations of dense and
//   sparce vectors.  To get the proper template inference,
//   SparseVector can't be a derivative of the dense vector type, so
//   we have a common base type from which both DenseVector and
//   SparseVector derive, and make that base class a dense vector in
//   all but name.

template <typename ValT>
class DenseVector : public Vector<ValT>
   {
   public:
      static DenseVector* create(size_t numelts) ;

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      DenseVector() : Vector<ValT>() {}
      DenseVector(const Vector<ValT>&v) : Vector<ValT>(v) {}
      ~DenseVector() {}
      DenseVector& operator= (const DenseVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<DenseVector> ;

   private:
      static Allocator s_allocator ;
   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class SparseVector : public Vector<ValT>
   {
   public:
      static SparseVector* create(size_t numelts) { return new SparseVector(numelts) ; }

      // retrieve elements of the vector
      IdxT keyAt(size_t N) const { return  m_indices[N] ; }
      using Vector<ValT>::elementValue ;
      
      // support for iterating through elements for e.g. vector similarity functions
      size_t elementIndex(size_t N) const { return (size_t)m_indices[N] ; }

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      SparseVector(size_t capacity = 0) ;
      SparseVector(const SparseVector&) ;
      ~SparseVector() { delete [] m_indices ; }
      SparseVector& operator= (const SparseVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<SparseVector> ;

      // type determination predicates
      static bool isSparseVector_(const Object *) { return true ; }
      static const char* typeName_(const Object*) { return "SparseVector" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<SparseVector*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete static_cast<SparseVector*>(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object *,char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;
	 
      // *** standard info functions ***
      static size_t size_(const Object *obj) { return static_cast<const SparseVector*>(obj)->size() ; }
      static bool empty_(const Object *obj) { return static_cast<const SparseVector*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) { return obj ; }
      static const Object *front_const(const Object *obj) { return obj ; }
      static const char *stringValue_(const Object *) { return nullptr ; }

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

   protected:
      void setElement(size_t N, IdxT key, ValT value)
	 {
	    this->m_indices[N] = key ;
	    this->m_values[N] = value ;
	 }

   protected:
      // helper functions, needed to properly output various index and value types
      static size_t item_c_len(intmax_t value)
	 {
	 return snprintf(nullptr,0,"%jd",value) ;
	 }
      static size_t item_c_len(uintmax_t value)
	 {
	 return snprintf(nullptr,0,"%ju",value) ;
	 }
      static size_t item_c_len(unsigned value)
	 {
	 return snprintf(nullptr,0,"%u",value) ;
	 }
      static size_t item_c_len(long double value)
	 {
	 return snprintf(nullptr,0,"%LG",value) ;
	 }
      static size_t item_c_len(double value)
	 {
	 return snprintf(nullptr,0,"%G",value) ;
	 }
      static size_t item_c_len(const Object* o)
	 {
	 return o ? o->cStringLength() : 4 ; // will print #N<> for nullptr
	 }
      static size_t item_c_len(const void*)
	 {
	 return 3 ; // will print ???
	 }

      static char* item_c_string(intmax_t value, char* buffer, size_t buflen)
	 {
	 return buffer + snprintf(buffer,buflen,"%jd",value) ;
	 }
      static char* item_c_string(uintmax_t value, char* buffer, size_t buflen)
	 {
	 return buffer + snprintf(buffer,buflen,"%ju",value) ;
	 }
      static char* item_c_string(unsigned value, char* buffer, size_t buflen)
	 {
	 return buffer + snprintf(buffer,buflen,"%u",value) ;
	 }
      static char* item_c_string(long double value, char* buffer, size_t buflen)
	 {
	 return buffer + snprintf(buffer,buflen,"%LG",value) ;
	 }
      static char* item_c_string(double value, char* buffer, size_t buflen)
	 {
	 return buffer + snprintf(buffer,buflen,"%G",value) ;
	 }
      static char* item_c_string(const Object* o, char* buffer, size_t buflen)
	 {
	 if (o)
	    return o->toCstring(buffer,buflen,0,0,0) ;
	 return buffer + snprintf(buffer,buflen,"#N<>") ;
	 }
      static char* item_c_string(const void*, char* buffer, size_t buflen)
	 {
	 return buffer + snprintf(buffer,buflen,"???") ;
	 }

      size_t index_c_len(size_t N) const
	 {
	 return item_c_len(keyAt(N)) ;
	 }
      char* index_c_string(size_t N, char* buffer, size_t buflen) const
	 {
	 return item_c_string(keyAt(N),buffer,buflen) ;
	 }

      size_t value_c_len(size_t N) const
	 {
	 return item_c_len(elementValue(N)) ;
	 }
      char* value_c_string(size_t N, char* buffer, size_t buflen) const
	 {
	 return item_c_string(elementValue(N),buffer,buflen) ;
	 }

   private:
      static Allocator s_allocator ;
   protected:
      IdxT*  m_indices ;
   } ;

extern template class SparseVector<uint32_t,uint32_t> ;
extern template class SparseVector<uint32_t,float> ;
extern template class SparseVector<uint32_t,double> ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class OneHotVector : public Vector<ValT>
   {
   public:
      static OneHotVector* create(size_t numelts) ;

   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      OneHotVector(IdxT index, ValT value = (ValT)1) ;
      OneHotVector(const OneHotVector&) ;
      ~OneHotVector() { m_value = (ValT)0 ; }
      OneHotVector& operator= (const OneHotVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<OneHotVector> ;

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<OneHotVector*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete static_cast<OneHotVector*>(obj) ; }

      // *** standard info functions ***
      static size_t size_(const Object *obj) { return static_cast<const OneHotVector*>(obj)->size() ; }
      static bool empty_(const Object *obj) { return static_cast<const OneHotVector*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) { return obj ; }
      static const Object *front_const(const Object *obj) { return obj ; }
      static const char *stringValue_(const Object *) { return nullptr ; }

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

   private:
      static Allocator s_allocator ;
   protected:
      IdxT m_index ;
      ValT m_value ;
   } ;

//----------------------------------------------------------------------------

} ; // end of namespace Fr

#endif /* !__FrVECTOR_H_INCLUDED */

// end of vector.h //
