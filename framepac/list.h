/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-27					*/
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

#ifndef _Fr_LIST_H_INCLUDED
#define _Fr_LIST_H_INCLUDED

#include "framepac/object.h"

namespace Fr {

// forward declarations
class List ;
class DblList ;
class ListBuilder ;
class DblListBuilder ;

//----------------------------------------------------------------------------
// due to the circular dependencies, we can't actually define many of the functions
//   inline in the iterator class definition; they will be defined after the underlying
//   class has been declared

class ListIter
   {
   public:
      ListIter(List* l) : m_list(l) {}
      ListIter(const List* l) : m_list(const_cast<List*>(l)) {}
      ListIter(const ListIter& o) : m_list(o.m_list) {}
      ListIter(ObjectIter& o) : m_list((List*)o.baseObject()) {}
      ~ListIter() = default ;

      inline Object* operator* () const ;
      List* operator-> () const { return  m_list ; }
      inline ListIter& operator++ () ;
      inline ListIter operator++ (int) ;
      bool operator== (const ListIter& other) const { return m_list == other.m_list ; }
      bool operator!= (const ListIter& other) const { return m_list != other.m_list ; }
   private:
      List*  m_list ;
   } ;

//----------------------------------------------------------------------------

// singly-linkd list
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

      List* removeIf(ObjectCompareFn*, const Object*) ;
      List* removeIf(ObjectPredicateFn*) ;
      List* removeIfNot(ObjectPredicateFn*) ;
      List* removeIfNot(ObjectCompareFn*, const Object*) ;

      void setFront(Object* o) { m_item = o ? o : empty_list ; }
      void replaceFront(Object* o) { m_item->free() ; setFront(o) ; }

      // *** standard info functions ***
      size_t size() const ;
      bool empty() const { return this == empty_list ; }
      operator bool () const { return /*this != nullptr &&*/ !this->empty() ; }

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
      bool isLast() const { return m_next == empty_list ; }
      List* last() const ;
      List* reverse() ;
      List* sort(ObjectOrderingFn*) ;
      Object* nth(size_t N) const ;
      List* nthcdr(size_t N) ;
      const List* nthcdr(size_t N) const ;
      const List* assoc(const Object* key) const ;

      void setNext(List* nxt) { m_next = nxt ; }
      List** nextPtr() { return &m_next ; }
      List* const* nextPtr() const { return &m_next ; }
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

   protected:
      static const char s_typename[] ;
      static List* empty_list ;
      List*   m_next ;
      Object* m_item ;

   private: // static members
      static Allocator s_allocator ;
      static Initializer s_init ;
   } ;

//----------------------------------------------------------------------------

template<>
inline Ptr<List>::Ptr() : m_object(List::emptyList()) { }

template<>
inline void Ptr<List>::release() { m_object = List::emptyList() ; }

template<>
inline Ptr<List>::operator bool() const { return m_object && m_object != List::emptyList() ; }

template<>
inline bool Ptr<List>::operator! () const { return !m_object || m_object == List::emptyList() ; }

typedef Ptr<List> ListPtr ;

//----------------------------------------------------------------------------
// deferred definitions of functions subject to circular dependencies

inline Object* ListIter::operator* () const { return m_list->front() ; }
inline ListIter& ListIter::operator++ () { if (m_list != List::empty_list) m_list = m_list->next() ; return *this ; }
inline ListIter ListIter::operator++ (int)
   { ListIter copy(*this) ; if (m_list != List::empty_list) m_list = m_list->next() ; return copy ; }

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------
// due to the circular dependencies, we can't actually define many of the functions
//   inline in the iterator class definition; they will be defined after the underlying
//   class has been declared

class DblListIter
   {
   public:
      DblListIter(DblList* l) : m_list(l) {}
      DblListIter(const DblList* l) : m_list(const_cast<DblList*>(l)) {}
      DblListIter(const DblListIter& o) : m_list(o.m_list) {}
      DblListIter(ObjectIter& o) : m_list((DblList*)o.baseObject()) {}
      ~DblListIter() = default ;

      inline Object* operator* () const ;
      DblList* operator-> () const { return  m_list ; }
      inline DblListIter& operator++ () ;
      inline DblListIter operator++ (int) ;
      inline DblListIter& operator-- () ;
      inline DblListIter operator-- (int) ;
      bool operator== (const DblListIter& other) const { return m_list == other.m_list ; }
      bool operator!= (const DblListIter& other) const { return m_list != other.m_list ; }

   private:
      DblList*	m_list ;
   } ;

//----------------------------------------------------------------------------

// doubly-linked list
class DblList : public List
   {
   public: // types
      typedef List super ;
   public:
      // *** object factories ***
      static DblList* create() ;
      static DblList* create(Object*) ;
      static DblList* create(Object*, Object*) ;
      static DblList* create(Object*, Object*, Object*) ;
      static DblList* create(Object*, Object*, Object*, Object*) ;
      static DblList* create(Object*, Object*, Object*, Object*, Object*) ;

      DblList* next() const { return static_cast<DblList*>(m_next) ; }
      DblList* prev() const { return m_prev ; }
      static DblList* emptyList() { return static_cast<DblList*>(empty_list) ; }
      //TODO

      void setPrev(List* prv) { m_next = static_cast<DblList*>(prv) ; }

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      DblList() ;
      DblList(const List*) ;
      DblList(const List&) ;
      ~DblList() {}
      DblList& operator= (const DblList&) = delete ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<DblList> ;

   protected: // types
      typedef Fr::Initializer<DblList> Initializer ;

   protected: // data members
      DblList* m_prev { static_cast<DblList*>(List::empty_list) } ;

   private: // static members
      static Allocator s_allocator ;
      static Initializer s_init ;
   } ;

//----------------------------------------------------------------------------

template <>
inline Ptr<DblList>::Ptr() : m_object(DblList::emptyList()) { }

typedef Ptr<DblList> DblListPtr ;

//----------------------------------------------------------------------------
// deferred definitions of functions subject to circular dependencies

inline Object* DblListIter::operator* () const { return m_list->front() ; }
inline DblListIter& DblListIter::operator++ ()
   { if (m_list != DblList::emptyList()) m_list = m_list->next() ; return *this ; }
inline DblListIter DblListIter::operator++ (int)
   { DblListIter copy(*this) ; if (m_list != DblList::emptyList()) m_list = m_list->next() ; return copy ; }
inline DblListIter& DblListIter::operator-- ()
   { if (m_list != DblList::emptyList()) m_list = m_list->prev() ; return *this ; }
inline DblListIter DblListIter::operator-- (int)
   { DblListIter copy(*this) ; if (m_list != DblList::emptyList()) m_list = m_list->prev() ; return copy ; }

/************************************************************************/
/************************************************************************/

class ListBuilder
   {
   public:
      ListBuilder() : m_list(List::emptyList()), m_list_end(&m_list) {}
      ListBuilder(const ListBuilder&) = delete ;
      ListBuilder(List*&) ;  // move given list
      ListBuilder(const List*, bool) ; // copy given list
      ~ListBuilder() { clear() ; }
      ListBuilder& operator= (const ListBuilder&) = delete ;

      // retrieve contents
      operator bool () const { return m_list != List::emptyList() ; }
      operator ListPtr () { return move() ; }
      List* operator * () const { return m_list ; }
      List* operator-> () const { return m_list ; }
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

   private:
      List*  m_list ;
      List** m_list_end ;
   } ;

/************************************************************************/
/************************************************************************/

class DblListBuilder
   {
   public:
      DblListBuilder() : m_list(DblList::emptyList()), m_list_end((List**)&m_list) {}
      DblListBuilder(const ListBuilder&) = delete ;
      DblListBuilder(DblList*&) ;  // move given list
      DblListBuilder(const DblList*, bool) ; // copy given list
      ~DblListBuilder() { clear() ; }
      DblListBuilder& operator= (const ListBuilder&) = delete ;

      // retrieve contents
      operator bool () const { return m_list != DblList::emptyList() ; }
      DblList* operator * () const { return m_list ; }
      DblList* operator-> () const { return m_list ; }
      DblList* move() { auto l = m_list ; m_list = DblList::emptyList() ; return l ; }

      // discard contents
      void clear() { m_list->free() ; m_list = DblList::emptyList() ; }
      
      // add an element to the start of the list
      void push(Object* o) ;
      // concatenate an entire list at the front of the existing list
      void prependList(List* l) ;
      void prependDblList(DblList* l) ;
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

      DblListBuilder& operator += (Object* o) { append(o) ; return *this ; }
      DblListBuilder& operator += (const char* s) { append(s) ; return *this ; }

   private:
      DblList* m_list ;
      List**   m_list_end ;
   } ;

/************************************************************************/
/************************************************************************/

void pushlist(Object*, List*&) ;
Object* poplist(List*&) ;
Object* poplist(ListPtr&) ;

} // end of namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::List> ;

} // end namespace FramepaC

#endif /* _Fr_LIST_H_INCLUDED */

// end of file list.h //
