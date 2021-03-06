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

#ifndef _Fr_OBJECTVMT_H_INCLUDED
#define _Fr_OBJECTVMT_H_INCLUDED

#include "framepac/config.h"

/************************************************************************/
/************************************************************************/

// in Object and its derivatives, we virtualize a non-virtual method
//    by calling a method pointer in the corresponding Object_VMT<>
//    and providing a second non-virtual implementation function to
//    which Object_VMT will dispatch

#define FrVIRTFUNC0(ret,name,altname,isconst)				\
   static ret altname(isconst Object *) ;				\
   ret name() isconst                                           	\
      { return FramepaC::Slab::VMT(this)->altname(this) ; }
#define FrVIRTFUNC1(ret,name,altname,isconst,t1,a1)			\
   static ret altname(isconst Object *,t1 a1) ;				\
   ret name(t1 a1) isconst						\
      { return FramepaC::Slab::VMT(this)->altname(this,a1) ; }
#define FrVIRTFUNC2(ret,name,altname,isconst,t1,a1,t2,a2)		\
   static ret altname(isconst Object *,t1 a1,t2 a2) ;			\
   ret name(t1 a1, t2 a2) isconst					\
      { return FramepaC::Slab::VMT(this)->altname(this,a1,a2) ; }
#define FrVIRTFUNC3(ret,name,altname,isconst,t1,a1,t2,a2,t3,a3)		\
   static ret altname(isconst Object *,t1 a1,t2 a2,t3 a3) ;		\
   ret name(t1 a1, t2 a2, t3 a3) isconst				\
      { return FramepaC::Slab::VMT(this)->altname(this,a1,a2,a3) ; }
#define FrVIRTFUNC4(ret,name,altname,isconst,t1,a1,t2,a2,t3,a3,t4,a4)	\
   static ret altname(isconst Object *,t1 a1,t2 a2,t3 a3,t4 a4) ; 	\
   ret name(t1 a1, t2 a2, t3 a3, t4 a4) isconst	           		\
      { return FramepaC::Slab::VMT(this)->altname(this,a1,a2,a3,a4) ; }
#define FrVIRTFUNC5(ret,name,altname,isconst,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5) \
   static ret altname(isconst Object *,t1 a1,t2 a2,t3 a3,t4 a4,t5 a5) ; 	\
   ret name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) isconst			\
      { return FramepaC::Slab::VMT(this)->altname(this,a1,a2,a3,a4,a5) ; }

#define FrVIRTFUNC0OVR(ret,name,alttype,altname,isconst)		\
   static ret altname##_(isconst Object *) ;                       	\
   ret name() isconst		                                  	\
   { FramepaC::Object_VMT<alttype>* vmt = (FramepaC::Object_VMT<alttype>*)FramepaC::Slab::isconst##VMT(this) ; \
     return static_cast<ret>(vmt->altname((alttype*)this)) ; }
#define FrVIRTFUNC1OVR(ret,name,altname,isconst,t1,a1)			\
   static ret altname##_(isconst Object *,t1 a1) ;			\
   ret name(t1 a1) isconst						\
      { return reinterpret_cast<ret>(FramepaC::Slab::isconst##VMT(this)->altname(this,a1)) ; }
#define FrVIRTFUNC2OVR(ret,name,altname,isconst,t1,a1,t2,a2)		\
   static ret altname##_(isconst Object *,t1 a1,t2 a2) ;		\
   ret name(t1 a1, t2 a2) isconst					\
      { return reinterpret_cast<ret>(FramepaC::Slab::isconst##VMT(this)->altname(this,a1,a2)) ; }
#define FrVIRTFUNC3OVR(ret,name,altname,isconst,t1,a1,t2,a2,t3,a3) 	\
   static ret altname##_(isconst Object *,t1 a1,t2 a2,t3 a3) ;		\
   ret name(t1 a1, t2 a2, t3 a3) isconst				\
      { return reinterpret_cast<ret>(FramepaC::Slab::isconst##VMT(this)->altname(this,a1,a2,a3)) ; }
#define FrVIRTFUNC4OVR(ret,name,altname,isconst,t1,a1,t2,a2,t3,a3,t4,a4) \
   static ret altname##_(isconst Object *,t1 a1,t2 a2,t3 a3,t4 a4) ;	\
   ret name(t1 a1, t2 a2, t3 a3, t4 a4) isconst	        		\
      { return reinterpret_cast<ret>(FramepaC::Slab::isconst##VMT(this)->altname(this,a1,a2,a3,a4)) ; }
#define FrVIRTFUNC5OVR(ret,name,altname,isconst,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5) \
   static ret altname##_(isconst Object *,t1 a1,t2 a2,t3 a3,t4 a4,t5 a5) ;	\
   ret name(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5) isconst			\
      { return reinterpret_cast<ret>(FramepaC::Slab::isconst##VMT(this)->altname(this,a1,a2,a3,a4,a5)) ; }

/************************************************************************/
/************************************************************************/

namespace Fr
{

// forward declarations
class Object ;
class Symbol ;

//----------------------------------------------------------------------------
// a smart pointer to <Object> that frees the object when it goes out of scope or has
//  another <Object> assigned to it

template <typename T>
class Ptr
   {
   public:
      Ptr() : m_object(nullptr) {}
      Ptr(T* o) : m_object(o) {}
      Ptr(const Ptr& o) = delete ; 		// Ptr is not copyable
      Ptr(Ptr& o) : m_object(o.move()) {} 	// Ptr is not copyable -- move it instead
      Ptr(Ptr&& o) : m_object(o.move()) { }	// grab ownership from other Ptr
      ~Ptr() { free() ; }

      Ptr& operator= (Ptr& o) { acquire(o) ; return *this ; }
      Ptr& operator= (T* o) { acquire(o) ; return *this ; }

      T& operator* () const { return *m_object ; }
      T* operator-> () const { return m_object ; }
      T* operator& () const { return m_object ; }
      explicit operator bool () const { return m_object != nullptr ; }
      bool operator! () const { return m_object == nullptr ; }
      operator T* () { return m_object ; }
      operator const T* () const { return m_object ; }

      void acquire(Ptr& o) { if (m_object && m_object != o.m_object) m_object->free() ; m_object = o.move() ; }
      void acquire(T* o) { if (m_object && m_object != o) m_object->free() ; m_object = o ; }
      void update(T* o) { m_object = o ; } // use with caution to avoid leaks!
      void release() { m_object = nullptr ; }
      T* move() { T* o = m_object ; release() ; return o ; }
      inline void free() { if (m_object) { m_object->free() ; release() ; } }
      Ptr& swap(Ptr& other) { T* tmp = other.m_object ; other.m_object = m_object ; m_object = tmp ; return *this ; }

   protected:
      T *m_object ;
   } ;

typedef Ptr<Object> ObjectPtr ;

//----------------------------------------------------------------------------
// this iterator can handle polymorphism, at the cost of making it heavier-weight
//   than more specific iterators and requiring virtual method calls
// we need to define this type long before we declare Object, because its
//   definition needs to be known by Object_VMT, which in turn needs to be known
//   by Object
// some of the methods can't be defined here, as they rely on the definition of Object...

class ObjectIter // : std::iterator<std::input_iterator_tag,Object*,std::ptrdiff_t,Object*,Object&>
   {
   public:
      ObjectIter(Object *o, size_t index = 0) : m_object(o), m_index(index) {}
      ObjectIter(const Object *o, size_t index = 0) : m_object(const_cast<Object*>(o)), m_index(index) {}
      ObjectIter(const ObjectIter &other) : m_object(other.m_object), m_index(other.m_index) {}
      ~ObjectIter() = default ;

      Object *baseObject() const { return m_object ; }
      size_t currentIndex() const { return m_index ; }
      void incrIndex() { m_index++ ; }

      Object* operator* () const { return m_object ; }
      Object* operator-> () const { return m_object ; }
      inline ObjectIter& operator++ () ;
      bool operator== (const ObjectIter &other) const
	 { return m_object == other.m_object && m_index == other.m_index ; }
      bool operator!= (const ObjectIter &other) const
	 { return m_object != other.m_object || m_index != other.m_index ; }
   protected:
      Object *m_object ;
      size_t  m_index ;
   protected:
      friend class Object ;
   } ;

} // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{


class ObjectVMT
   {
   public:
      ObjectVMT() = default ;
      ~ObjectVMT() = default ;

      // *** put type name first to make it easier for GDB pretty-printer to find and to avoid breaking
      //     it when adding other new pointers ***
      const char* type_name ;

      // *** destroying ***
      void (*free_)(Fr::Object*) ;
      // use shallowFree() on a shallowCopy()
      void (*shallowFree_)(Fr::Object*) ;

      // *** standard info functions ***
      size_t (*size_)(const Fr::Object*) ;
      bool (*empty_)(const Fr::Object*) ;

      // *** iterator support ***
      Fr::Object* (*next_)(const Fr::Object*) ;
      Fr::ObjectIter& (*next_iter)(const Fr::Object*, Fr::ObjectIter& it) ;

      // *** copying ***
      Fr::ObjectPtr (*clone_)(const Fr::Object*) ;
      Fr::Object* (*shallowCopy_)(const Fr::Object*) ;
      Fr::ObjectPtr (*subseq_int)(const Fr::Object*, size_t start,size_t stop) ;
      Fr::ObjectPtr (*subseq_iter)(const Fr::Object*, Fr::ObjectIter start,Fr::ObjectIter stop) ;

      // *** comparison functions ***
      size_t (*hashValue_)(const Fr::Object*) ;
      bool (*equal_)(const Fr::Object*, const Fr::Object* other) ;
      int (*compare_)(const Fr::Object*, const Fr::Object* other) ;
      // comparison function for STL algorithms
      int (*lessThan_)(const Fr::Object*, const Fr::Object* other) ;

      // *** standard access functions ***
      Fr::Object* (*front_)(Fr::Object*) ;
      const Fr::Object* (*front_const)(const Fr::Object*) ;
      const char * (*stringValue_)(const Fr::Object*) ;
      double (*floatValue_)(const Fr::Object*) ;
      double (*imagValue_)(const Fr::Object*) ;
      long int (*intValue_)(const Fr::Object*) ;
      mpz_t (*bignumValue_)(const Fr::Object*) ;
      mpq_t (*rationalValue_)(const Fr::Object*) ;
      long (*nthInt_)(const Fr::Object*,size_t) ;
      double (*nthFloat_)(const Fr::Object*,size_t) ;

      // *** dynamic type determination ***
      // get label (if any) applied to the object
      Fr::Symbol* (*label_)(const Fr::Object*) ;
      // type determination predicates
      bool (*isArray_)(const Fr::Object*) ;
      bool (*isBigNum_)(const Fr::Object*) ;
      bool (*isBitVector_)(const Fr::Object*) ;
      bool (*isCluster_)(const Fr::Object*) ;
      bool (*isComplex_)(const Fr::Object*) ;
      bool (*isFloat_)(const Fr::Object*) ;
      bool (*isInteger_)(const Fr::Object*) ;
      bool (*isList_)(const Fr::Object*) ;
      bool (*isMap_)(const Fr::Object*) ;
      bool (*isMatrix_)(const Fr::Object*) ;
      bool (*isNumber_)(const Fr::Object*) ;
      bool (*isObject_)(const Fr::Object*) ;
      bool (*isOneHotVector_)(const Fr::Object*) ;
      bool (*isRational_)(const Fr::Object*) ;
      bool (*isSet_)(const Fr::Object*) ;
      bool (*isSparseVector_)(const Fr::Object*) ;
      bool (*isString_)(const Fr::Object*) ;
      bool (*isSymbolTable_)(const Fr::Object*) ;
      bool (*isSymbol_)(const Fr::Object*) ;
      bool (*isTermVector_)(const Fr::Object*) ;
      bool (*isVector_)(const Fr::Object*) ;

      // *** I/O ***
      // generate printed representation into a buffer
      size_t (*cStringLength_)(const Fr::Object*, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      size_t (*jsonStringLength_)(const Fr::Object*, bool wrap, size_t indent) ;
      char* (*toCstring_)(const Fr::Object*, char* buffer, size_t buflen, size_t wrap_at,
	 size_t indent, size_t wrapped_indent) ;
      bool (*toJSONString_)(const Fr::Object*, char* buffer, size_t buflen, bool wrap, size_t indent) ;

      // matrix helper functions
      void (*matrixSet_)(Fr::Object*, size_t row, size_t col, double value) ;
      double (*matrixGet_)(const Fr::Object*, size_t row, size_t col) ;
      void* (*matrixElt_)(const Fr::Object*, size_t row, size_t col) ;
} ;

//----------------------------------------------------------------------------
// polymorphic companion class to Object hierarchy

template <class ObjT>
class Object_VMT : public ObjectVMT
   {
   public: // types
      typedef ObjectVMT super ;
   public:
      // since this is a Singleton class, we have a function to return the single instance of the class
      static const ObjectVMT* instance() { return &s_instance ;  }

   private:
      // the singleton instance
      static constexpr ObjectVMT s_instance
	 {
	    ObjT::s_typename,
	    &ObjT::free_,
	    &ObjT::shallowFree_,
	    &ObjT::size_,
	    &ObjT::empty_,
	    &ObjT::next_,
	    &ObjT::next_iter,
	    &ObjT::clone_,
	    &ObjT::shallowCopy_,
	    &ObjT::subseq_int,
	    &ObjT::subseq_iter,
	    &ObjT::hashValue_,
	    &ObjT::equal_,
	    &ObjT::compare_,
	    &ObjT::lessThan_,
	    &ObjT::front_,
	    &ObjT::front_const,
	    &ObjT::stringValue_,
	    &ObjT::floatValue_,
	    &ObjT::imagValue_,
	    &ObjT::intValue_,
	    &ObjT::bignumValue_,
	    &ObjT::rationalValue_,
	    &ObjT::nthInt_,
	    &ObjT::nthFloat_,
	    &ObjT::label_,
	    &ObjT::isArray_,
	    &ObjT::isBigNum_,
	    &ObjT::isBitVector_,
	    &ObjT::isCluster_,
	    &ObjT::isComplex_,
	    &ObjT::isFloat_,
	    &ObjT::isInteger_,
	    &ObjT::isList_,
	    &ObjT::isMap_,
	    &ObjT::isMatrix_,
	    &ObjT::isNumber_,
	    &ObjT::isObject_,
	    &ObjT::isOneHotVector_,
	    &ObjT::isRational_,
	    &ObjT::isSet_,
	    &ObjT::isSparseVector_,
	    &ObjT::isString_,
	    &ObjT::isSymbolTable_,
	    &ObjT::isSymbol_,
	    &ObjT::isTermVector_,
	    &ObjT::isVector_,
	    &ObjT::cStringLength_,
	    &ObjT::jsonStringLength_,
	    &ObjT::toCstring_,
	    &ObjT::toJSONString_,
	    &ObjT::matrixSet_,
	    &ObjT::matrixGet_,
	    &ObjT::matrixElt_
	    } ;
   private:
      // don't allow allocation on the heap
      void *operator new(size_t) = delete ;
      void *operator new(size_t,void*) = delete ;
      void operator delete(void*) = delete ;
      // don't allow user to instantiate -- must go through instance()
      Object_VMT() = delete ;
      ~Object_VMT() = delete ;
   } ;

template <typename T>
constexpr ObjectVMT Object_VMT<T>::s_instance ;

} // end namespace FramepaC

namespace std
{
template<typename T>
void swap(Fr::Ptr<T>& p1, Fr::Ptr<T>& p2) { p1.swap(p2) ; }

} // end namespace std

#endif /* !_Fr_OBJECTVMT_H_INCLUDED */

// end of file objectvmt.h //
