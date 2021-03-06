/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-18					*/
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

#ifndef __Fr_OBJECT_H_INCLUDED
#define __Fr_OBJECT_H_INCLUDED

#include <iostream>
#include <string>
#include "framepac/smartptr.h"
#include "framepac/memory.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

// forward declarations
class Object ;
class CFile ;

/************************************************************************/
/************************************************************************/

typedef bool ObjectPredicateFn(const Object*) ;
typedef bool ObjectCompareFn(const Object*, const Object*) ;
typedef int ObjectOrderingFn(const Object*, const Object*) ;

/************************************************************************/
/************************************************************************/

// non-polymorphic underlying version of Object

class Object
   {
   public:
      ~Object() {}
      static ObjectPtr create(char*& printed_representation) ;
      static ObjectPtr create(const char*& printed_representation) ;
      static ObjectPtr create(const Object*) ;
      static ObjectPtr create(FILE*) ;
      static ObjectPtr create(CFile&) ;
      static ObjectPtr create(istream&) ;
      static ObjectPtr create(const std::string&) ;

      static ObjectPtr createFromJSON(const char*& printed_representation) ;
      static ObjectPtr createFromJSON(FILE*) ;
      static ObjectPtr createFromJSON(CFile&) ;
      static ObjectPtr createFromJSON(istream&) ;
      static ObjectPtr createFromJSON(const std::string&) ;

      // *** copying ***
      FrVIRTFUNC0(ObjectPtr,clone,clone_,const) ;
      // note: shallow and deep copies are the same for most objects
      FrVIRTFUNC0(Object*,shallowCopy,shallowCopy_,const) ;
      FrVIRTFUNC2(ObjectPtr,subseq,subseq_int,const,size_t,start,size_t,stop) ;
      FrVIRTFUNC2(ObjectPtr,subseq,subseq_iter,const,ObjectIter,start,ObjectIter,stop) ;
      
      // *** destroying ***
      FrVIRTFUNC0(void,free,free_,) ;
      // use shallowFree() on a shallowCopy(); shallow and deep copies are the same for most objects
      FrVIRTFUNC0(void,shallowFree,shallowFree_,) ;

      // *** dynamic type determination ***
      // name of the actual type of the current object
      const char* typeName() const { return FramepaC::Slab::VMT(this)->type_name ; }
      // get label (if any) applied to the object
      FrVIRTFUNC0(Symbol*,label,label_,const) ;
      // type determination predicates
      FrVIRTFUNC0(bool,isArray,isArray_,const) ;
      FrVIRTFUNC0(bool,isBigNum,isBigNum_,const) ;
      FrVIRTFUNC0(bool,isBitVector,isBitVector_,const) ;
      FrVIRTFUNC0(bool,isCluster,isCluster_,const) ;
      FrVIRTFUNC0(bool,isComplex,isComplex_,const) ;
      FrVIRTFUNC0(bool,isFloat,isFloat_,const) ;
      FrVIRTFUNC0(bool,isInteger,isInteger_,const) ;
      FrVIRTFUNC0(bool,isList,isList_,const) ;
      FrVIRTFUNC0(bool,isMap,isMap_,const) ;
      FrVIRTFUNC0(bool,isMatrix,isMatrix_,const) ;
      FrVIRTFUNC0(bool,isNumber,isNumber_,const) ;
      FrVIRTFUNC0(bool,isObject,isObject_,const) ;
      FrVIRTFUNC0(bool,isOneHotVector,isOneHotVector_,const) ;
      FrVIRTFUNC0(bool,isRational,isRational_,const) ;
      FrVIRTFUNC0(bool,isSet,isSet_,const) ;
      FrVIRTFUNC0(bool,isSparseVector,isSparseVector_,const) ;
      FrVIRTFUNC0(bool,isString,isString_,const) ;
      FrVIRTFUNC0(bool,isSymbol,isSymbol_,const) ;
      FrVIRTFUNC0(bool,isSymbolTable,isSymbolTable_,const) ;
      FrVIRTFUNC0(bool,isTermVector,isTermVector_,const) ;
      FrVIRTFUNC0(bool,isVector,isVector_,const) ;

      // *** I/O ***
      const char* printableName() const { return (this != nullptr) ? stringValue() : nullptr ; }
      // generate printed representation to a stream
      ostream& print(ostream&) const ;
      // generate printed representation into an allocated buffer
      CharPtr cString(size_t wrap_at = 0, size_t indent = 0, size_t wrapped_indent = 0) const ;
      CharPtr jsonString(bool wrap = false, size_t indent = 0) const ;
      // generate printed representation into a buffer
      FrVIRTFUNC5(char*,toCstring,toCstring_,const,char*,buffer,size_t,buflen,size_t,wrap_at,size_t,indent,size_t,wrapped_indent) ;
      char* toCstring(char* buffer, size_t buflen) const { return toCstring(buffer,buflen,0,0,0) ; }
      FrVIRTFUNC4(bool,toJSONString,toJSONString_,const,char*,buffer,size_t,buflen,bool,wrap,size_t,indent) ;
      bool toJSONString(char* buffer, size_t buflen) const { return toJSONString(buffer,buflen,false,0) ; }
      // determine length of buffer required for string representation of object
      FrVIRTFUNC3(size_t,cStringLength,cStringLength_,const,size_t,wrap_at,size_t,indent,size_t,wrapped_indent) ;
      size_t cStringLength(size_t wrap_at = ~0UL) const { return cStringLength(wrap_at,0,0) ; }
      FrVIRTFUNC2(size_t,jsonStringLength,jsonStringLength_,const,bool,wrap,size_t,indent) ;
      size_t jsonStringLength(bool wrap) const { return jsonStringLength(wrap,0) ; }
      size_t jsonStringLength() const { return jsonStringLength(false,0) ; }

      // convert text into an Object
      static Object* read(const char* s, const char** s_next = nullptr) ;
      static Object* read(FILE* fp) ;
      static Object* read(istream&) ;
      // helper function to make it easier to display objects in the debugger
      void _() const ;

      // *** standard info functions ***
      FrVIRTFUNC0(size_t,size,size_,const) ;
      FrVIRTFUNC0(bool,empty,empty_,const) ;

      // *** standard access functions ***
      FrVIRTFUNC0(Object*,front,front_,) ;
      FrVIRTFUNC0(const Object*,front,front_const,const) ;
      FrVIRTFUNC0(const char*,stringValue,stringValue_,const) ;
      FrVIRTFUNC0(double,floatValue,floatValue_,const) ;
      FrVIRTFUNC0(double,imagValue,imagValue_,const) ;
      FrVIRTFUNC0(long int,intValue,intValue_,const) ;
      FrVIRTFUNC0(mpz_t,bignumValue,bignumValue_,const) ;
      FrVIRTFUNC0(mpq_t,rationalValue,rationalValue_,const) ;
      FrVIRTFUNC1(long,nthInt,nthInt_,const,size_t,N) ;
      FrVIRTFUNC1(double,nthFloat,nthFloat_,const,size_t,N) ;
      FrVIRTFUNC2(double,matrixGet,matrixGet_,const,size_t,row,size_t,col) ;
      FrVIRTFUNC3(void,matrixSet,matrixSet_,,size_t,row,size_t,col,double,val) ;
      FrVIRTFUNC2(void*,matrixElt,matrixElt_,const,size_t,row,size_t,col) ;

      // *** comparison functions ***
      FrVIRTFUNC0(size_t,hashValue,hashValue_,const) ;
      FrVIRTFUNC1(bool,equal,equal_,const,const Object*,other) ;
      FrVIRTFUNC1(int,compare,compare_,const,const Object*,other) ;
      // comparison function for STL algorithms
      FrVIRTFUNC1(int,lessThan,lessThan_,const,const Object*,other) ;
      int lessThan(const Object &other) const { return lessThan(&other) ; }

      // *** iterator support ***
      // this is the default version for non-aggregate types: start with the object itself,
      //   then increment to the null pointer, which serves as the past-the-end iterator value
      ObjectIter begin() const { return ObjectIter(this) ; }
      ObjectIter cbegin() const { return ObjectIter(this) ; }
      ObjectIter end() const { return ObjectIter((const Object*)nullptr) ; }
      ObjectIter cend() const { return ObjectIter((const Object*)nullptr) ; }
      FrVIRTFUNC0(Object*,next,next_,const) ;
      FrVIRTFUNC1(ObjectIter&,next,next_iter,const,ObjectIter&,it) ;

      // *** operator/typecast overloads ***
      operator bool () const
	 { const void* v = this ; // work around "non-null pointer compared against null" warning
           return v != nullptr && !this->empty() ; }
      operator const char* () const { return stringValue() ; }

   protected:
      Object(const Object*) {}
      Object(const Object&) {}
      Object(const Object&&) {}
      Object() {}

   private:
      static const char s_typename[] ;

   protected: // static data
      friend class FramepaC::Object_VMT<Object>  ;
   } ;

inline Object* Object::shallowCopy_(const Object* obj) { return clone_(obj) ; }
inline void Object::free_(Object*) {}
inline void Object::shallowFree_(Object* obj) { free_(obj) ; }
inline Symbol* Object::label_(const Object*) { return nullptr ; }
inline bool Object::isArray_(const Object*) { return false ; }
inline bool Object::isBigNum_(const Object*) { return false ; }
inline bool Object::isBitVector_(const Object*) { return false ; }
inline bool Object::isCluster_(const Object*) { return false ; }
inline bool Object::isComplex_(const Object*) { return false ; }
inline bool Object::isFloat_(const Object*) { return false ; }
inline bool Object::isInteger_(const Object*) { return false ; }
inline bool Object::isList_(const Object*) { return false ; }
inline bool Object::isMap_(const Object*) { return false ; }
inline bool Object::isMatrix_(const Object*) { return false ; }
inline bool Object::isNumber_(const Object*) { return false ; }
inline bool Object::isObject_(const Object*) { return true ; }
inline bool Object::isOneHotVector_(const Object*) { return false ; }
inline bool Object::isRational_(const Object*) { return false ; }
inline bool Object::isSet_(const Object*) { return false ; }
inline bool Object::isSparseVector_(const Object*) { return false ; }
inline bool Object::isString_(const Object*) { return false ; }
inline bool Object::isSymbol_(const Object*) { return false ; }
inline bool Object::isSymbolTable_(const Object*) { return false ; }
inline bool Object::isTermVector_(const Object*) { return false ; }
inline bool Object::isVector_(const Object*) { return false ; }

inline size_t Object::size_(const Object*) { return 0 ; }
inline bool Object::empty_(const Object* obj) { return size_(obj) == 0 ; }
inline Object* Object::front_(Object* obj) { return obj ; }
inline const Object* Object::front_const(const Object* obj) { return obj ; }
inline const char* Object::stringValue_(const Object*) { return nullptr ; }
inline double Object::floatValue_(const Object*) { return 0.0 ; }
inline double Object::imagValue_(const Object*) { return 0.0 ; }
inline long Object::intValue_(const Object*) { return 0 ; }
inline long Object::nthInt_(const Object*,size_t) { return 0 ; }
inline double Object::nthFloat_(const Object*,size_t) { return 0.0 ; }
inline void Object::matrixSet_(Object*, size_t, size_t, double) { return ; }
inline double Object::matrixGet_(const Object*, size_t, size_t) { return 0.0 ; }
inline void* Object::matrixElt_(const Object*, size_t, size_t) { return nullptr ; }

inline bool Object::equal_(const Object* obj, const Object* other) { return obj == other ; }

inline Object* Object::next_(const Object*) { return nullptr ; }
inline ObjectIter& Object::next_iter(const Object*,ObjectIter& it) { it.m_object = nullptr ; return it ; }

/************************************************************************/
/************************************************************************/

template <typename T = Object>
class ScopedObject : public Ptr<T>
   {
   public:
      typedef Ptr<T> super ;
   public:
      ScopedObject() : super(T::create()) { }
      ScopedObject(nullptr_t) : super(nullptr) { }
      template <typename ...Args>
      ScopedObject(Args... args) : super(T::create(args...)) {}

      ScopedObject& operator = (ScopedObject&) = delete ;
      ScopedObject& operator = (const ScopedObject&) = delete ;
      ScopedObject& operator = (ScopedObject&& orig) { this->acquire(orig) ; return *this ; }
      ScopedObject& operator = (ObjectPtr& op) { this->acquire(op) ; return *this ; }
      ScopedObject& operator = (ObjectPtr&& op) { this->acquire(op) ; return *this ; }
      ScopedObject& operator = (T* o) { this->acquire(o) ; return *this ; }
   } ;

/************************************************************************/
/************************************************************************/

inline ObjectIter& ObjectIter::operator++ ()
{ 
   return m_object ? m_object->next(*this) : *this ; 
}

/************************************************************************/
/************************************************************************/

bool equal(const Object* obj1, const Object* obj2) ;

inline ostream& operator << (ostream& out, const Object* obj)
{
   return obj->print(out) ;
}

} // end of namespace Fr

/************************************************************************/
/************************************************************************/

namespace std
{
// custom specializations of std::hash to support FramepaC objects

template<>
struct hash<Fr::Object>
{
   std::size_t operator() (const Fr::Object& o) const
      { return o.hashValue() ; }

} ;

template<>
struct hash<Fr::ObjectPtr>
{
   std::size_t operator() (const Fr::ObjectPtr& o) const
      { return o->hashValue() ; }

} ;

} // end namespace std

#endif /* !__Fr_OBJECT_H_INCLUDED */

// end of file object.h //
