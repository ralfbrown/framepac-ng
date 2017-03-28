/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
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

using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

// request explicit instantiation; we declared it "extern" in the header so this
//   will be the only copy of the non-inlined code generated in object modules
template class Allocator<Array> ;

static const FramepaC::Object_VMT<Array> array_vmt ;
Allocator<Array> Array::s_allocator(&array_vmt) ;

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
