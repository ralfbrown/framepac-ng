/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#ifndef _Fr_LIST_H_INCLUDED
#define _Fr_LIST_H_INCLUDED

#include <forward_list>
#include "framepac/object.h"

namespace Fr {

// forward declarations
class List ;
class ListBuilder ;

//----------------------------------------------------------------------------
// due to the circular dependencies, we can't actually define many of the functions
//   inline in the iterator class definition; they will be defined after the underlying
//   class has been declared

class ListIter
   {
   private:
      List	*m_list ;
   public:
      ListIter(List *l) : m_list(l) {}
      ListIter(const List *l) : m_list(const_cast<List*>(l)) {}
      ListIter(const ListIter &o) : m_list(o.m_list) {}
      ListIter(ObjectIter &o) : m_list((List*)o.baseObject()) {}
      ~ListIter() = default ;

      inline Object* operator* () const ;
//!!!      List* operator-> () const { return  m_list ; }
      inline ListIter& operator++ () ;
      bool operator== (const ListIter& other) const { return m_list == other.m_list ; }
      bool operator!= (const ListIter& other) const { return m_list != other.m_list ; }
   } ;

//----------------------------------------------------------------------------

class List : public Object
   {
   public: // types
      typedef Object super ;
   public:
      // *** object factories ***
      static List* create() ;
      static List* create(Object*) ;
      static List* create(Object*, Object*) ;
      static List* create(Object*, Object*, Object*) ;
      static List* create(Object*, Object*, Object*, Object*) ;
      static List* create(Object*, Object*, Object*, Object*, Object*) ;
      static List* create(const char*&) ;
      static List* create(istream&) ;

      // generate a list of strings from a 'sentence' with single blanks delimiting tokens
      static List* createWordList(const char*, char delim = ' ') ;

      bool member(const Object* o) const ;
      Object* member(const Object* o, ObjectCompareFn* fn) const ;

      List* push(Object* o) { List* l = List::create(o) ; l->setNext(this) ; return l ; }
      List* pop(Object*& o) ;
      List* elide(size_t start, size_t stop) ;
      List* nconc(List* newtail) ;

      List* removeIf(ObjectPredicateFn*) ;

      void setFront(Object* o) { m_item = o ? o : empty_list ; }

      // *** standard info functions ***
      size_t size() const ;
      bool empty() const { return this == empty_list ; }
      operator bool () const { return this != nullptr && !this->empty() ; }

      // *** standard access functions ***
      Object* front() const { return m_item ; }

      List* subseq(ListIter start, ListIter stop, bool shallow = false) const ;

      // *** iterator support ***
      ListIter begin() const { return ListIter(this) ; }
      ListIter cbegin() const { return ListIter(this) ; }
      static ListIter end() { return ListIter(empty_list) ; }
      static ListIter cend() { return ListIter(empty_list) ; }
      List* next() const { return m_next ; }

      // *** utility functions ***
      bool contains(const Object*) const ;
      List* last() const ;
      List* reverse() ;
      List* sort(ObjectOrderingFn*) ;
      Object* nth(size_t N) const ;
      List* nthcdr(size_t N) ;
      const List* nthcdr(size_t N) const ;
      const List* assoc(const Object* key) const ;

      void setNext(List* nxt) { m_next = nxt ; }
      List** nextPtr() { return &m_next ; }
      List* const * nextPtr() const { return &m_next ; }
      static List* emptyList() { return empty_list ; }

      // *** startup/shutdown functions ***
      static void StaticInitialization() ;
      static void StaticCleanup() ;

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      List() ;
      List(const List*) ;
      List(const List&) ;
      ~List() {}
      List& operator= (const List&) = delete ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<List> ;
      friend class ListIter ;
      friend class ListBuilder ;

      // type determination predicates
      static bool isList_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "List" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object*) ;
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) ;
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object*) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object*) ;
      static bool empty_(const Object* obj) { return obj == empty_list ; }

      // *** standard access functions ***
      static Object* front_(Object*) ;
      static const Object* front_(const Object*) ;
      static const char* stringValue_(const Object*) { return nullptr ; }
      static double floatValue_(const Object*) { return 0.0 ; }
      static long int intValue(const Object*) { return 0 ; }
      static mpz_t bignumValue(const Object*) { return mpz_zero() ; }
      static mpq_t rationalValue(const Object*) { return mpq_zero() ; }
      static long nthInt_(const Object* obj, size_t N)
	 { const Object* val = static_cast<const List*>(obj)->nth(N) ;
           return val ? val->intValue() : 0 ; }
      static double nthFloat_(const Object* obj, size_t N)
	 { const Object* val = static_cast<const List*>(obj)->nth(N) ;
           return val ? val->floatValue() : 0 ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

   protected: // types
      typedef Fr::Initializer<List> Initializer ;

   private: // static members
      static Allocator s_allocator ;
      static Initializer s_init ;
      static List* empty_list ;
   private:
      List*   m_next ;
      Object* m_item ;
   } ;

/************************************************************************/
/************************************************************************/

class ListBuilder
   {
   private:
      List  *m_list ;
      List **m_list_end ;

   public:
      ListBuilder() : m_list(List::emptyList()), m_list_end(&m_list) {}
      ListBuilder(const ListBuilder&) = delete ;
      ListBuilder(List*&) ;  // move given list
      ListBuilder(const List*, bool) ; // copy given list
      ~ListBuilder() { clear() ; }
      ListBuilder& operator= (const ListBuilder&) = delete ;

      // retrieve contents
      operator bool () const { return m_list != List::emptyList() ; }
      List* operator * () const { return m_list ; }
      List* move() { List* l = m_list ; m_list = List::emptyList() ; return l ; }

      // discard contents
      void clear() { m_list->free() ; m_list = List::emptyList() ; }
      
      // add an element to the start of the list
      void push(Object* o) ;
      // concatenate an entire list at the front of the existing list
      void prependList(List* l) ;
      // add an element to the end of the list
      void append(Object* o) ;
      void append(const char* o) ;
      void appendClone(Object* o) ;
      // concatenate an entire list to the end of the existing list
      void appendList(List* l) ;
      // remove the first element of the list
      Object* pop() ;
      // reverse the order of elements in the list
      void reverse() ;

      ListBuilder& operator += (Object* o) { append(o) ; return *this ; }
      ListBuilder& operator += (const char* s) { append(s) ; return *this ; }
   } ;

//----------------------------------------------------------------------------
// deferred definitions of functions subject to circular dependencies

inline Object* ListIter::operator* () const { return m_list->front() ; }
inline ListIter& ListIter::operator++ () { if (m_list != List::empty_list) m_list = m_list->next() ; return *this ; }

/************************************************************************/
/************************************************************************/

void pushlist(Object*, List*&) ;

// end of namespace Fr
} ;

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::List> ;

} ; // end namespace FramepaC

#endif /* _Fr_LIST_H_INCLUDED */

// end of file list.h //
