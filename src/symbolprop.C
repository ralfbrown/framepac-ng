/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-17					*/
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

#include "framepac/frame.h"
#include "framepac/list.h"
#include "framepac/symbol.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class SymbolProperties				*/
/************************************************************************/

SymbolProperties::SymbolProperties()
   : m_invrelation(nullptr), m_frame(nullptr), m_plist(List::emptyList())
{
   return ;
}

//----------------------------------------------------------------------------

SymbolProperties::~SymbolProperties()
{
   if (m_frame)
      m_frame->free() ;
   m_invrelation = nullptr ;
   m_plist->free() ;
   return ;
}

//----------------------------------------------------------------------------

void SymbolProperties::frame(Frame* f)
{
   if (m_frame)
      m_frame->free() ;
   m_frame = f ;
   return ;
}

//----------------------------------------------------------------------------

Object* SymbolProperties::getProperty(Symbol* propname) const
{
   const List* a = m_plist->assoc(propname) ;
   return a ? a->nth(1) : nullptr ;
}

//----------------------------------------------------------------------------

void SymbolProperties::setProperty(Symbol* propname, Object* value)
{
   List* a = const_cast<List*>(m_plist->assoc(propname)) ;
   if (a)
      {
      // replace the existing value
      List* tail = a->next() ;
      if (tail)
	 {
	 tail->replaceFront(value) ;
	 }
      else
	 {
	 a->setNext(List::create(value)) ;
	 }
      }
   m_plist = m_plist->push(List::create(propname,value)) ;
   return ;
}


} // end namespace Fr


// end of file symbolprop.C //
