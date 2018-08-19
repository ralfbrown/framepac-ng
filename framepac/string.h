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

#ifndef __Fr_STRING_H_INCLUDED
#define __Fr_STRING_H_INCLUDED

#include <cstring>
#include "framepac/object.h"

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

template <typename T>
class PointerPlus16
   {
#if defined(__GNUC__) && defined(__x86_64__) && defined(__LP64__)
   // x86_64 currently only uses 48-bit virtual addresses, so we can
   //   stuff an additional 16-bit value into a 64-bit pointer field.
   public:
      PointerPlus16(T* ptr = nullptr) { m_pointer = ((uintptr_t)ptr) & POINTER_MASK ; }
      PointerPlus16(T* ptr, uint16_t val)
	 {
	    m_pointer = (((uintptr_t)ptr) & POINTER_MASK) | (((uintptr_t)val) << VALUE_SHIFT) ;
	 }
      ~PointerPlus16() {}

      T* pointer() const { return reinterpret_cast<T*>(m_pointer & POINTER_MASK) ; }
      uint16_t extra() const { return (uint16_t)(m_pointer >> VALUE_SHIFT) ; }

      void pointer(T* ptr) { m_pointer = (((uintptr_t)ptr) & POINTER_MASK) | (m_pointer & VALUE_MASK) ; }
      void extra(uint16_t val) { m_pointer = (m_pointer & POINTER_MASK) | (((uintptr_t)val) << VALUE_SHIFT) ; }

   protected:
      static constexpr uintptr_t POINTER_MASK = 0x0000FFFFFFFFFFFFUL ;
      static constexpr uintptr_t VALUE_MASK = 0xFFFF000000000000UL ;
      static constexpr unsigned VALUE_SHIFT = 48 ;
      uintptr_t m_pointer ;
#else
   // For 32-bit architectures and (most?) non-x86 64-bit architectures,
   //   we'll need to create a struct with both a pointer and integer
   //   fields instead of packing the two into a single value.
   public:
      PointerPlus16(T* ptr = nullptr, uint16_t val = 0) { m_pointer = ptr ; m_extra = val ; }
      ~PointerPlus16() {}

      T* pointer() const { return reinterpret_cast<T*>(m_pointer) ; }
      uint16_t extra() const { return m_extra ; }

      void pointer(T* ptr) { m_pointer = ptr ; }
      void extra(uint16_t val) { m_extra = val ; }

   protected:
      void* m_pointer ;
      uint16_t m_extra ;
#endif /* GCC 64-bit build */
   } ;


} // end namespace FramepaC

/************************************************************************/
/************************************************************************/

namespace Fr {

// forward declaration
class String ;

/************************************************************************/
/************************************************************************/

// a smart pointer to String that frees the object when it goes out of scope or has
//  another String assigned to it

typedef Ptr<String> StringPtr ;

/************************************************************************/
/************************************************************************/

typedef char* StringIter ;
typedef const char* ConstStringIter ;

typedef std::reverse_iterator<StringIter> RevStringIter ;
typedef std::reverse_iterator<ConstStringIter> ConstRevStringIter ;

/************************************************************************/
/************************************************************************/

class String : public Object
   {
   public: // types
      typedef Object super ;
   public:
      static String* create() { return new String ; }
      static String* create(const char* s) { return new String(s) ; }
      static String* create(const char* s, size_t len) { return new String(s,len) ; }
      static String* create(const String* s) { return new String(s) ; }
      static String* create(const Object* obj) { return new String(obj) ; }
      static String* create(istream&) ;

      size_t c_len() const
	 { size_t len = m_buffer.extra() ; return (len == 0xFFFF) ? ((size_t*)m_buffer.pointer())[-1] : len ; }
      char* c_str() { return m_buffer.pointer() ; }
      const char* c_str() const { return m_buffer.pointer() ; }

      // *** standard info functions ***
      size_t size() const { return c_len() ; }
      size_t empty() const { return m_buffer.extra() == 0 ; }

      // *** standard access functions ***
      char front() const { return *m_buffer.pointer() ; }
      String* subseq(StringIter start, StringIter stop, bool shallow = false) const ;

      // *** comparison functions ***
      
      // *** iterator support ***
      StringIter begin() { return StringIter(c_str()) ; }
      ConstStringIter cbegin() const { return ConstStringIter(c_str()) ; }
      StringIter end() { return StringIter(c_str()+c_len()) ; }
      ConstStringIter cend() const { return ConstStringIter(c_str()+c_len()) ; }
      String *next() const { return nullptr ; }
      RevStringIter rbegin() { return RevStringIter(end()) ; }
      ConstRevStringIter crbegin() { return ConstRevStringIter(end()) ; }
      RevStringIter rend() { return RevStringIter(begin()) ; }
      ConstRevStringIter crend() { return ConstRevStringIter(begin()) ; }

      // *** operator/typecast overloads ***
      operator bool () const { return c_str() != nullptr ; }
      operator const char* () const { return c_str() ; }

      static void StaticInitialization() ;

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      String() : Object(), m_buffer(nullptr) {}
      String(const char* valuebuffer, size_t len) ;
      String(const char* value) : String(value, value?std::strlen(value):0) {}
      String(const String* s) : String(s->c_str(),s->c_len()) {}
      String(const String& s) : String(s.c_str(), s.c_len()) {}
      String(const Object*) ;
      void init(const char* s, size_t len) ;

      ~String() ;
      String &operator= (const String&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<String> ;

      // type determination predicates
      static bool isString_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return s_typename ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*,ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<String*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { free_(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return static_cast<const String*>(obj)->size() ; }
      static bool empty_(const Object* obj) { return static_cast<const String*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object *front_(Object* obj) { return obj ; }
      static const Object* front_const(const Object* obj) { return obj ; }
      static const char* stringValue_(const Object* obj)
	 { return (static_cast<const String*>(obj))->c_str() ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object* obj) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }

   private: // static members
      static Allocator s_allocator ;
      static Initializer<String> s_initializer ;
      static const char s_typename[] ;
   protected: // data members
      // pack pointer to the actual string plus 16 bits of length into a single 64-bit value.  If the stored
      //   length is 0xFFFF, then the initial size_t of the buffer is the actual length of the string.
      FramepaC::PointerPlus16<char> m_buffer ;
   } ;

/************************************************************************/
/************************************************************************/

inline istream& operator >> (istream& in, String*& obj)
{
   obj = String::create(in) ;
   return in ;
}

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class Object_VMT<Fr::String> ;

} ; // end namespace FramepaC

#endif /* !__Fr_STRING_H_INCLUDED */

// end of file string.h //
