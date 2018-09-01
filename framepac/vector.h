/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-08-27					*/
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

#ifndef Fr_VECTOR_H_INCLUDED
#define Fr_VECTOR_H_INCLUDED

#include "framepac/as_string.h"
#include "framepac/critsect.h"
#include "framepac/list.h"
#include "framepac/smartptr.h"
#include "framepac/symbol.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

// abstract class, do not instantiate
class VectorBase : public Object
   {
   public:
      VectorBase() {}
      ~VectorBase() {}
      
      Symbol* key() const { return m_key ; }
      Symbol* label() const { return m_label ; }
      float weight() const { return m_weight ; }
      void* userData() const { return m_user ; }

      void setKey(Symbol* k) { m_key = k ; }
      void setLabel(Symbol* l) { m_label = l ; }
      void setWeight(size_t wt) { m_weight = (float)wt ; }
      void setWeight(float wt) { m_weight = wt ; }
      void setWeight(double wt) { m_weight = (float)wt ; }
      void setUserData(void* u) { m_user = u ; }

   protected:
      void startModifying() { m_critsect.lock() ; m_length = -1 ; }
      void doneModifying() { m_critsect.unlock() ; }

   protected:
      size_t size() const { return m_size ; }
      size_t capacity() const { return m_capacity ; }

   protected: // data
      Symbol*  m_key { nullptr } ;	// the vector's name (e.g. word for which this is a context vector)
      Symbol*  m_label { nullptr } ;	// user label applied to vector (e.g. cluster name)
      mutable void*  m_user { nullptr };// available for user to store any needed extra data about this vector
      mutable double m_length { -1 } ;	// cached vector length (L2-norm)
      uint32_t m_size { 0 } ;	  	// number of elements in vector
      uint32_t m_capacity { 0 } ;	// number of elements allocated (may be greater than m_size)
      float    m_weight { 1.0f } ; 
      mutable CriticalSection m_critsect ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename ValT>
class Vector : public VectorBase
   {
   public: // types
      typedef Object super ;
      // export the template type parameter for use in other templates that may not have
      //   an explicit parameter giving this type because they inferred the vector type
      typedef ValT value_type ;

   public:
      static Vector* create(size_t numelts) ;

      void setElement(size_t N, ValT value)
	 {
	 if (N >= Vector<ValT>::m_capacity && !this->reserve(std::max(N+1,2*Vector<ValT>::capacity())))
	    return ;
	 if (N >= this->m_size) this->m_size = N+1 ;
	 this->m_values[N] = value ;
	 }
      
      double length() const { return m_length >= 0.0 ? m_length : vectorLength() ; }

      // support for iterating through elements for e.g. vector similarity functions
      size_t numElements() const { return m_size ; }
      ValT elementValue(size_t N) const { return m_values[N] ; }
      size_t elementIndex(size_t N) const { return N ; }
      
      // arithmetic operations
      template <typename IdxT>
      Vector* add(const Vector* other) const ;		// ret = (*this) + (*other)
      Vector* incr(const Vector* other) ;		// (*this) += (*other)
      Vector* incr(const Vector* other, ValT wt) ;	// (*this) += wt*(*other)
      void scale(double factor) ;
      void normalize() ;

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Vector(size_t capacity = 0) ;
      Vector(const Vector&) ;
      ~Vector() { m_size = 0 ; }
      Vector& operator= (const Vector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Vector> ;

      double vectorLength() const ;

      // type determination predicates
      static bool isVector_(const Object *) { return true ; }
      static const char* typeName_(const Object*) { return s_typename ; }
      static Symbol* label_(const Object* obj) { return static_cast<const Vector*>(obj)->m_label ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<Vector*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { free_(obj) ; }

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
      static double floatValue_(const Object* obj) { return static_cast<const Vector*>(obj)->length() ; }
      static long int intValue(const Object* obj)
	 { return (long)(floatValue_(obj) + 0.5) ; }

      static long nthInt_(const Object* obj, size_t N)
	 { return (long)static_cast<const Vector<ValT>*>(obj)->m_values[N] ; }
      static double nthFloat_(const Object* obj, size_t N)
	 { return (double)static_cast<const Vector<ValT>*>(obj)->m_values[N] ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

      // STL compatibility
      bool reserve(size_t n) ;
      
   protected:
      // helper functions, needed to properly output various index and value types
      size_t value_c_len(size_t N) const
	 {
	 return len_as_string(elementValue(N)) ;
	 }
      char* value_c_string(size_t N, char* buffer, size_t buflen) const
	 {
	 return as_string(elementValue(N),buffer,buflen) ;
	 }

   private:
      static Allocator s_allocator ;
   protected:
      static const char s_typename[] ;
      NewPtr<ValT> m_values ;
   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class OneHotVector : public Vector<ValT>
   {
   public: // types
      // export the template type parameter for use in other templates that may not have
      //   an explicit parameter giving this type because they inferred the vector type
      typedef IdxT index_type ;

   public: // methods
      static OneHotVector* create(IdxT index, ValT value) { return new OneHotVector(index,value) ; }

      // support for iterating through elements for e.g. vector similarity functions
      size_t elementIndex(size_t /*N*/) const { return (size_t)m_index ; }
      ValT elementValue(size_t N) const { return N == (size_t)m_index ? m_value : (ValT)0 ; }

   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      OneHotVector(IdxT index, ValT value = (ValT)1) : m_index(index), m_value(value) {}
      OneHotVector(const OneHotVector&) ;
      ~OneHotVector() { m_value = (ValT)0 ; }
      OneHotVector& operator= (const OneHotVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<OneHotVector> ;

      // type determination predicates
      static bool isSparseVector_(const Object *) { return true ; }
      static bool isOneHotVector_(const Object *) { return true ; }
      static const char* typeName_(const Object*) { return "OneHotVector" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<OneHotVector*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { free_(obj) ; }

      // *** standard info functions ***
      static size_t size_(const Object *obj) { return static_cast<const OneHotVector*>(obj)->size() ; }
      static bool empty_(const Object *obj) { return static_cast<const OneHotVector*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) { return obj ; }
      static const Object *front_const(const Object *obj) { return obj ; }
      static const char *stringValue_(const Object *) { return nullptr ; }
      static long nthInt_(const Object* obj, size_t N)
	 { auto v = static_cast<const Vector<ValT>*>(obj) ;
           return N == (size_t)v->m_index ? (long)v->m_value : 0 ; }
      static double nthFloat_(const Object* obj, size_t N)
	 { auto v = static_cast<const Vector<ValT>*>(obj) ;
           return N == (size_t)v->m_index ? (double)v->m_value : 0 ; }

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

   private:
      static Allocator s_allocator ;
      static const char s_typename[] ;
   protected:
      IdxT m_index ;
      ValT m_value ;
   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class SparseVector : public Vector<ValT>
   {
   public: // types
      // export the template type parameter for use in other templates that may not have
      //   an explicit parameter giving this type because they inferred the vector type
      typedef IdxT index_type ;

   public: // methods
      static SparseVector* create(size_t numelts = 0) { return new SparseVector(numelts) ; }
      static SparseVector* create(const char* rep) { return new SparseVector(rep) ; }

      bool newElement(IdxT index, ValT value) ;

      // retrieve elements of the vector
      IdxT keyAt(size_t N) const { return  m_indices[N] ; }
      using Vector<ValT>::elementValue ;
      
      // support for iterating through elements for e.g. vector similarity functions
      size_t elementIndex(size_t N) const { return (size_t)m_indices[N] ; }

      // arithmetic operations
      SparseVector* add(const Vector<ValT>* other) const ;
      SparseVector* add(const SparseVector* other) const ;
      SparseVector* add(const OneHotVector<IdxT,ValT>* other) const ;
      SparseVector* incr(const Vector<ValT>* other, ValT wt = 1.0) ;
      SparseVector* incr(const SparseVector* other, ValT wt = 1.0) ;
      SparseVector* incr(const OneHotVector<IdxT,ValT>* other, ValT wt = 1.0) ;

      // STL compatibility
      bool reserve(size_t n) ;
      
   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      SparseVector(size_t capacity = 0) ;
      SparseVector(const char* rep) ;
      SparseVector(const SparseVector&) ;
      // G++ 4.8 breaks linkage if we explicitly default the dtor....
      // ~SparseVector() = default ;
      SparseVector& operator= (const SparseVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<SparseVector> ;

      // type determination predicates
      static bool isSparseVector_(const Object *) { return true ; }
      static const char* typeName_(const Object*) { return s_typename ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<SparseVector*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { free_(obj) ; }

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
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

   protected:
      static size_t totalElements(const SparseVector* v1, const Vector<ValT>* v2) ;
      static size_t totalElements(const SparseVector* v1, const SparseVector* v2) ;
      void setElement(size_t N, IdxT k, ValT value)
	 {
	 if (N >= Vector<ValT>::m_capacity && !this->reserve(std::max(N+1,2*Vector<ValT>::capacity())))
	    return ;
	 if (N >= this->m_size) this->m_size = N+1 ;
	 this->m_indices[N] = k ;
	 this->m_values[N] = value ;
	 }

   protected:
      // helper functions, needed to properly output various index and value types
      size_t index_c_len(size_t N) const
	 {
	 return len_as_string(keyAt(N)) ;
	 }
      char* index_c_string(size_t N, char* buffer, size_t buflen) const
	 {
	 return as_string(keyAt(N),buffer,buflen) ;
	 }


   private:
      static Allocator s_allocator ;
      static const char s_typename[] ;
   protected:
      NewPtr<IdxT> m_indices ;
   } ;

extern template class SparseVector<uint32_t,uint32_t> ;
extern template class SparseVector<uint32_t,float> ;
extern template class SparseVector<uint32_t,double> ;
extern template class SparseVector<Object*,float> ;
extern template class SparseVector<Object*,double> ;

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
   public: // types
      typedef Vector<ValT> super ;
   public:
      static DenseVector* create(size_t capacity = 0) { return new DenseVector(capacity) ; }
      static DenseVector* create(const char* rep) { return new DenseVector(rep) ; }
      static DenseVector* create(const DenseVector* orig) { return orig ? new DenseVector(*orig) : new DenseVector ; }

      // arithmetic operations
      DenseVector* add(const DenseVector* other) const ;
      template <typename IdxT>
      SparseVector<IdxT,ValT>* add(const SparseVector<IdxT,ValT>* other) const ;
      template <typename IdxT>
      DenseVector<ValT>* add(const OneHotVector<IdxT,ValT>* other) const ;

      DenseVector* incr(const DenseVector* other, double wt = 1.0) ;
      template <typename IdxT>
      DenseVector* incr(const SparseVector<IdxT,ValT>* other) ;
      template <typename IdxT>
      DenseVector* incr(const SparseVector<IdxT,ValT>* other, ValT wt = 1.0) ;
      template <typename IdxT>
      DenseVector* incr(const OneHotVector<IdxT,ValT>* other, ValT wt = 1.0) ;

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      DenseVector(size_t cap = 0) : super(cap) {}
      DenseVector(const char* rep) ;
      DenseVector(const Vector<ValT>&v) : super(v) {}
      ~DenseVector() {}
      DenseVector& operator= (const DenseVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<DenseVector> ;

      // type determination predicates
      static const char* typeName_(const Object*) { return s_typename ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

   protected:
      static const char s_typename[] ;

   private:
      static Allocator s_allocator ;
   } ;

extern template class DenseVector<uint32_t> ;
extern template class DenseVector<float> ;
extern template class DenseVector<double> ;

//----------------------------------------------------------------------------

template <typename T>
class VectorGroup
   {
   public:
      VectorGroup() ;
      VectorGroup(const VectorGroup&) ;
      ~VectorGroup() ;

      void clear() ;
      bool addMembers(List* newmembers) ;
      bool addMember(T* v) ;
      void trimMembers(bool clear_cluster_membership = false) ;
      void setCentroid(T* c) { m_centroid = c ; }
      bool merge(VectorGroup* other, bool sum_sizes) ;

      // accessors
      T* centroid() const { return m_centroid ; }
      size_t size() const { return m_size ; }
      size_t capacity() const { return m_capacity ; }
      const T** members() const { return m_vectors ; }
      
   protected:
      T**    m_vectors ;
      T*     m_centroid ;
      size_t m_size ;			// number of vectors in group
      size_t m_capacity ;		// size of vector array
   } ;

//----------------------------------------------------------------------------

} ; // end of namespace Fr

#endif /* !Fr_VECTOR_H_INCLUDED */

// end of vector.h //
