/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-09					*/
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
#include "framepac/init.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

// request explicit instantiation; we declared it "extern" in the header so this
//   will be the only copy of the non-inlined code generated in object modules
template class Allocator<List> ;

// define the static members of List
static const FramepaC::Object_VMT<List> list_vmt ;
Allocator<List> List::s_allocator(&list_vmt) ;
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

List::List(const List *orig) : m_next(orig->m_next), m_item(orig->m_item)
{
//FIXME
   return ;
}

//----------------------------------------------------------------------------

List::List(const List &orig) : Object(), m_next(orig.m_next), m_item(orig.m_item)
{
//FIXME
   return ;
}

//----------------------------------------------------------------------------

List *List::create()
{
   return empty_list ;
}

//----------------------------------------------------------------------------

List *List::create(Object *obj)
{
   List *l { new List };
   l->m_next = empty_list ;
   l->m_item = obj ;
   return l ;
}

//----------------------------------------------------------------------------

List *List::create(Object *obj1, Object *obj2)
{
   List *tail { create(obj2) };
   List *l = new List ;
   l->m_next = tail ;
   l->m_item = obj1 ;
   return l ;
}

//----------------------------------------------------------------------------

List *List::create(Object *obj1, Object *obj2, Object *obj3)
{
   List *tail { create(obj2,obj3) };
   List *l = new List ;
   l->m_next = tail ;
   l->m_item = obj1 ;
   return l ;
}

//----------------------------------------------------------------------------

List *List::create(const char * /*string_rep*/)
{

   return empty_list ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr List::clone_(const Object *obj)
{
   const List *old_l { static_cast<const List*>(obj) };
   ListBuilder newlist ;
   for (auto item : *old_l)
      {
      Object *copy = item ;
      if (item)
	 {
	 ObjectPtr cp(item->clone()) ;
	 copy = cp ;
	 cp.release() ;
	 }
      newlist += copy ;
      }
   return ObjectPtr(*newlist) ;
}

//----------------------------------------------------------------------------

Object *List::shallowCopy_(const Object *obj)
{
   const List *old_l { static_cast<const List*>(obj) };
   ListBuilder newlist ;
#if 0
   for ( ; old_l ; old_l = old_l->next())
      {
      newlist += old_l->front() ;
      }
#else
   for (auto item : *old_l) newlist += item ;
#endif
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

List *List::subseq(ListIter start, ListIter stop, bool shallow) const
{
   ListBuilder copy ;
   for ( ; start != stop ; ++start)
      {
      Object *item { *start };
      if (item && !shallow)
	 {
	 ObjectPtr cp(item->clone()) ;
	 item = cp ;
	 cp.release() ;
	 }
      copy += item ;
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
      size_t pos { 0 };
      for ( ; l != empty_list && pos < start ; pos++, l = l->next())
	 {
	 }
      if (!l)
	 return ObjectPtr(empty_list) ;
      for ( ; l != empty_list && pos < stop ; pos++, l = l->next())
	 {
	 Object *item { l->front() };
	 if (item)
	    {
	    ObjectPtr cp(item->clone()) ;
	    item = cp ;
	    cp.release() ;
	    }
	 copy += item ;
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
      Object *item { *start };
      if (item)
	 {
	 ObjectPtr cp(item->clone()) ;
	 item = cp ;
	 cp.release() ;
	 }
      copy += item ;
      }
   return ObjectPtr(*copy) ;
}

//----------------------------------------------------------------------------

size_t List::cStringLength_(const Object *, size_t wrap_at, size_t indent)
{
   (void)wrap_at; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool List::toCstring_(const Object *, char *buffer, size_t buflen, size_t wrap_at, size_t indent)
{
   (void)buffer; (void)buflen; (void)wrap_at; (void)indent; //FIXME
   //FIXME
   return true ;
}

//----------------------------------------------------------------------------

size_t List::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ; (void)wrap; (void)indent ;
   return 0 ; //FIXME
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
   const List *l { this };
   while (l != empty_list)
      {
      sz++ ;
      l = l->next() ;
      }
   return sz ;
}

//----------------------------------------------------------------------------

size_t List::size_(const Object *obj)
{
   size_t sz(0) ;
   const List *l { static_cast<const List*>(obj) };
   while (l != empty_list)
      {
      sz++ ;
      l = l->next() ;
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
