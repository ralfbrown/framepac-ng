/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
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
   static ret altname(isconst Object *) ;            			\
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

/************************************************************************/
/************************************************************************/

namespace Fr
{

// forward declarations
class Object ;
class ObjectPtr ;

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


class Object_VMT_Base
   {
   public:
      Object_VMT_Base(unsigned /*subtypes*/)
	 {
	 }
      ~Object_VMT_Base() = default ;

      // *** destroying ***
      void (*free_)(Fr::Object*) ;
      // use shallowFree() on a shallowCopy()
      void (*shallowFree_)(Fr::Object*) ;
      // *** reclamation for non-Object items
      void (*releaseSlab_)(class Slab*) ;

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

      // *** dynamic type determination ***
      // name of the actual type of the current object
      const char* (*typeName_)(const Fr::Object*) ;
      // type determination predicates
      bool (*isArray_)(const Fr::Object*) ;
      bool (*isBigNum_)(const Fr::Object*) ;
      bool (*isBitVector_)(const Fr::Object*) ;
      bool (*isComplex_)(const Fr::Object*) ;
      bool (*isFloat_)(const Fr::Object*) ;
      bool (*isInteger_)(const Fr::Object*) ;
      bool (*isList_)(const Fr::Object*) ;
      bool (*isMap_)(const Fr::Object*) ;
      bool (*isNumber_)(const Fr::Object*) ;
      bool (*isRational_)(const Fr::Object*) ;
      bool (*isSparseVector_)(const Fr::Object*) ;
      bool (*isString_)(const Fr::Object*) ;
      bool (*isSymbolTable_)(const Fr::Object*) ;
      bool (*isSymbol_)(const Fr::Object*) ;
      bool (*isVector_)(const Fr::Object*) ;

      // *** I/O ***
      // generate printed representation into a buffer
      size_t (*cStringLength_)(const Fr::Object*, size_t wrap_at, size_t indent) ;
      size_t (*jsonStringLength_)(const Fr::Object*, bool wrap, size_t indent) ;
      bool (*toCstring_)(const Fr::Object*, char* buffer, size_t buflen, size_t wrap_at, size_t indent) ;
      bool (*toJSONString_)(const Fr::Object*, char* buffer, size_t buflen, bool wrap, size_t indent) ;

   public:  // data members
      mutable Slab* m_currslab[1] ; // FIXME
      Slab* m_orphans[1] ; // FIXME
} ;

//----------------------------------------------------------------------------
// polymorphic companion class to Object hierarchy

template <class ObjT, unsigned SubTypes = 1>
class Object_VMT : public Object_VMT_Base
   {
   public:
      // since this is a Singleton class, we have a function to return the single instance of the class
      static const Object_VMT* instance()
	 {
	 static const Object_VMT single_instance(SubTypes) ;
	 return &single_instance ;
	 }

   private:
      // don't allow allocation on the heap
      void *operator new(size_t) = delete ;
      void *operator new(size_t,void*) = delete ;
      void operator delete(void*) = delete ;
      // don't allow user to instantiate -- must go through instance()
      Object_VMT(unsigned num_subtypes) : Object_VMT_Base(num_subtypes)
	 {
	 free_ = &ObjT::free_ ;
	 shallowFree_ = &ObjT::shallowFree_ ;
	 releaseSlab_ = &ObjT::releaseSlab_ ;
	 size_ = &ObjT::size_ ;
	 empty_ = &ObjT::empty_ ;
	 next_ = &ObjT::next_ ;
	 next_iter = &ObjT::next_iter ;
	 clone_ = &ObjT::clone_ ;
	 shallowCopy_ = &ObjT::shallowCopy_ ;
	 subseq_int = &ObjT::subseq_int ;
	 subseq_iter = &ObjT::subseq_iter ;
	 hashValue_ = &ObjT::hashValue_ ;
	 equal_ = &ObjT::equal_ ;
	 compare_ = &ObjT::compare_ ;
	 lessThan_ = &ObjT::lessThan_ ;
	 front_ = &ObjT::front_ ;
	 front_const = &ObjT::front_const ;
	 stringValue_ = &ObjT::stringValue_ ;
	 floatValue_ = &ObjT::floatValue_ ;
	 imagValue_ = &ObjT::imagValue_ ;
	 intValue_ = &ObjT::intValue_ ;
	 bignumValue_ = &ObjT::bignumValue_ ;
	 rationalValue_ = &ObjT::rationalValue_ ;
	 typeName_ = &ObjT::typeName_ ;
	 isArray_ = &ObjT::isArray_ ;
	 isBigNum_ = &ObjT::isBigNum_ ;
	 isBitVector_ = &ObjT::isBitVector_ ;
	 isComplex_ = &ObjT::isComplex_ ;
	 isFloat_ = &ObjT::isFloat_ ;
	 isInteger_ = &ObjT::isInteger_ ;
	 isMap_ = &ObjT::isMap_ ;
	 isNumber_ = &ObjT::isNumber_ ;
	 isRational_ = &ObjT::isRational_ ;
	 isSparseVector_ = &ObjT::isSparseVector_ ;
	 isString_ = &ObjT::isString_ ;
	 isSymbolTable_ = &ObjT::isSymbolTable_ ;
	 isSymbol_ = &ObjT::isSymbol_ ;
	 isVector_ = &ObjT::isVector_ ;
	 cStringLength_ = &ObjT::cStringLength_ ;
	 jsonStringLength_ = &ObjT::jsonStringLength_ ;
	 toCstring_ = &ObjT::toCstring_ ;
	 toJSONString_ = &ObjT::toJSONString_ ;
	 }
      ~Object_VMT() = default ;

   private: // data members
      static thread_local Slab* m_currslab_local[SubTypes] ;
   } ;

typedef class Object_VMT<Fr::Object> ObjectVMT ;
extern template class Object_VMT<Fr::Object> ;

} // end namespace FramepaC

#endif /* !_Fr_OBJECTVMT_H_INCLUDED */

// end of file objectvmt.h //
