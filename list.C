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

List* List::create(const char* /*string_rep*/)
{

   return empty_list ; //FIXME
}

//----------------------------------------------------------------------------

List* List::create(istream&)
{
   //FIXME
   return empty_list ;
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
   return ObjectPtr(*newlist) ;
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
   return *newlist ;
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

List* List::last() const
{
   if (this == emptyList())
      return const_cast<List*>(this) ;
   const List* l = this ;
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
   List* next ;
   while ((next = l->next()) != emptyList())
      {
      l->setNext(prev) ;
      prev = l ;
      l = next ;
      }
   return l ;
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
   return *copy ;
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
   return ObjectPtr(*copy) ;
}

//----------------------------------------------------------------------------

ObjectPtr List::subseq_iter(const Object *, ObjectIter start, ObjectIter stop)
{
   ListBuilder copy ;
   for ( ; start != stop ; ++start)
      {
      copy.appendClone(*start) ;
      }
   return ObjectPtr(*copy) ;
}

//----------------------------------------------------------------------------

size_t List::cStringLength_(const Object* obj, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   const List* list = static_cast<const List*>(obj) ;
   size_t len = indent + 2 ;
   bool wrapped { false } ;
   (void)wrapped_indent; //FIXME
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
   return buffer ;
}

//----------------------------------------------------------------------------

size_t List::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)wrap; //FIXME
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
   return false ; //FIXME
}

//----------------------------------------------------------------------------

size_t List::size() const
{
   size_t sz(0) ;
   for (const List *l = this ; l != empty_list ; l = l->next())
      {
      sz++ ;
      }
   return sz ;
}

//----------------------------------------------------------------------------

size_t List::size_(const Object *obj)
{
   size_t sz(0) ;
   for (const List *l = static_cast<const List*>(obj) ; l != empty_list ; l = l->next())
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
   size_t len = size_(obj) ;
   uint64_t hashstate = fasthash64_init(len) ;
   for (auto elt : *reinterpret_cast<const List*>(obj))
      {
      if (elt) hashstate = fasthash64_add(hashstate,elt->hashValue()) ;
      }
   return (size_t)fasthash64_finalize(hashstate) ;
}

//----------------------------------------------------------------------------

bool List::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int List::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int List::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
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
