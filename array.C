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

#include "framepac/array.h"
#include "framepac/fasthash64.h"

using namespace FramepaC ;
using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

Allocator Array::s_allocator(FramepaC::Object_VMT<Array>::instance(),sizeof(Array)) ;

/************************************************************************/
/************************************************************************/

Array::Array(size_t initial_size)
   : m_array(new Object*[initial_size ? initial_size : 1]),
     m_size(initial_size),
     m_alloc(initial_size ? initial_size : 1)
{
   for (size_t i = 0 ; i < initial_size ; i++)
      m_array[i] = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

Array::Array(const Array *orig)
   : Object(),
     m_array(nullptr),
     m_size(0),
     m_alloc(0)
{
   if (orig)
      {
      m_size = orig->size() ;
      m_array = new Object*[m_size] ;
      m_alloc = m_size ;
      for (size_t i = 0 ; i < m_size ; i++)
	 {
	 Object *elt { nullptr }; //FIXME: orig[i] ;
	 Object *copy = nullptr ;
	 if (elt)
	    {
	    ObjectPtr cp = elt->clone() ;
	    copy = cp ;
	    cp.release() ;
	    }
	 m_array[i] = copy ;
	 }
      }
   else
      {
      m_array = new Object*[1] ;
      }
   return ;
}

//----------------------------------------------------------------------------

Array::Array(const Object *obj, size_t repeat)
   : Array(repeat)
{
   (void)obj;//FIXME
   for (size_t i = 0 ; i < repeat ; i++)
      {
//!!!      m_array[i] = obj->clone() ;
   //FIXME?
      }
   return ;
}

//----------------------------------------------------------------------------

Array::~Array()
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

ObjectPtr Array::clone_(const Object *)
{

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Array::subseq_int(const Object *, size_t start, size_t stop)
{
   (void)start; (void)stop; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Array::subseq_iter(const Object *,ObjectIter start, ObjectIter stop)
{
   (void)start; (void)stop; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

size_t Array::cStringLength_(const Object *,size_t wrap_at, size_t indent)
{
   (void)wrap_at; (void)indent;//FIXME

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Array::toCstring_(const Object *,char *buffer, size_t buflen, size_t wrap_at, size_t indent)
{
   (void)buffer; (void)buflen; (void)wrap_at; (void)indent; //FIXME
   //FIXME
   return true ;
}

//----------------------------------------------------------------------------

size_t Array::hashValue_(const Object* obj)
{
   // the hash value of an array is the hash of the hash values of its elements
   size_t len = size_(obj) ;
   uint64_t hashstate = fasthash64_init(len) ;
   const Array* arr = reinterpret_cast<const Array*>(obj) ;
   for (size_t i = 0 ; i <  len ; ++i)
      {
      Object* elt = arr->m_array[i] ;
      if (elt) hashstate = fasthash64_add(hashstate,elt->hashValue()) ;
      }
   return (size_t)fasthash64_finalize(hashstate) ;
}

//----------------------------------------------------------------------------

bool Array::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int Array::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Array::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
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
template class Object_VMT<Fr::Array> ;

} // end namespace FramepaCC

// end of file array.C //
