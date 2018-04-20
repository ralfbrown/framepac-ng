/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-19					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#include "framepac/array.h"

using namespace FramepaC ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

Allocator RefArray::s_allocator(FramepaC::Object_VMT<RefArray>::instance(),sizeof(RefArray)) ;

/************************************************************************/
/************************************************************************/

RefArray::RefArray(const Array* orig)
   : Array()
{
   if (orig)
      {
      m_size = orig->size() ;
      m_array = new Object*[m_size] ;
      m_alloc = m_size ;
      for (size_t i = 0 ; i < m_size ; i++)
	 {
	 this->m_array[i] = orig->getNth(i) ;
	 }
      }
   else
      {
      m_array = new Object*[1] ;
      }
   return ;
}

//----------------------------------------------------------------------------

RefArray::RefArray(const Object* obj, size_t repeat)
   : RefArray(repeat)
{
   (void)obj;//FIXME
   for (size_t i = 0 ; i < repeat ; i++)
      {
      m_array[i] = const_cast<Object*>(obj) ;
      }
   return ;
}

//----------------------------------------------------------------------------

RefArray::~RefArray()
{
   m_size = 0 ;
   // ~Array() will free m_array for us
   return ;
}

//----------------------------------------------------------------------------

ObjectPtr RefArray::clone_(const Object* obj)
{
   const RefArray* orig = static_cast<const RefArray*>(obj) ;
   return ObjectPtr(new RefArray(orig)) ;
}

//----------------------------------------------------------------------------

ObjectPtr RefArray::subseq_int(const Object* obj, size_t start, size_t stop)
{
   auto a = static_cast<const Array*>(obj) ;
   if (start > stop)
      start = stop ;
   RefArray* new_array = RefArray::create(stop-start) ;
   for (size_t i = start ; i < stop ; ++i)
      {
      auto o = a->getNth(i) ;
      new_array->setNth(i-start,o) ;
      }
   return ObjectPtr(new_array) ;
}

//----------------------------------------------------------------------------

ObjectPtr RefArray::subseq_iter(const Object*,ObjectIter start, ObjectIter stop)
{
   const Object* obj = start.baseObject() ;
   return subseq_int(obj,start.currentIndex(),stop.currentIndex()) ;
}

//----------------------------------------------------------------------------

void RefArray::clearArray(bool free_objects)
{
   if (free_objects)
      {
      for (size_t i = 0 ; i < size() ; ++i)
	 {
	 if (m_array[i])
	    m_array[i]->free() ;
	 }
      }
   for (size_t i = 0 ; i < size() ; ++i)
      m_array[i] = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

bool RefArray::append(Object* obj)
{
   if (m_size >= m_alloc)
      {
      size_t newsize = m_alloc < 30 ? 30 : 3 * m_alloc / 2 ;
      if (!reserve(newsize))
	 return false ;
      }
   m_array[m_size++] = obj ;
   return true ;
}

//----------------------------------------------------------------------------

bool RefArray::elide(size_t N)
{
   if (N >= m_size)
      return false ;
   for (size_t i = N ; i+1 < m_size ; ++i)
      {
      m_array[i] = m_array[i+1] ;
      }
   m_size-- ;
   return true ;
}
   
//----------------------------------------------------------------------------

void RefArray::pop_back()
{
   if (m_size > 0) --m_size ;
   return ;
}

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Array>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::RefArray> ;

} // end namespace FramepaCC

// end of file refarray.C //
