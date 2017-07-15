/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-12					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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
#include "framepac/string.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

ListBuilder::ListBuilder(List*&& init_list)
{
   m_list = init_list ;
   List* tail = m_list ;
   if (tail != List::emptyList())
      {
      while (tail->next() != List::emptyList())
	 tail = tail->next() ;
      }
   m_list_end = tail->nextPtr() ;
   init_list = List::emptyList() ;
   return ;
}

//----------------------------------------------------------------------------

void ListBuilder::push(Object* o)
{
   List* new_l = List::create(o) ;
   new_l->setNext(m_list) ;
   m_list = new_l ;
   return ;
}

//----------------------------------------------------------------------------

void ListBuilder::prependList(List* l)
{
   if (l == List::emptyList())
      return ;
   List* tail = l->last() ;
   tail->setNext(m_list) ;
   m_list = l ;
   return ;
}

//----------------------------------------------------------------------------

void ListBuilder::append(Object* o)
{
   if (m_list == List::emptyList())
      {
      m_list = List::create(o) ;
      m_list_end = m_list->nextPtr() ;
      return ;
      }
   List* newelt = List::create(o) ;
   *m_list_end = newelt ;
   m_list_end = newelt->nextPtr() ;
   return ;
}

//----------------------------------------------------------------------------

void ListBuilder::append(const char* s)
{
   if (s)
      {
      append(String::create(s)) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void ListBuilder::appendClone(Object* o)
{
   if (o)
      {
      ObjectPtr cp(o->clone()) ;
      o = cp ;
      cp.release() ;
      }
   append(o) ;
   return ;
}

//----------------------------------------------------------------------------

void ListBuilder::appendList(List* l)
{
   List* tail = l->last() ;
   if (m_list == List::emptyList())
      {
      m_list = l ;
      }
   else
      {
      *m_list_end = l ;
      }
   m_list_end = tail->nextPtr() ;
   return ;
}

//----------------------------------------------------------------------------

Object* ListBuilder::pop()
{
   if (m_list == List::emptyList())
      return nullptr ;
   List* elt = m_list ;
   m_list = m_list->next() ;
   if (m_list == List::emptyList()) m_list_end = &m_list ;
   // chop the element out of the list, extract the item it contains, and then free the node
   Object* o = elt->front() ;
   elt->setNext(nullptr) ;
   elt->shallowFree() ;
   return o ;
}

//----------------------------------------------------------------------------

void ListBuilder::reverse()
{
//FIXME
   return ;
}


} // end of namespace Fr

// end of file listbuilder.C //

