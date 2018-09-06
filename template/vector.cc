/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.11, last edit 2018-09-06					*/
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

template <typename IdxT, typename ValT>
Vector<IdxT,ValT>::Vector(size_t cap)
{
   this->reserve(cap) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
Vector<IdxT,ValT>::Vector(const Vector& orig)
   : Vector(orig.size())
{
   std::copy(*orig.m_values.full,(*orig.m_values.full)+orig.size(),*this->m_values.full) ;
   this->m_size = orig.size() ;
   this->setKey(orig.key()) ;
   this->setLabel(orig.label()) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double Vector<IdxT,ValT>::vectorLength() const
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

template <typename IdxT, typename ValT>
Vector<IdxT,ValT>* Vector<IdxT,ValT>::add(const Vector* other) const
{
   if (!other) return static_cast<Vector*>(this->clone().move()) ;
   if (this->isSparseVector())
      {
      if (other->isSparseVector())
	 {
	 return static_cast<const sparse_type*>(this)->add(static_cast<const sparse_type*>(other)) ;
	 }
      else
	 {
	 return static_cast<const sparse_type*>(this)->add(static_cast<const dense_type*>(other)) ;
	 }
      }
   else if (other->isSparseVector())
      {
      return static_cast<const sparse_type*>(other)->add(static_cast<const dense_type*>(this)) ;
      }
   else
      {
      return static_cast<const dense_type*>(other)->add(static_cast<const dense_type*>(this)) ;
      }
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
Vector<IdxT,ValT>* Vector<IdxT,ValT>::incr(const Vector* other)
{
   if (!other) return this ;
   if (this->isSparseVector())
      {
      auto sv1 = static_cast<SparseVector<IdxT,ValT>*>(this) ;
      return sv1->incr(other) ;
      }
   else if (other->isSparseVector())
      {
      for (size_t i = 0 ; i < other->numElements() ; ++i)
	 {
	 auto sv = static_cast<const SparseVector<IdxT,ValT>*>(other) ;
	 auto indx = sv->elementIndex(i) ;
	 if (indx < this->m_size)
	    {
	    this->m_values.full[indx] += sv->elementValue(i) ;
	    }
	 }
      }
   else if (this->m_size == other->m_size)
      {
      // both vectors are dense and have the same dimensions, so we can just do an element-by-element addition
      for (size_t i = 0 ; i < this->m_size ; ++i)
	 {
	 this->m_values.full[i] += other->m_values.full[i] ;
	 }
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
Vector<IdxT,ValT>* Vector<IdxT,ValT>::incr(const Vector* other, ValT weight)
{
   if (!other) return this ;
   if (this->isSparseVector())
      {
      auto sv1 = static_cast<SparseVector<IdxT,ValT>*>(this) ;
      return sv1->incr(other,weight) ;
      }
   else if (other->isSparseVector())
      {
      for (size_t i = 0 ; i < other->numElements() ; ++i)
	 {
	 auto sv = static_cast<const SparseVector<IdxT,ValT>*>(other) ;
	 auto indx = sv->elementIndex(i) ;
	 if (indx < this->m_size)
	    {
	    this->m_values.full[indx] += (weight * sv->elementValue(i)) ;
	    }
	 }
      }
   else if (this->m_size == other->m_size)
      {
      // both vectors are dense and have the same dimensions, so we can just do an element-by-element addition
      for (size_t i = 0 ; i < this->m_size ; ++i)
	 {
	 this->m_values.full[i] += (weight * other->m_values.full[i]) ;
	 }
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
void Vector<IdxT,ValT>::scale(double factor)
{
   for (size_t i = 0 ; i < this->numElements() ; ++i)
      {
      this->m_values.full[i] *= factor ;
      }
   return  ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
void Vector<IdxT,ValT>::normalize()
{
   double factor = this->vectorLength() ;
   if (factor <= 0.0) return ;
   for (size_t i = 0 ; i < this->numElements() ; ++i)
      {
      this->m_values.full[i] /= factor ;
      }
   return  ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr Vector<IdxT,ValT>::clone_(const Object* obj)
{
   return obj ? new Vector<IdxT,ValT>(*static_cast<const Vector*>(obj)) : nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr Vector<IdxT,ValT>::subseq_int(const Object* obj, size_t start, size_t stop)
{
   if (!obj || start > stop)
      return nullptr ;
   //TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr Vector<IdxT,ValT>::subseq_iter(const Object*, ObjectIter start, ObjectIter stop)
{
   return subseq_int(start.baseObject(),start.currentIndex(),stop.currentIndex()) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t Vector<IdxT,ValT>::hashValue_(const Object* obj)
{
   auto tv = static_cast<const Vector<IdxT,ValT>*>(obj) ;
   size_t numelts = tv->numElements() ;
   FastHash64 hash(numelts) ;
   for (size_t i = 0 ; i < numelts ; ++i)
      {
      hash += tv->elementValue(i) ;
      }
   return *hash ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool Vector<IdxT,ValT>::equal_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2)
      return true ;			// identity implies equal values
   //TODO
   return false ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
int Vector<IdxT,ValT>::compare_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2)
      return 0 ;			// identity implies equal values
   //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
int Vector<IdxT,ValT>::lessThan_(const Object* obj1, const Object* obj2)
{
   return compare_(obj1,obj2) <  0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool Vector<IdxT,ValT>::reserve(size_t N)
{
   if (N < m_capacity) return true ;  // nothing to do
   auto new_values = new ValT[N] ;
   std::copy(*this->m_values.full,(*this->m_values.full)+this->size(),new_values) ;
   this->startModifying() ;
   this->m_values.full = new_values ;
   this->m_capacity = N ;
   this->doneModifying() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t Vector<IdxT,ValT>::cStringLength_(const Object* obj, size_t wrap_at,
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

template <typename IdxT, typename ValT>
char* Vector<IdxT,ValT>::toCstring_(const Object* obj, char* buffer, size_t buflen,
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

template <typename IdxT, typename ValT>
size_t Vector<IdxT,ValT>::jsonStringLength_(const Object* obj, bool /*wrap*/, size_t indent)
{
   // for now, represent the vector as a JSON string whose value is the C string representation of the vector
   return 2 + cStringLength_(obj,~0,indent,indent) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool Vector<IdxT,ValT>::toJSONString_(const Object* obj, char* buffer, size_t buflen,
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
