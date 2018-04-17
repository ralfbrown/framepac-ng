/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-17					*/
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

#include "framepac/array.h"
#include "framepac/fasthash64.h"
#include "framepac/random.h"

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
	 Object *elt { orig->m_array[i] };
	 m_array[i] = elt ? elt->clone().move() : nullptr ;
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
   for (size_t i = 0 ; i < repeat ; i++)
      {
      m_array[i] = obj ? obj->clone().move() : nullptr ;
      }
   return ;
}

//----------------------------------------------------------------------------

Array::~Array()
{
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      Object* obj = m_array[i] ;
      if (obj) obj->free() ;
      }
   delete[] m_array ;
   m_size = 0 ;
   m_alloc = 0 ;
   return ;
}

//----------------------------------------------------------------------------

static Object* dummy_obj = nullptr ;

Object*& Array::operator [] (size_t N)
{
   return (N < size()) ? m_array[N] : dummy_obj ;
}

//----------------------------------------------------------------------------

void Array::setNth(size_t N, const Object* val)
{
   if (N < m_size)
      {
      if (m_array[N]) m_array[N]->free() ;
      m_array[N] = val ? val->clone().move() : nullptr ;
      }
   return ;
}

//----------------------------------------------------------------------------

bool Array::append(Object* obj)
{
   if (m_size >= m_alloc)
      {
      size_t newsize = m_alloc < 30 ? 30 : 3 * m_alloc / 2 ;
      if (!reserve(newsize))
	 return false ;
      }
   m_array[m_size++] = obj ? obj->clone().move() : nullptr ;
   return true ;
}

//----------------------------------------------------------------------------

bool Array::reserve(size_t N)
{
   if (N > capacity())
      {
      Object** new_arr = new Object*[N] ;
      if (new_arr)
	 {
	 for (size_t i = 0 ; i < size() ; ++i)
	    new_arr[i] = m_array[i] ;
	 for (size_t i = size() ; i < N ; ++i)
	    new_arr[i] = nullptr ;
	 delete[] m_array ;
	 m_array = new_arr ;
	 m_alloc = N ;
	 return true ;
	 }
      }
   return false ;
}

//----------------------------------------------------------------------------

void Array::resize(size_t N)
{
   Object** new_arr = new Object*[N] ;
   if (new_arr)
      {
      for (size_t i = 0 ; i < size() && i < N ; ++i)
	 new_arr[i] = m_array[i] ;
      for (size_t i = size() ; i < N ; ++i)
	 new_arr[i] = nullptr ;
      delete[] m_array ;
      m_array = new_arr ;
      m_size = std::min(size(),N) ;
      m_alloc = N ;
      }
   return ;
}

//----------------------------------------------------------------------------

void Array::resize(size_t N, Object*)
{
   (void)N;
   return ; //FIXME
}

//----------------------------------------------------------------------------

void Array::shrink_to_fit()
{
   if (size() < capacity())
      {
      Object** new_arr = new Object*[size()] ;
      if (new_arr)
	 {
	 for (size_t i = 0 ; i < size() ; ++i)
	    new_arr[i] = m_array[i] ;
	 m_alloc = size() ;
	 delete[] m_array ;
	 m_array = new_arr ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void Array::pop_back()
{
   if (m_size > 0)
      {
      --m_size ;
      if (m_array[m_size])
	 m_array[m_size]->free() ;
      }
   return ;
}

//----------------------------------------------------------------------------

ObjectPtr Array::clone_(const Object* obj)
{
   const Array* orig = static_cast<const Array*>(obj) ;
   return ObjectPtr(new Array(orig)) ;
}

//----------------------------------------------------------------------------

ObjectPtr Array::subseq_int(const Object *, size_t start, size_t stop)
{
   (void)start; (void)stop; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Array::subseq_iter(const Object*,ObjectIter start, ObjectIter stop)
{
   (void)start; (void)stop; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

size_t Array::cStringLength_(const Object* obj,size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   const Array* arr = static_cast<const Array*>(obj) ;
   size_t len = indent + 4 + (arr->size() ? arr->size()-1 : 0) ;
   bool wrapped { false } ;
   (void)wrapped_indent; //FIXME
   for (size_t i = 0 ; i < arr->size() ; ++i)
      {
      const Object* o = arr->at(i) ;
      if (o)
	 len += o->cStringLength(wrap_at,wrapped?indent+3:0,wrapped_indent) ;
      else
	 len += 4 ; // nullptr generates #N<> in the printed representation
      }
   return len ;
}

//----------------------------------------------------------------------------

char* Array::toCstring_(const Object* obj,char* buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   const Array* arr = static_cast<const Array*>(obj) ;
   size_t needed = snprintf(buffer,buflen,"%*s",(int)indent,"#A(") ;
   if (needed > buflen) return buffer ;
   const char* bufend = buffer + buflen ;
   buffer += needed ;
   bool wrapped { false } ;
   (void)wrapped_indent; //FIXME
   for (size_t i = 0 ; i < arr->size() ; ++i)
      {
      if (i) *buffer++ = ' ' ;
      const Object* o = arr->at(i) ;
      if (o)
	 buffer = o->toCstring(buffer,bufend - buffer,wrap_at,wrapped?indent+3:0,wrapped_indent) ;
      else if (buffer + 4 <= bufend)
	 {
	 // put in #N<> as a representation of nullptr
	 *buffer++ = '#' ;
	 *buffer++ = 'N' ;
	 *buffer++ = '<' ;
	 *buffer++ = '>' ;
	 }
      }
   *buffer++ = ')' ;
   *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t Array::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)wrap; //FIXME
   size_t len = indent + 2 ; // initial and trailing brackets
   size_t items = 0 ;
   for (const Object* o : *static_cast<const Array*>(obj))
      {
      if (o)
	 len += o->jsonStringLength(wrap,0) ;
      }
   return len + (items ? items-1 : 0) ;
}

//----------------------------------------------------------------------------

bool Array::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //FIXME
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

//----------------------------------------------------------------------------

RefArray* Array::randomSample(double sz) const
{
   if (sz <= 0)
      return RefArray::create() ;
   if (sz >= this->size())
      return RefArray::create(this) ;
   if (sz < 1)
      sz = (size_t)(sz * this->size() + 0.9) ;
   size_t to_sample = (size_t)sz ;
   RefArray* sample = RefArray::create(to_sample) ;
   bool* selected = RandomSample(this->size(),to_sample) ;
   size_t index = 0 ;
   for (size_t i = 0 ; i < size() ; ++i)
      {
      if (selected[i])
	 sample->setNth(index++,getNth(i)) ;
      }
   return sample ;
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
