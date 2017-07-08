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

#ifndef __Fr_STRING_H_INCLUDED
#define __Fr_STRING_H_INCLUDED

#include "framepac/object.h"

namespace Fr {

// forward declaration
class String ;

/************************************************************************/
/************************************************************************/

// a smart pointer to Object that frees the object when it goes out of scope or has
//  another Object assigned to it

class StringPtr
   {
   private:
      String *m_string ;
   public:
      StringPtr(String *s = nullptr) : m_string(s) {}
      StringPtr(const StringPtr &s) = delete ; 	// not copyable
      StringPtr(StringPtr &&s) : m_string(s.m_string) { s.release() ; }	// grab ownership from other ObjectPtr
      ~StringPtr() { free() ; }

      String& operator* () const { return *m_string ; }
      String* operator-> () const { return m_string ; }
      StringPtr& operator= (StringPtr &s)
	 {
	 if (m_string != s.m_string) free() ;
	 acquire(s) ;
	 return *this ;
	 }
      operator String* () const { return m_string ; }
      operator bool () const { return m_string != nullptr ; }

      void acquire(StringPtr &s) { m_string = s.m_string ; s.release() ; }
      void release() { m_string = nullptr ; }
      inline void free() ; //  forward declaration, see after definition of String
   } ;

/************************************************************************/
/************************************************************************/

template <typename T>
class StringIter_
   {
   private:
      T *m_string ;
   public:
      StringIter_(T *s) : m_string(s) {}
      StringIter_(const StringIter_&) = default ;
      ~StringIter_() {}
      StringIter_& operator= (const StringIter_&) = default ;

      T& operator* () const { return *m_string ; }
      T* operator-> () const { return m_string ; }
      StringIter_& operator++ () { ++m_string ; return *this ; }
      T& operator[] (size_t index) const { return m_string[index] ; }

      bool operator== (const StringIter_& other) const { return m_string == other.m_string ; }
      bool operator!= (const StringIter_& other) const { return m_string != other.m_string ; }
   } ;

typedef StringIter_<char> StringIter ;
typedef StringIter_<const char> ConstStringIter ;

/************************************************************************/
/************************************************************************/

class String : public Object
   {
   public:
      static String *create() { return new String ; }
      static String *create(const char *s) { return new String(s) ; }
      static String *create(const char *s, size_t len) { return new String(s,len) ; }
      static String *create(const Object *obj) { return new String(obj) ; }

      const char *c_str() const { return m_string ; }

      // *** standard info functions ***
      size_t size() const { return m_size ; }
      size_t empty() const { return m_size == 0 ; }

      // *** standard access functions ***
      char front() const { return *m_string ; }
      String *subseq(StringIter start, StringIter stop, bool shallow = false) const ;

      // *** iterator support ***
      StringIter begin() { return StringIter(m_string) ; }
      ConstStringIter cbegin() const { return ConstStringIter(m_string) ; }
      StringIter end() { return StringIter(m_string+size()) ; }
      ConstStringIter cend() const { return ConstStringIter(m_string+size()) ; }
      String *next() const { return nullptr ; }

   private: // static members
      static Allocator s_allocator ;
   protected: // data members
      char  *m_string ;
      size_t m_size ;

   protected: // creation/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      String() : m_string(nullptr), m_size(0) {}
      String(const char* value) ;
      String(const char *valuebuffer, size_t len) ; //FIXME
      String(const String *) ;
      String(const String &) ;
      String(const Object *) ;
      ~String() { delete m_string ; }
      String &operator= (const String&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<String> ;

      // type determination predicates
      static bool isString_(const Object *) { return true ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<String*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete static_cast<String*>(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *,size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object *,char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *obj) { return static_cast<const String*>(obj)->size() ; }
      static bool empty_(const Object *obj) { return static_cast<const String*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) { return obj ; }
      static const Object *front_const(const Object *obj) { return obj ; }
      static const char *stringValue_(const Object *obj) { return (static_cast<const String*>(obj))->m_string ; }

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }
   } ;

//----------------------------------------------------------------------------

inline void StringPtr::free() { if (m_string) { m_string->free() ; release() ; } }

//----------------------------------------------------------------------------

class ShortConstString : public Object
   {
   public:
      static ShortConstString *create(const char *s) ;

      // *** iterator support ***
      ConstStringIter begin() const { return ConstStringIter(m_str) ; }
      ConstStringIter cbegin() const { return ConstStringIter(m_str) ; }
      ConstStringIter end() const { return ConstStringIter(m_str + m_len) ; }
      ConstStringIter cend() const { return ConstStringIter(m_str + m_len) ; }

   private:

   protected:
      friend class FramepaC::Object_VMT<ShortConstString> ;

      // type determination predicates
      static bool isString_(const Object *) { return true ; }

   protected:
      uint8_t	m_len ;
      char      m_str[1] ;
   } ;

//----------------------------------------------------------------------------

class ConstString : public ShortConstString
   {
   public:
      static ConstString *create(const char *s) ;
      static ConstString *create(const char *s, size_t len) ;
   private:

   protected:
      friend class FramepaC::Object_VMT<ConstString> ;

   protected:
      uint32_t	m_fulllen ;
      char     *m_fullstr ;
   } ;

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class Object_VMT<Fr::String> ;

} ; // end namespace FramepaC

#endif /* !__Fr_STRING_H_INCLUDED */

// end of file string.h //
