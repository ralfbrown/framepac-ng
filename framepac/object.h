/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-17					*/
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

// a smart pointer to Object that frees the object when it goes out of scope or has
//  another Object assigned to it

class ObjectPtr
   {
   private:
      Object *m_object ;
   public:
      ObjectPtr(Object *o = nullptr) : m_object(o) {}
      ObjectPtr(const ObjectPtr &o) = delete ; 	// not copyable
      ObjectPtr(ObjectPtr &&o) : m_object(o.m_object) { o.release() ; }	// grab ownership from other ObjectPtr
      ~ObjectPtr() { free() ; }

      Object& operator* () const { return *m_object ; }
      Object* operator-> () const { return m_object ; }
      Object* operator& () const { return m_object ; }
      ObjectPtr& operator= (ObjectPtr &o)
	 {
	 if (m_object != o.m_object) free() ;
	 acquire(o) ;
	 return *this ;
	 }
      operator Object* () const { return m_object ; }
      operator bool () const { return m_object != nullptr ; }

      void acquire(ObjectPtr &o) { m_object = o.m_object ; o.release() ; }
      void release() { m_object = nullptr ; }
      Object* move() { Object* o = m_object ; release() ; return o ; }
      inline void free() ; //  forward declaration, see after definition of Object
   } ;

/************************************************************************/
/************************************************************************/

// non-polymorphic underlying version of Object

class Object
   {
   private:
      // no data members //
   protected:
      Object(const Object*) {}
      Object(const Object&) {}
      Object(const Object&&) {}
      Object() {}
   public:
      ~Object() {}
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
      FrVIRTFUNC0(const char*,typeName,typeName_,const) ;
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
      // generate printed representation; returned value must be freed
      char* cString(size_t wrap_at = 0, size_t indent = 0, size_t wrapped_indent = 0) const ;
      char* jsonString(bool wrap = false, size_t indent = 0) const ;
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
      operator bool () const
	 { const void* v = this ; // work around "non-null pointer compared against null" warning
           return v != nullptr && !this->empty() ; }

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

   } ;

inline Object* Object::shallowCopy_(const Object* obj) { return clone_(obj) ; }
inline void Object::free_(Object*) {}
inline void Object::shallowFree_(Object* obj) { free_(obj) ; }
inline const char* Object::typeName_(const Object*) { return "Object" ; }
inline bool Object::isArray_(const Object*) { return false ; }
inline bool Object::isBigNum_(const Object*) { return false ; }
inline bool Object::isBitVector_(const Object*) { return false ; }
inline bool Object::isCluster_(const Object*) { return false ; }
inline bool Object::isComplex_(const Object*) { return false ; }
inline bool Object::isFloat_(const Object*) { return false ; }
inline bool Object::isInteger_(const Object*) { return false ; }
inline bool Object::isList_(const Object*) { return false ; }
inline bool Object::isMap_(const Object*) { return false ; }
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

inline bool Object::equal_(const Object* obj, const Object* other) { return obj == other ; }

inline Object* Object::next_(const Object*) { return nullptr ; }
inline ObjectIter& Object::next_iter(const Object*,ObjectIter& it) { it.m_object = nullptr ; return it ; }

//----------------------------------------------------------------------------

inline void ObjectPtr::free() { if (m_object) { m_object->free() ; release() ; } }

/************************************************************************/
/************************************************************************/

template <typename T = Object>
class ScopedObject
   {
   public:
      ScopedObject() { m_object = T::create() ; }
      ScopedObject(nullptr_t) { m_object = nullptr ; }
      template <typename ...Args>
      ScopedObject(Args... args) { m_object = T::create(args...) ; }
      ~ScopedObject() { if (m_object) m_object->free() ; }

      ScopedObject& operator = (ScopedObject&) = delete ;
      ScopedObject& operator = (ObjectPtr& op)
	 { if (m_object) m_object->free() ; m_object = static_cast<T*>(&op) ; op.release() ; return *this ; }
      ScopedObject& operator = (T* o) { if (m_object) m_object->free() ; m_object = o ; return *this ; }

      void release() { m_object = nullptr ; }

      operator bool () const { return m_object != nullptr ; }
      operator T* () const { return m_object ; }
      T* operator -> () const { return  m_object ; }

   protected:
      T* m_object ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename T = Object>
class ScopedObjectPtr
   {
   public:
      ScopedObjectPtr(T* o) : m_object(o) { }
      ~ScopedObjectPtr() { if (m_object) m_object->free() ; }

      void release() { m_object = nullptr ; }

      operator bool () const { return m_object != nullptr ; }
      operator T* () const { return m_object ; }
      T* operator -> () const { return  m_object ; }

   protected:
      T* m_object ;
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

// end of namespace Fr
} ;

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
