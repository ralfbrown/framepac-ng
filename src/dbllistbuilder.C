/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-13					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018,2019 Carnegie Mellon University			*/
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

DblListBuilder::DblListBuilder(DblList*& init_list)
{
   m_list = init_list ? init_list : DblList::emptyList() ;
   auto tail = m_list->last() ;
   m_list_end = tail->nextPtr() ;
   init_list = DblList::emptyList() ;
   return ;
}

//----------------------------------------------------------------------------

DblListBuilder::DblListBuilder(const DblList* init_list, bool)
{
   if (init_list == nullptr) init_list = DblList::emptyList() ;
   m_list = static_cast<DblList*>(init_list->clone().move()) ;
   auto tail = m_list->last() ;
   m_list_end = tail->nextPtr() ;
   return ;
}

//----------------------------------------------------------------------------

void DblListBuilder::push(Object* o)
{
   auto new_l = DblList::create(o) ;
   new_l->setNext(m_list) ;
   if (m_list != DblList::emptyList())
      m_list->setPrev(new_l) ;
   m_list = new_l ;
   return ;
}

//----------------------------------------------------------------------------

void DblListBuilder::prependDblList(DblList* l)
{
   if (!l || l == DblList::emptyList())
      return ;
   auto tail = l->last() ;
   tail->setNext(m_list) ;
   if (m_list != DblList::emptyList())
      m_list->setPrev(tail) ;
   m_list = l ;
   return ;
}

//----------------------------------------------------------------------------

void DblListBuilder::append(Object* o)
{
   if (!o)
      o = DblList::emptyList() ;
   if (m_list == DblList::emptyList())
      {
      m_list = DblList::create(o) ;
      m_list_end = m_list->nextPtr() ;
      return ;
      }
   auto newelt = DblList::create(o) ;
//TODO:   newelt->setPrev(*m_list_end) ;
   *m_list_end = newelt ;
   m_list_end = newelt->nextPtr() ;
   return ;
}

//----------------------------------------------------------------------------

void DblListBuilder::append(const char* s)
{
   if (s)
      {
      append(String::create(s)) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void DblListBuilder::appendClone(Object* o)
{
   if (o)
      {
      append(o->clone().move()) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void DblListBuilder::appendList(List* l)
{
   auto tail = l->last() ;
   if (m_list == DblList::emptyList())
      {
      m_list = static_cast<DblList*>(l) ;
      }
   else
      {
      *m_list_end = l ;
      }
   m_list_end = tail->nextPtr() ;
   return ;
}

//----------------------------------------------------------------------------

Object* DblListBuilder::pop()
{
   if (m_list == DblList::emptyList())
      return nullptr ;
   auto elt = m_list ;
   m_list = m_list->next() ;
   if (m_list == DblList::emptyList()) m_list_end = (List**)&m_list ;
   // chop the element out of the list, extract the item it contains, and then free the node
   auto o = elt->front() ;
   elt->setNext(nullptr) ;
   elt->shallowFree() ;
   return o ;
}

//----------------------------------------------------------------------------

void DblListBuilder::reverse()
{
   auto prev = DblList::emptyList() ;
   auto curr = m_list ;
   // swap the head and tail pointers
   m_list_end = m_list->nextPtr() ;
   // now iterate along the list, reversing the links
   while (curr != DblList::emptyList())
      {
      auto next = curr->next() ;
      curr->setNext(prev) ;
      m_list = curr ;
      prev = curr ;
      curr = next ;
      }
   return ;
}


} // end of namespace Fr

// end of file listbuilder.C //

