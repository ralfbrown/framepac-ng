/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-04					*/
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

#include "framepac/list.h"
#include "framepac/fasthash64.h"
#include "framepac/init.h"

using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

// define the static members of List
Allocator List::s_allocator(FramepaC::Object_VMT<List>::instance(),sizeof(List)) ;
List::Initializer List::s_init ;
const char List::s_typename[] = "List" ;
List* List::empty_list ;

// register initialization and cleanup functions for the List class as a whole
// these will be called by Fr::Initialize() and Fr::Shutdown()
Fr::Initializer<List> static_init ;

/************************************************************************/
/************************************************************************/

List::List() : m_next(nullptr), m_item(nullptr)
{
   return ;
}

//----------------------------------------------------------------------------

List::List(const List* orig) : m_next(orig->m_next), m_item(orig->m_item)
{
//FIXME
   return ;
}

//----------------------------------------------------------------------------

List::List(const List& orig) : Object(), m_next(orig.m_next), m_item(orig.m_item)
{
//FIXME
   return ;
}

//----------------------------------------------------------------------------

List* List::create()
{
   return empty_list ;
}

//----------------------------------------------------------------------------

List* List::create(Object *obj)
{
   List* l { new List };
   l->m_next = empty_list ;
   l->m_item = obj ;
   return l ;
}

//----------------------------------------------------------------------------

List* List::create(Object* obj1, Object* obj2)
{
   List* tail { create(obj2) };
   List* l = new List ;
   l->m_next = tail ;
   l->m_item = obj1 ;
   return l ;
}

//----------------------------------------------------------------------------

List* List::create(Object* obj1, Object* obj2, Object* obj3)
{
   List* tail { create(obj2,obj3) };
   List* l = new List ;
   l->m_next = tail ;
   l->m_item = obj1 ;
   return l ;
}

//----------------------------------------------------------------------------

List* List::create(Object* obj1, Object* obj2, Object* obj3, Object* obj4)
{
   List* tail { create(obj2,obj3,obj4) };
   List* l = new List ;
   l->m_next = tail ;
   l->m_item = obj1 ;
   return l ;
}

//----------------------------------------------------------------------------

List* List::create(Object* obj1, Object* obj2, Object* obj3, Object* obj4, Object* obj5)
{
   List* tail { create(obj2,obj3,obj4,obj5) };
   List* l = new List ;
   l->m_next = tail ;
   l->m_item = obj1 ;
   return l ;
}

//----------------------------------------------------------------------------

List* List::create(const char*& string_rep)
{
   if (!string_rep || string_rep[0] != '(')
      {
      return empty_list ;
      }
   ++string_rep ;			// consume the opening paren
   ListBuilder lb ;
   // until we hit the closing paren or end of string, convert items to Objects
   //   and add to the list builder
   while (*string_rep)
      {
      // skip leading whitespace
      while (*string_rep && isspace(*string_rep))
	 ++string_rep ;
      // check for end of list
      if (*string_rep == ')')
	 {
	 ++string_rep ;
	 break ;
	 }
      else if (!*string_rep)
	 break ;
      // convert the next object in the list
      lb += Object::create(string_rep) ;
      }
   return lb.move() ;
}

//----------------------------------------------------------------------------

List* List::create(istream&)
{
   //FIXME
   return empty_list ;
}

//----------------------------------------------------------------------------

bool List::member(const Object* o) const
{
   const List* l = this ;
   if (l)
      {
      for (auto f : *l)
	 {
	 if (f == o || (f && f->equal(o)))
	    return true ;
	 }
      }
   return false ;
}

//----------------------------------------------------------------------------

Object* List::member(const Object* o, ObjectCompareFn *fn) const
{
   const List* l = this ;
   if (l && o)
      {
      for (auto f : *l)
	 {
	 if (fn(f,o)) return f ;
	 }
      }
   return nullptr ;
}

//----------------------------------------------------------------------------

const List* List::assoc(const Object* key) const
{
   for (const auto a : *this)
      {
      if (a && a->isList() && a->front() && a->front()->equal(key))
	 return static_cast<const List*>(a) ;
      }
   return empty_list ;
}

//----------------------------------------------------------------------------

List* List::pop(Object*& obj)
{
   if (this == empty_list)
      {
      obj = nullptr ;
      return this ;
      }
   obj = this->front() ;
   List* nxt = this->next() ;
   this->setNext(empty_list) ;
   this->shallowFree() ;
   return nxt ;
}

//----------------------------------------------------------------------------

List* List::elide(size_t start, size_t stop)
{
   if (stop <= start)
      return this ;
   size_t to_elide = stop - start ;
   if (start == 0)
      {
      List* l = this ;
      while (l && l != emptyList() && to_elide > 0)
	 {
	 List* tmp = l->next() ;
	 l->setNext(nullptr) ;
	 l->free() ;
	 l = tmp ;
	 --to_elide ;
	 }
      return l ;
      }
   else
      {
      List* prev = nthcdr(start-1) ;
      List* curr = prev->next() ;
      while (curr && curr != emptyList() && to_elide > 0)
	 {
	 List* tmp = curr->next() ;
	 curr->setNext(nullptr) ;
	 curr->free() ;
	 curr = tmp ;
	 --to_elide ;
	 }
      prev->setNext(curr) ;
      return this ;
      }
}

//----------------------------------------------------------------------------

List* List::nconc(List* newtail)
{
   List* t = last() ;
   if (t == empty_list) return newtail ;
   t->setNext(newtail) ;
   return this ;
}

//----------------------------------------------------------------------------

List* List::removeIf(ObjectPredicateFn* fn)
{
   if (!fn) return this ;
   ListBuilder result ;
   for (auto item : *this)
      {
      if (!fn(item))
	 result += item ;
      else if (item)
	 item->free() ;
      }
   this->shallowFree() ;
   return result.move() ;
}

//----------------------------------------------------------------------------

List* List::removeIf(ObjectCompareFn* fn, const Object* other)
{
   if (!fn) return this ;
   ListBuilder result ;
   for (auto item : *this)
      {
      if (!fn(item,other))
	 result += item ;
      else if (item)
	 item->free() ;
      }
   this->shallowFree() ;
   return result.move() ;
}

//----------------------------------------------------------------------------

List* List::removeIfNot(ObjectPredicateFn* fn)
{
   if (!fn) return this ;
   ListBuilder result ;
   for (auto item : *this)
      {
      if (fn(item))
	 result += item ;
      else if (item)
	 item->free() ;
      }
   this->shallowFree() ;
   return result.move() ;
}

//----------------------------------------------------------------------------

List* List::removeIfNot(ObjectCompareFn* fn, const Object* other)
{
   if (!fn) return this ;
   ListBuilder result ;
   for (auto item : *this)
      {
      if (fn(item,other))
	 result += item ;
      else if (item)
	 item->free() ;
      }
   this->shallowFree() ;
   return result.move() ;
}

//----------------------------------------------------------------------------

ObjectPtr List::clone_(const Object *obj)
{
   const List *old_l { static_cast<const List*>(obj) };
   ListBuilder newlist ;
   for (auto item : *old_l)
      {
      newlist.appendClone(item) ;
      }
   return ObjectPtr(newlist.move()) ;
}

//----------------------------------------------------------------------------

Object *List::shallowCopy_(const Object *obj)
{
   const List *old_l { static_cast<const List*>(obj) };
   ListBuilder newlist ;
   for (auto item : *old_l)
      {
      newlist += item ;
      }
   return newlist.move() ;
}

//----------------------------------------------------------------------------

void List::free_(Object *obj)
{
   List *l { static_cast<List*>(obj) };
   while (l && l != empty_list)
      {
      List *n { l->next() };
      if (l->m_item) l->m_item->free() ;
      delete l ;
      l = n ;
      }
   return ;
}

//----------------------------------------------------------------------------

void List::shallowFree_(Object *obj)
{
   List *l { static_cast<List*>(obj) };
   while (l && l != empty_list)
      {
      List *n = l->next() ;
      delete l ;
      l = n ;
      }
   return ;
}

//----------------------------------------------------------------------------

bool List::contains(const Object* o) const
{
   const List* l = this ;
   while (l && l != empty_list)
      {
      Object* f = l->front() ;
      if (!f && !o) return true ;
      if (f->equal(o)) return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

List* List::last() const
{
   const List* l = this ;
   if (l == emptyList() || l == nullptr)
      return emptyList() ;
   while (l->next() != emptyList())
      l = l->next() ;
   return const_cast<List*>(l) ;
}

//----------------------------------------------------------------------------

List* List::reverse()
{
   List* l = this ;
   List* prev = emptyList() ;
   if (l == prev)
      return l ;
   List* nxt ;
   while ((nxt = l->next()) != emptyList())
      {
      l->setNext(prev) ;
      prev = l ;
      l = nxt ;
      }
   return l ;
}

//----------------------------------------------------------------------------

static List* merge(List* l1, List* l2, ObjectOrderingFn* cmp)
{
   if (!l2 || !*l2)
      return l1 ;
   if (!l1)
      return l2 ;
   List *result ;
   if (cmp(l1->front(),l2->front()) <= 0)
      {
      result = l1 ;
      l1 = l1->next() ;
      }
   else
      {
      result = l2 ;
      l2 = l2->next() ;
      }
   List* prev = result ;
   while (*l1 && *l2)
      {
      if (cmp(l1->front(),l2->front()) <= 0)
	 {
	 prev->setNext(l1) ;		// glue item from first list onto end of result
	 prev = l1 ;
	 l1 = l1->next() ;		// then advance down the first list
	 }
      else
	 {
	 prev->setNext(l2) ;		// glue item from second list onto end of result
	 prev = l2 ;
	 l2 = l2->next() ;		// then advance down the second list
	 }
      }
   if (*l1)
      prev->setNext(l1) ;
   else
      prev->setNext(l2) ;
   return result ;
}

//----------------------------------------------------------------------------

List* List::sort(ObjectOrderingFn* cmp)
{
   if (!cmp || next() == emptyList())	// single-element/empty list or no compare func?
      return this ;			// nothing to be done
   List *sorted[CHAR_BIT * sizeof(size_t)] ; // sorted sublists, with lengths in powers of two
   List* list = this ;
   // initialize sublists
   size_t maxsize = 0 ;
   sorted[0] = nullptr ;
   // scan down the input, creating sorted sublists as we go
   while (list != emptyList())
      {
      // remove the head node from the list
      List* sublist = list ;
      list = list->next() ;
      sublist->setNext(emptyList()) ;

      // merge with successively longer sublists until we get to a currently-unused power of two
      size_t i ;
      for (i = 0 ; i <= maxsize && sorted[i] ; ++i)
	 {
	 sublist = merge(sorted[i],sublist,cmp) ;
	 sorted[i] = nullptr ;
	 }
      sorted[i] = sublist ;
      if (i > maxsize)
	 ++maxsize ;
      }
   // we've now consumed all the input, so just merge the remaining already-sorted sublists
   List* result = sorted[0] ;
   for (size_t i = 1 ; i <= maxsize ; ++i)
      {
      if (sorted[i])
	 result = merge(sorted[i],result,cmp) ;
      }
   return result ;
}

//----------------------------------------------------------------------------

Object* List::nth(size_t N) const
{
   const List* l { this } ;
   for ( ; N > 0 && l && l != empty_list ; --N)
      {
      l = l->next() ;
      }
   return l->front() ;
}

//----------------------------------------------------------------------------

List* List::nthcdr(size_t N)
{
   List* l { this } ;
   for ( ; N > 0 && l && l != empty_list ; --N)
      {
      l = l->next() ;
      }
   return l ;
}

//----------------------------------------------------------------------------

const List* List::nthcdr(size_t N) const
{
   const List* l { this } ;
   for ( ; N > 0 && l && l != empty_list ; --N)
      {
      l = l->next() ;
      }
   return l ;
}

//----------------------------------------------------------------------------

List *List::subseq(ListIter start, ListIter stop, bool shallow) const
{
   ListBuilder copy ;
   for ( ; start != stop ; ++start)
      {
      if (shallow)
	 copy.append(*start) ;
      else
	 copy.appendClone(*start) ;
      }
   return copy.move() ;
}

//----------------------------------------------------------------------------

ObjectPtr List::subseq_int(const Object *obj, size_t start, size_t stop)
{
   const List *l = static_cast<const List*>(obj) ;
   ListBuilder copy ;
   if (l)
      {
      if (start > stop) std::swap(start,stop) ;
      l = l->nthcdr(start) ;
      if (!l || l == empty_list)
	 return ObjectPtr(empty_list) ;
      stop -= start ;
      for ( ; l != empty_list && stop > 0 ; --stop, l = l->next())
	 {
	 copy.appendClone(l->front()) ;
	 }
      }
   return ObjectPtr(copy.move()) ;
}

//----------------------------------------------------------------------------

ObjectPtr List::subseq_iter(const Object *, ObjectIter start, ObjectIter stop)
{
   ListBuilder copy ;
   for ( ; start != stop ; ++start)
      {
      copy.appendClone(*start) ;
      }
   return ObjectPtr(copy.move()) ;
}

//----------------------------------------------------------------------------

size_t List::cStringLength_(const Object* obj, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   const List* list = static_cast<const List*>(obj) ;
   size_t len = indent + 2 ;
   bool wrapped { false } ;
   for (size_t i = 0 ; list && list != empty_list ; ++i, list = list->next())
      {
      size_t objlen = list->front()->cStringLength(wrap_at,wrapped?indent+1:(i?1:0),wrapped_indent) ;
      len += objlen ;
      }
   return len ;
}

//----------------------------------------------------------------------------

char* List::toCstring_(const Object* obj, char* buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   if (buflen < indent+2) return buffer ;
   char* bufend = buffer + buflen ;
   for (size_t i = 0 ; i < indent ; ++i)
      {
      *buffer++ = ' ' ;
      }
   *buffer++ = '(' ;
   bool wrapped { false } ;
   const List* list = static_cast<const List*>(obj) ;
   for (size_t i = 0 ; list && list != empty_list ; ++i, list = list->next())
      {
      buffer = list->front()->toCstring(buffer,bufend-buffer,wrap_at,wrapped?indent+1:(i?1:0),wrapped_indent) ;
      }
   *buffer++ = ')' ;
   *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t List::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)wrap; //TODO
   size_t len = indent + 2 ; // initial and trailing brackets
   size_t items = 0 ;
   for (const Object* o : *static_cast<const List*>(obj))
      {
      len += o->jsonStringLength(wrap,0) ;
      }
   return len + (items ? items-1 : 0) ;
}

//----------------------------------------------------------------------------

bool List::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //TODO
}

//----------------------------------------------------------------------------

size_t List::size() const
{
   size_t sz(0) ;
   for (const List *l = this ; l && l != empty_list ; l = l->next())
      {
      sz++ ;
      }
   return sz ;
}

//----------------------------------------------------------------------------

size_t List::size_(const Object *obj)
{
   size_t sz(0) ;
   for (const List *l = static_cast<const List*>(obj) ; l && l != empty_list ; l = l->next())
      {
      sz++ ;
      }
   return sz ;
}

//----------------------------------------------------------------------------

Object *List::front_(Object *obj)
{
   List *l { reinterpret_cast<List*>(obj) };
   return l->m_item ;
}

//----------------------------------------------------------------------------

const Object *List::front_(const Object *obj)
{
   const List *l { reinterpret_cast<const List*>(obj) };
   return l->m_item ;
}

//----------------------------------------------------------------------------

size_t List::hashValue_(const Object* obj)
{
   // the hash value of a list is the hash of the hash values of its elements
   FastHash64 hash(size_(obj)) ;
   for (auto elt : *reinterpret_cast<const List*>(obj))
      {
      if (elt) hash += elt->hashValue() ;
      }
   return (size_t)*hash ;
}

//----------------------------------------------------------------------------

bool List::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;
   if (!obj || !other || !other->isList())
      return false ;
   auto l1 = static_cast<const List*>(obj) ;
   auto l2 = static_cast<const List*>(other) ;
   while (l1 != empty_list && l2 != empty_list)
      {
      auto o1 = l1->front() ;
      auto o2 = l2->front() ;
      if (o1 != o2 && (!o1 || !o2 || !o1->equal(o2)))
	 return false ;
      l1 = l1->next() ;
      l2 = l2->next() ;
      }
   return l1 == empty_list && l2 == empty_list ;// lists are equal if we've consumed both in their entirety
}

//----------------------------------------------------------------------------

int List::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;
   if (!obj) return -1 ;
   if (!other || !other->isList())
      return +1 ;
   auto l1 = static_cast<const List*>(obj) ;
   auto l2 = static_cast<const List*>(other) ;
   while (l1 && l1 != empty_list && l2 && l2 != empty_list)
      {
      auto o1 = l1->front() ;
      auto o2 = l2->front() ;
      int cmp = o1->compare(o2) ;
      if (cmp)
	 return cmp ;
      }
   if (l1 && l1 != empty_list) return +1 ;
   else if (l2 && l2 != empty_list) return -1 ;
   return 0 ;				// we got all the way to the end, so they're equal
}

//----------------------------------------------------------------------------

int List::lessThan_(const Object *obj, const Object *other)
{
   return compare_(obj,other) < 0 ;
}


//----------------------------------------------------------------------------

void List::StaticInitialization()
{
   empty_list = new List ;
   // point both head and tail of the empty list at itself; if we somehow wind
   //   up trying to deref the empty list, we'll get back another empty list
   //   instead of a null pointer
   empty_list->m_item = empty_list ;
   empty_list->m_next = empty_list ;
   return ;
}

//----------------------------------------------------------------------------

void List::StaticCleanup()
{
   delete empty_list ;
   empty_list = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

/************************************************************************/
/*	Procedural Interface functions					*/
/************************************************************************/

void pushlist(Object* obj, List*& list)
{
   List* newhead = List::create(obj) ;
   newhead->setNext(list) ;
   list = newhead ;
   return ;
}

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<List>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::List> ;

} // end namespace FramepaC

// end of file list.C //
