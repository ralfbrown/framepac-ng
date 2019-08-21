/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-08-21					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018,2019 Carnegie Mellon University		*/
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

template <typename IdxT, typename ValT>  class DenseVector ;
template <typename IdxT, typename ValT>  class SparseVector ;

template <typename IdxT, typename ValT>
class Vector : public VectorBase
   {
   public: // types
      typedef Object super ;
      typedef DenseVector<IdxT,ValT> dense_type ;
      typedef SparseVector<IdxT,ValT> sparse_type ;
      
      // export the template type parameter for use in other templates that may not have
      //   an explicit parameter giving this type because they inferred the vector type
      typedef ValT value_type ;

   public:
      static Vector* create(size_t numelts) { return new Vector(numelts) ; }

      void setElement(size_t N, ValT value)
	 {
	 if (N >= this->m_capacity && !this->reserve(std::max(N+1,2*this->capacity())))
	    return ;
	 if (N >= this->m_size) this->m_size = N+1 ;
	 this->m_values.full[N] = value ;
	 }
      
      double length() const { return m_length >= 0.0 ? m_length : vectorLength() ; }

      // support for iterating through elements for e.g. vector similarity functions
      size_t numElements() const { return m_size ; }
      ValT elementValue(size_t N) const { return m_values.full[N] ; }
      size_t elementIndex(size_t N) const { return N ; }
      
      // arithmetic operations
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
	 { return (long)static_cast<const Vector*>(obj)->m_values.full[N] ; }
      static double nthFloat_(const Object* obj, size_t N)
	 { return (double)static_cast<const Vector*>(obj)->m_values.full[N] ; }

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
      // unfortunately, Vector needs to know some of the details of SparseVector and OneHotVector...
      union val_by_type
	 {
	 public:
	    ValT* full ;
	    ValT  onehot ;
         public:
	    val_by_type() : full(nullptr) { }
	    ~val_by_type() {}
	 } m_values ;
      union idx_by_type
	 {
         public:
	    IdxT* full ;
	    IdxT  onehot ;
         public:
	    idx_by_type() : full(nullptr) { }
	    ~idx_by_type() {}
	 } m_indices ;
   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class OneHotVector : public Vector<IdxT,ValT>
   {
   public: // types
      // export the template type parameter for use in other templates that may not have
      //   an explicit parameter giving this type because they inferred the vector type
      typedef IdxT index_type ;

   public: // methods
      static OneHotVector* create(IdxT index, ValT value) { return new OneHotVector(index,value) ; }

      // support for iterating through elements for e.g. vector similarity functions
      size_t elementIndex(size_t /*N*/) const { return (size_t)this->m_indices.onehot ; }
      ValT elementValue(size_t N) const { return N == (size_t)this->m_indices.onehot ? this->m_values.onehot : ValT(0) ; }

   protected:
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      OneHotVector(IdxT index, ValT value = (ValT)1) { this->m_indices.onehot = index ; this->m_values.onehot = value ; }
      OneHotVector(const OneHotVector&) = default ;
      ~OneHotVector() {}
      OneHotVector& operator= (const OneHotVector&) = default ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<OneHotVector> ;

      // type determination predicates
      static bool isSparseVector_(const Object *) { return true ; }
      static bool isOneHotVector_(const Object *) { return true ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *obj)
	 {
	    auto orig = static_cast<const OneHotVector*>(obj) ;
	    return new OneHotVector(orig->m_indices.onehot,orig->m_values.onehot) ;
	 }
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t /*start*/, size_t /*stop*/)
	 {
	    //TODO
	    return nullptr ;
	 }
      static ObjectPtr subseq_iter(const Object *,ObjectIter /*start*/, ObjectIter /*stop*/)
	 {
	    //TODO
	    return nullptr ;
	 }

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
	 { auto v = static_cast<const OneHotVector<IdxT,ValT>*>(obj) ;
           return N == (size_t)v->m_indices.onehot ? (long)v->m_values.onehot : 0 ; }
      static double nthFloat_(const Object* obj, size_t N)
	 { auto v = static_cast<const OneHotVector<IdxT,ValT>*>(obj) ;
           return N == (size_t)v->m_indices.onehot ? (double)v->m_values.onehot : 0 ; }

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other)
	 {
	    if (other && other->isOneHotVector())
	       {
	       auto v1 = static_cast<const OneHotVector*>(obj) ;
	       auto v2 = static_cast<const OneHotVector*>(other) ;
	       return v1->m_indices.onehot == v2->m_indices.onehot && v1->m_values.onehot == v2->m_values.onehot ;
	       }
	    else
	       return obj == other ;
	 }
      static int compare_(const Object *obj, const Object *other)
	 {
	    //TODO
	    return obj == other ;
	 }
      static int lessThan_(const Object */*obj*/, const Object */*other*/)
	 {
	    //TODO
	    return 0 ;
	 }

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

   private:
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename IdxT, typename ValT>
const char OneHotVector<IdxT,ValT>::s_typename[] = "OneHotVector" ;

template <typename IdxT, typename ValT>
Allocator OneHotVector<IdxT,ValT>::s_allocator(FramepaC::Object_VMT<OneHotVector<IdxT,ValT>>::instance(),
   sizeof(OneHotVector<IdxT,ValT>)) ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class SparseVector : public Vector<IdxT,ValT>
   {
   public: // types
      typedef Vector<IdxT,ValT> super ;
      // export the template type parameter for use in other templates that may not have
      //   an explicit parameter giving this type because they inferred the vector type
      typedef IdxT index_type ;

   public: // methods
      static SparseVector* create(size_t numelts = 0) { return new SparseVector(numelts) ; }
      static SparseVector* create(const char* rep) { return new SparseVector(rep) ; }

      bool newElement(IdxT index, ValT value) ;

      // retrieve elements of the vector
      IdxT keyAt(size_t N) const { return this->m_indices.full[N] ; }
      using super::elementValue ;
      
      // support for iterating through elements for e.g. vector similarity functions
      size_t elementIndex(size_t N) const { return (size_t)this->m_indices.full[N] ; }

      // arithmetic operations
      SparseVector* add(const super* other) const ;
      SparseVector* add(const SparseVector* other) const ;
      SparseVector* add(const OneHotVector<IdxT,ValT>* other) const ;
      SparseVector* incr(const Vector<IdxT,ValT>* other, ValT wt = 1.0) ;

      // STL compatibility
      bool reserve(size_t n) ;
      
   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      SparseVector(size_t capacity = 0) ;
      SparseVector(const char* rep) ;
      SparseVector(const SparseVector&) ;
      ~SparseVector()
	 {
	 delete[] this->m_values.full ; this->m_values.full = nullptr ;
	 delete[] this->m_indices.full ; this->m_indices.full = nullptr ;
	 }
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
      static size_t totalElements(const SparseVector* v1, const super* v2) ;
      static size_t totalElements(const SparseVector* v1, const SparseVector* v2) ;
      void setElement(size_t N, IdxT k, ValT value)
	 {
	 if (N >= this->m_capacity && !this->reserve(std::max(N+1,2*this->capacity())))
	    return ;
	 if (N >= this->m_size) this->m_size = N+1 ;
	 this->m_indices.full[N] = k ;
	 this->m_values.full[N] = value ;
	 }

      void updateContents(IdxT* indices, ValT* values) ;

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

template <typename IdxT, typename ValT>
class DenseVector : public Vector<IdxT,ValT>
   {
   public: // types
      typedef Vector<IdxT,ValT> super ;
      typedef SparseVector<IdxT,ValT> sparse_type ;
   public:
      static DenseVector* create(size_t capacity = 0) { return new DenseVector(capacity) ; }
      static DenseVector* create(const char* rep) { return new DenseVector(rep) ; }
      static DenseVector* create(const DenseVector* orig) { return orig ? new DenseVector(*orig) : new DenseVector ; }

      // arithmetic operations
      DenseVector* add(const DenseVector* other) const ;
      sparse_type* add(const sparse_type* other) const ;
      DenseVector* add(const OneHotVector<IdxT,ValT>* other) const ;

      DenseVector* incr(const DenseVector* other, double wt = 1.0) ;
      DenseVector* incr(const sparse_type* other) ;
      DenseVector* incr(const sparse_type* other, ValT wt = 1.0) ;
      DenseVector* incr(const OneHotVector<IdxT,ValT>* other, ValT wt = 1.0) ;

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      DenseVector(size_t cap = 0) ;
      DenseVector(const char* rep) ;
      DenseVector(const super&vec) : super(vec) {} ;
      DenseVector(const DenseVector&) = default ;
      ~DenseVector() { delete[] this->m_values.full ; this->m_values.full = nullptr ; }
      DenseVector& operator= (const DenseVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<DenseVector> ;

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

   private:
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

extern template class DenseVector<uint32_t,uint32_t> ;
extern template class DenseVector<uint32_t,float> ;
extern template class DenseVector<uint32_t,double> ;

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
