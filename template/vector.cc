/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-08-27					*/
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

#include <cmath>
#include "framepac/fasthash64.h"
#include "framepac/vector.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class Vector					*/
/************************************************************************/

template <typename ValT>
Vector<ValT>::Vector(size_t cap)
{
   this->reserve(cap) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename ValT>
Vector<ValT>::Vector(const Vector& orig)
   : Vector(orig.size())
{
   std::copy(*orig.m_values,(*orig.m_values)+orig.size(),*this->m_values) ;
   this->m_size = orig.size() ;
   this->setKey(orig.key()) ;
   this->setLabel(orig.label()) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename ValT>
double Vector<ValT>::vectorLength() const
{
   this->m_critsect.lock() ;
   double len { 0.0 } ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      ValT elt = this->elementValue(i) ;
      len += (elt * elt) ;
      }
   len = std::sqrt(len) ;
   m_length = len ;
   this->m_critsect.unlock() ;
   return len ;
}

//----------------------------------------------------------------------------

template <typename ValT>
template <typename IdxT>
Vector<ValT>* Vector<ValT>::add(const Vector* other) const
{
   if (!other) return this->clone().move() ;
   if (this->isSparseVector())
      {
      if (other->isSparseVector())
	 {
	 return static_cast<const SparseVector<IdxT,ValT>*>(this)->add(static_cast<SparseVector<IdxT,ValT>*>(other)) ;
	 }
      else
	 {
	 return static_cast<const SparseVector<IdxT,ValT>*>(this)->add(static_cast<DenseVector<ValT>*>(other)) ;
	 }
      }
   else if (other->isSparseVector())
      {
      return static_cast<const SparseVector<IdxT,ValT>*>(other)->add(static_cast<DenseVector<ValT>*>(this)) ;
      }
   else
      {
      return static_cast<const DenseVector<ValT>*>(other)->add(static_cast<DenseVector<ValT>*>(this)) ;
      }
}

//----------------------------------------------------------------------------

template <typename ValT>
Vector<ValT>* Vector<ValT>::incr(const Vector* other)
{
   if (!other) return this ;
   if (this->isSparseVector())
      {
      //TODO: 
      }
   else if (other->isSparseVector())
      {
      //TODO:
      }
   else if (this->m_size == other->m_size)
      {
      // both vectors are dense and have the same dimensions, so we can just do an element-by-element addition
      for (size_t i = 0 ; i < this->m_size ; ++i)
	 {
	 m_values[i] += other->m_values[i] ;
	 }
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename ValT>
Vector<ValT>* Vector<ValT>::incr(const Vector* other, ValT weight)
{
   if (!other) return this ;
   if (this->isSparseVector())
      {
      //TODO: 
      }
   else if (other->isSparseVector())
      {
      //TODO:
      }
   else if (this->m_size == other->m_size)
      {
      // both vectors are dense and have the same dimensions, so we can just do an element-by-element addition
      for (size_t i = 0 ; i < this->m_size ; ++i)
	 {
	 m_values[i] += (weight * other->m_values[i]) ;
	 }
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename ValT>
void Vector<ValT>::scale(double factor)
{
   for (size_t i = 0 ; i < this->numElements() ; ++i)
      {
      m_values[i] *= factor ;
      }
   return  ;
}

//----------------------------------------------------------------------------

template <typename ValT>
void Vector<ValT>::normalize()
{
   double factor = this->vectorLength() ;
   if (factor <= 0.0) return ;
   for (size_t i = 0 ; i < this->numElements() ; ++i)
      {
      m_values[i] /= factor ;
      }
   return  ;
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr Vector<ValT>::clone_(const Object* obj)
{
   return obj ? new Vector<ValT>(*static_cast<const Vector*>(obj)) : nullptr ;
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr Vector<ValT>::subseq_int(const Object* obj, size_t start, size_t stop)
{
   if (!obj || start > stop)
      return nullptr ;
   //TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr Vector<ValT>::subseq_iter(const Object*, ObjectIter start, ObjectIter stop)
{
   return subseq_int(start.baseObject(),start.currentIndex(),stop.currentIndex()) ;
}

//----------------------------------------------------------------------------

template <typename ValT>
size_t Vector<ValT>::hashValue_(const Object* obj)
{
   auto tv = static_cast<const Vector<ValT>*>(obj) ;
   size_t numelts = tv->numElements() ;
   FastHash64 hash(numelts) ;
   for (size_t i = 0 ; i < numelts ; ++i)
      {
      hash += tv->elementValue(i) ;
      }
   return *hash ;
}

//----------------------------------------------------------------------------

template <typename ValT>
bool Vector<ValT>::equal_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2)
      return true ;			// identity implies equal values
   //TODO
   return false ;
}

//----------------------------------------------------------------------------

template <typename ValT>
int Vector<ValT>::compare_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2)
      return 0 ;			// identity implies equal values
   //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

template <typename ValT>
int Vector<ValT>::lessThan_(const Object* obj1, const Object* obj2)
{
   return compare_(obj1,obj2) <  0 ;
}

//----------------------------------------------------------------------------

template <typename ValT>
bool Vector<ValT>::reserve(size_t N)
{
   if (N < m_capacity) return true ;  // nothing to do
   auto new_values = new ValT[N] ;
   std::copy(*this->m_values,(*this->m_values)+this->size(),new_values) ;
   this->startModifying() ;
   this->m_values = new_values ;
   this->m_capacity = N ;
   this->doneModifying() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename ValT>
size_t Vector<ValT>::cStringLength_(const Object* obj, size_t wrap_at,
   size_t indent, size_t wrapped_indent)
{
   auto v = static_cast<const Vector*>(obj) ;
   // format of printed rep is #<type:weight:label:v1 v2 .... vN>
   size_t len = snprintf(nullptr,0,"%*s#<%s:%g:",(int)indent,"",v->typeName(),v->weight()) ;
   if (v->label()) len += v->label()->cStringLength(wrap_at,0,wrapped_indent) ;
   if (v->numElements() > 0)
      len += v->numElements() - 1 ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      {
      len += v->value_c_len(i) ;
      }
   return len  ;
}

//----------------------------------------------------------------------------

template <typename ValT>
char* Vector<ValT>::toCstring_(const Object* obj, char* buffer, size_t buflen,
   size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   auto v = static_cast<const Vector*>(obj) ;
   if (buflen < indent + 4 + strlen(v->typeName()))
      return buffer ;
   char* bufend = buffer + buflen ;
   int count = snprintf(buffer,buflen,"%*s#<%s:%g:",(int)indent,"",v->typeName(),v->weight()) ;
   buffer += count ;
   if (v->label())
      {
      buffer =  v->label()->toCstring(buffer,bufend-buffer,wrap_at,0,wrapped_indent) ;
      }
   if (buffer < bufend)
      *buffer++ = ':' ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      {
      if (i && buffer < bufend)
	 *buffer++ = ' ' ;
      buffer = v->value_c_string(i,buffer,bufend-buffer) ;
      }
   if (buffer < bufend)
      *buffer++ = '>' ;
   if (buffer < bufend)
      *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

template <typename ValT>
size_t Vector<ValT>::jsonStringLength_(const Object* obj, bool /*wrap*/, size_t indent)
{
   // for now, represent the vector as a JSON string whose value is the C string representation of the vector
   return 2 + cStringLength_(obj,~0,indent,indent) ;
}

//----------------------------------------------------------------------------

template <typename ValT>
bool Vector<ValT>::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   if (!buffer || buflen == 0)
      return false ;
   // for now, represent the vector as a JSON string whose value is the C string representation of the vector
   const char* bufend = buffer + buflen ;
   if (buflen > 0)
      {
      *buffer++ = '"' ;
      buflen-- ;
      }
   buffer = toCstring_(obj,buffer,buflen,~0,indent,indent) ;
   if (buffer < bufend)
      {
      *buffer++ = '"' ;
      return true ;
      }
   return false ;
}



} // end namespace Fr

// end of file vector.cc //
