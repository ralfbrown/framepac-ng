/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-05					*/
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

// in Object_VMT, we just store a pointer to the proper type's
// non-virtual implementation method.
#define FrVIRTIMPL0(ret,name,isconst)					\
   ret (*name)(isconst Fr::Object*) = &ObjT::name ;
#define FrVIRTIMPL1(ret,name,isconst,t1,a1)				\
   ret (*name)(isconst Fr::Object*,t1) = &ObjT::name ;
#define FrVIRTIMPL2(ret,name,isconst,t1,a1,t2,a2)			\
   ret (*name)(isconst Fr::Object*,t1,t2) = &ObjT::name ;
#define FrVIRTIMPL3(ret,name,isconst,t1,a1,t2,a2,t3,a3)			\
   ret (*name)(isconst Fr::Object*,t1,t2,t3) = &ObjT::name ;
#define FrVIRTIMPL4(ret,name,isconst,t1,a1,t2,a2,t3,a3,t4,a4)		\
   ret (*name)(isconst Fr::Object*,t1,t2,t3,t4) = &ObjT::name ;

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

//----------------------------------------------------------------------------
// polymorphic companion class to Object hierarchy

template <class ObjT>
class Object_VMT
   {
   private:
      // don't allow allocation on the heap
      void *operator new(size_t) = delete ;
      void *operator new(size_t,void*) = delete ;
      void operator delete(void*) = delete ;
   public:
      Object_VMT() = default ;
      ~Object_VMT() = default ;

      // *** destroying ***
      FrVIRTIMPL0(void,free_,) ;
      // use shallowFree() on a shallowCopy()
      FrVIRTIMPL0(void,shallowFree_,) ;
      // *** reclamation for non-Object items
      void (*releaseSlab_)(class Slab*) = &ObjT::releaseSlab_ ;

      // *** standard info functions ***
      FrVIRTIMPL0(size_t,size_,const) ;
      FrVIRTIMPL0(bool,empty_,const) ;

      // *** iterator support ***
      FrVIRTIMPL0(Fr::Object*,next_,const) ;
      FrVIRTIMPL1(Fr::ObjectIter&,next_iter,const,Fr::ObjectIter&,it) ;

      // *** copying ***
      FrVIRTIMPL0(Fr::ObjectPtr,clone_,const) ;
      FrVIRTIMPL0(Fr::Object*,shallowCopy_,const) ;
      FrVIRTIMPL2(Fr::ObjectPtr,subseq_int,const,size_t,start,size_t,stop) ;
      FrVIRTIMPL2(Fr::ObjectPtr,subseq_iter,const,Fr::ObjectIter,start,Fr::ObjectIter,stop) ;

      // *** comparison functions ***
      FrVIRTIMPL0(size_t,hashValue_,const) ;
      FrVIRTIMPL1(bool,equal_,const,const Fr::Object*,other) ;
      FrVIRTIMPL1(int,compare_,const,const Fr::Object*,other) ;
      // comparison function for STL algorithms
      FrVIRTIMPL1(int,lessThan_,const,const Fr::Object*,other) ;

      // *** standard access functions ***
      FrVIRTIMPL0(Fr::Object*,front_,) ;
      FrVIRTIMPL0(const Fr::Object*,front_const,const) ;
      FrVIRTIMPL0(const char *,stringValue_,const) ;
      FrVIRTIMPL0(double,floatValue_,const) ;
      FrVIRTIMPL0(double,imagValue_,const) ;
      FrVIRTIMPL0(long int,intValue_,const) ;
      FrVIRTIMPL0(mpz_t,bignumValue_,const) ;
      FrVIRTIMPL0(mpq_t,rationalValue_,const) ;

      // *** dynamic type determination ***
      // name of the actual type of the current object
      FrVIRTIMPL0(const char*,typeName_,const) ;
      // type determination predicates
      FrVIRTIMPL0(bool,isArray_,const) ;
      FrVIRTIMPL0(bool,isBigNum_,const) ;
      FrVIRTIMPL0(bool,isBitVector_,const) ;
      FrVIRTIMPL0(bool,isComplex_,const) ;
      FrVIRTIMPL0(bool,isFloat_,const) ;
      FrVIRTIMPL0(bool,isInteger_,const) ;
      FrVIRTIMPL0(bool,isList_,const) ;
      FrVIRTIMPL0(bool,isMap_,const) ;
      FrVIRTIMPL0(bool,isNumber_,const) ;
      FrVIRTIMPL0(bool,isRational_,const) ;
      FrVIRTIMPL0(bool,isSparseVector_,const) ;
      FrVIRTIMPL0(bool,isString_,const) ;
      FrVIRTIMPL0(bool,isSymbolTable_,const) ;
      FrVIRTIMPL0(bool,isSymbol_,const) ;
      FrVIRTIMPL0(bool,isVector_,const) ;

      // *** I/O ***
      // generate printed representation into a buffer
      FrVIRTIMPL2(size_t,cStringLength_,const,size_t,wrap_at,size_t,indent) ;
      FrVIRTIMPL2(size_t,jsonStringLength_,const,bool,wrap,size_t,indent) ;
      FrVIRTIMPL4(bool,toCstring_,const,char*,buffer,size_t,buflen,size_t,wrap_at,size_t,indent) ;
      FrVIRTIMPL4(bool,toJSONString_,const,char*,buffer,size_t,buflen,bool,wrap,size_t,indent) ;
   } ;

typedef class Object_VMT<Fr::Object> ObjectVMT ;
extern template class Object_VMT<Fr::Object> ;

} // end namespace FramepaC

#endif /* !_Fr_OBJECTVMT_H_INCLUDED */

// end of file objectvmt.h //
