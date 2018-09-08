/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.11, last edit 2018-09-06					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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

#include <cstring>
#include "template/bufbuilder.cc"
#include "framepac/fasthash64.h"
#include "framepac/vector.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class SparseVector					*/
/************************************************************************/

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>::SparseVector(size_t cap)
   : Vector<IdxT,ValT>(0)
{
   this->m_indices.full = nullptr ;
   this->reserve(cap) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>::SparseVector(const char* rep)
{
   BufferBuilder<IdxT,16> indices ;
   BufferBuilder<ValT,16> values ;
   while (indices.read(rep))
      {
      if (*rep != ':')
	 break ;
      ++rep ;
      if (!values.read(rep))
	 break ;
      }
   this->m_size = values.size() ;
   this->m_capacity = values.capacity() ;
   this->m_values.full = values.move() ;
   this->m_indices.full = indices.move() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>::SparseVector(const SparseVector& orig)
   : Vector<IdxT,ValT>(orig)
{
   this->m_indices.full = new IdxT[this->capacity()] ;
   std::copy(orig.m_indices.full,orig.m_indices.full+orig.size(),this->m_indices.full) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t SparseVector<IdxT,ValT>::totalElements(const SparseVector<IdxT,ValT>* v1,
   const SparseVector<IdxT,ValT>* v2)
{
   size_t total { 0 } ;
   size_t elts1 { v1->numElements() } ;
   size_t elts2 { v2->numElements() } ;
   size_t pos1 { 0 } ;
   size_t pos2 { 0 } ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 = v1->elementIndex(pos1) ;
      auto elt2 = v2->elementIndex(pos2) ;
      if (elt1 <= elt2)
	 {
	 pos1++ ;
	 }
      if (elt1 >= elt2)
	 {
	 pos2++ ;
	 }
      total++ ;
      }
   total += (elts1 - pos1) + (elts2 - pos2) ;
   return total ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t SparseVector<IdxT,ValT>::totalElements(const SparseVector<IdxT,ValT>* v1,
   const Vector<IdxT,ValT>* v2)
{
   if (v2->isSparseVector())
      return totalElements(v1,static_cast<const SparseVector<IdxT,ValT>*>(v2)) ;
   size_t total { 0 } ;
   size_t elts1 { v1->numElements() } ;
   size_t elts2 { v2->numElements() } ;
   size_t pos1 { 0 } ;
   size_t pos2 { 0 } ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 = v1->elementIndex(pos1) ;
      auto elt2 = v2->elementIndex(pos2) ;
      if (elt1 <= elt2)
	 {
	 pos1++ ;
	 }
      if (elt1 >= elt2)
	 {
	 pos2++ ;
	 }
      total++ ;
      }
   total += (elts1 - pos1) + (elts2 - pos2) ;
   return total ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool SparseVector<IdxT,ValT>::newElement(IdxT index, ValT value)
{
   // perform a binary search for the requested index
   size_t lo = 0 ;
   size_t hi = this->size() ;
   while (hi > lo)
      {
      size_t mid = lo + (hi-lo)/2 ;
      IdxT mid_idx = (IdxT)this->elementIndex(mid) ;
      if (mid_idx == index)
	 return false ;			// failed, the element is already present
      else if (mid_idx < index)
	 lo = mid+1 ;
      else // if (mid_idx > index)
	 hi = mid ;
      }
   // if we didn't find the element, shift the following elements to
   //   make a gap, then put the new element in the gap
   // first, expand the vector's backing store if necessary
   if (this->size() >= this->capacity() && !this->reserve(std::max(14UL,3*this->capacity()/2)))
      {
      return false ;			// unable to insert the element because we couldn't expand the vector
      }
   for (size_t i = this->size() ; i > lo ; --i)
      {
      this->m_indices.full[i] = this->m_indices.full[i-1] ;
      this->m_values.full[i] = this->m_values.full[i-1] ;
      }
   this->m_indices.full[lo] = index ;
   this->m_values.full[lo] = value ;
   this->m_size++ ;
   return true ;			// we successfully added the element
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::add(const Vector<IdxT,ValT>* other) const
{
   if (!other)
      return static_cast<SparseVector<IdxT,ValT>*>(&*this->clone().move()) ;
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create(totalElements(this,other)) ;
   size_t count { 0 } ;
   size_t elts1 { this->numElements() } ;
   size_t elts2 { other->numElements() } ;
   size_t pos1 { 0 } ;
   size_t pos2 { 0 } ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 = (IdxT)(this->elementIndex(pos1)) ;
      auto elt2 = (IdxT)(other->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 result->m_indices.full[count] = elt1 ;
	 result->m_values.full[count++] = this->elementValue(pos1++) ;
	 }
      else if (elt1 > elt2)
	 {
	 result->m_indices.full[count] = elt2 ;
	 result->m_values.full[count++] = other->elementValue(pos2++) ;
	 }
      else // elt1 == elt2
	 {
	 result->m_indices.full[count] = elt1 ;
	 result->m_values.full[count++] = this->elementValue(pos1++) + other->elementValue(pos2++) ;
	 }
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::add(const SparseVector<IdxT,ValT>* other) const
{
   if (!other)
      return static_cast<SparseVector<IdxT,ValT>*>(&*this->clone().move()) ;
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create(totalElements(this,other)) ;
   size_t count { 0 } ;
   size_t elts1 { this->numElements() } ;
   size_t elts2 { other->numElements() } ;
   size_t pos1 { 0 } ;
   size_t pos2 { 0 } ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 = (IdxT)(this->elementIndex(pos1)) ;
      auto elt2 = (IdxT)(other->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 result->m_indices.full[count] = elt1 ;
	 result->m_values.full[count++] = this->elementValue(pos1++) ;
	 }
      else if (elt1 > elt2)
	 {
	 result->m_indices.full[count] = elt2 ;
	 result->m_values.full[count++] = other->elementValue(pos2++) ;
	 }
      else // elt1 == elt2
	 {
	 result->m_indices.full[count] = elt1 ;
	 result->m_values.full[count++] = this->elementValue(pos1++) + other->elementValue(pos2++) ;
	 }
      }
   while (pos1 < elts1)
      {
      result->m_indices.full[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values.full[count++] = this->elementValue(pos1++) ;
      }
   while (pos2 < elts2)
      {
      result->m_indices.full[count] = (IdxT)(other->elementIndex(pos2)) ;
      result->m_values.full[count++] = other->elementValue(pos2++) ;
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::add(const OneHotVector<IdxT,ValT>* other) const
{
   size_t count = this->numElements() ;
   
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create(count) ;
   auto elt2 = (IdxT)other->elementIndex(0) ;
   count = 0 ;
   size_t elts1 { this->numElements() } ;
   size_t pos1 { 0 } ;
   while ((IdxT)this->elementIndex(pos1) < elt2)
      {
      result->m_indices.full[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values.full[count++] = this->elementValue(pos1++) ;
      }
   if ((IdxT)this->elementIndex(pos1) == elt2)
      {
      result->m_indices.full[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values.full[count++] = this->elementValue(pos1++) + other->elementValue(0) ;
      }
   else
      {
      result->m_indices.full[count] = (IdxT)(other->elementIndex(0)) ;
      result->m_values.full[count++] = other->elementValue(0) ;
      }
   while (pos1 < elts1)
      {
      result->m_indices.full[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values.full[count++] = this->elementValue(pos1++) ;
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::incr(const Vector<IdxT,ValT>* other, ValT wt)
{
   if (!other)
      return static_cast<SparseVector<IdxT,ValT>*>(&*this->clone().move()) ;
   size_t new_size { totalElements(this,other) } ;
   IdxT* new_indices { new IdxT[new_size] } ;
   ValT* new_values { new ValT[new_size] } ;
   size_t elts1 { this->numElements() } ;
   size_t elts2 { other->numElements() } ;
   size_t pos1 { 0 } ;
   size_t pos2 { 0 } ;
   size_t count { 0 } ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 = (IdxT)(this->elementIndex(pos1)) ;
      auto elt2 = (IdxT)(other->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 new_indices[count] = elt1 ;
	 new_values[count++] = this->elementValue(pos1++) ;
	 }
      else if (elt1 > elt2)
	 {
	 new_indices[count] = elt2 ;
	 new_values[count++] = wt*other->elementValue(pos2++) ;
	 }
      else // elt1 == elt2
	 {
	 new_indices[count] = elt1 ;
	 new_values[count++] = this->elementValue(pos1++) + wt*other->elementValue(pos2++) ;
	 }
      }
   while (pos1 < elts1)
      {
      new_indices[count] = (IdxT)(this->elementIndex(pos1)) ;
      new_values[count++] = this->elementValue(pos1++) ;
      }
   while (pos2 < elts2)
      {
      new_indices[count] = (IdxT)(other->elementIndex(pos2)) ;
      new_values[count++] = wt*other->elementValue(pos2++) ;
      }
   this->startModifying() ;
   this->m_size = new_size ;
   this->m_capacity = new_size ;
   this->updateContents(new_indices,new_values) ;
   this->doneModifying() ;
   return this ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr SparseVector<IdxT,ValT>::clone_(const Object* obj)
{
   return obj ? new SparseVector<IdxT,ValT>(*static_cast<const SparseVector*>(obj)) : nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr SparseVector<IdxT,ValT>::subseq_int(const Object* obj, size_t start, size_t stop)
{
   if (!obj || start > stop)
      return nullptr ;
   //TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr SparseVector<IdxT,ValT>::subseq_iter(const Object*, ObjectIter start, ObjectIter stop)
{
   return subseq_int(start.baseObject(),start.currentIndex(),stop.currentIndex()) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t SparseVector<IdxT,ValT>::hashValue_(const Object* obj)
{
   auto tv = static_cast<const SparseVector<IdxT,ValT>*>(obj) ;
   size_t numelts = tv->numElements() ;
   FastHash64 hash(numelts) ;
   for (size_t i = 0 ; i < numelts ; ++i)
      {
      hash += tv->elementIndex(i) ;
      hash += tv->elementValue(i) ;
      }
   return *hash ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool SparseVector<IdxT,ValT>::equal_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2) return true ;	// if same object, contents must be the same, too
   if (!obj2 || !obj2->isSparseVector())
      return false ;			// can currently only compare sparse vector with another sparse vector
   auto v1 = static_cast<const SparseVector<IdxT,ValT>*>(obj1) ;
   auto v2 = static_cast<const SparseVector<IdxT,ValT>*>(obj2) ;
   if (v1->size() != v2->size())
      return false ;
   //TODO
   return false ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
int SparseVector<IdxT,ValT>::lessThan_(const Object* obj1, const Object* obj2)
{
   return compare_(obj1,obj2) < 0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
int SparseVector<IdxT,ValT>::compare_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2)
      return 0 ;			// identical objects implies equal values
   //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
void SparseVector<IdxT,ValT>::updateContents(IdxT* indices, ValT* values)
{
   delete[] this->m_indices.full ;
   this->m_indices.full = indices ;
   delete[] this->m_values.full ;
   this->m_values.full = values ;
   return  ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool SparseVector<IdxT,ValT>::reserve(size_t N)
{
   if (N < this->m_capacity) return true ;  // nothing to do
   auto new_indices = new IdxT[N] ;
   auto new_values = new ValT[N] ;
   for (size_t i = 0 ; i < this->size() ; ++i)
      {
      new_indices[i] = this->m_indices.full[i] ;
      new_values[i] = this->m_values.full[i] ;
      }
   this->startModifying() ;
   this->updateContents(new_indices,new_values) ;
   this->m_capacity = N ;
   this->doneModifying() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t SparseVector<IdxT,ValT>::cStringLength_(const Object* obj, size_t wrap_at,
   size_t indent, size_t wrapped_indent)
{
   auto v = static_cast<const SparseVector*>(obj) ;
   // format of printed rep is #<type:key:label:i1:v1 i2:v2 .... iN:vN>
   size_t len = indent + 6 + strlen(v->typeName()) + v->numElements() ;
   if (v->key())
      {
      len += v->key()->cStringLength(wrap_at,0,wrapped_indent) ;
      }
   if (v->label())
      {
      len += v->label()->cStringLength(wrap_at,0,wrapped_indent) ;
      }
   if (v->numElements() > 0)
      len += v->numElements() - 1 ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      {
      len += v->index_c_len(i) ;
      len += v->value_c_len(i) ;
      }
   return len  ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
char* SparseVector<IdxT,ValT>::toCstring_(const Object* obj, char* buffer, size_t buflen,
   size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   auto v = static_cast<const SparseVector*>(obj) ;
   if (buflen < indent + 5 + strlen(v->typeName()))
      return buffer ;
   char* bufend = buffer + buflen ;
   buffer += snprintf(buffer,buflen,"%*s#<%s:",(int)indent,"",v->typeName()) ;
   if (v->key())
      {
      buffer = v->key()->toCstring(buffer,bufend-buffer,wrap_at,0,wrapped_indent) ;
      }
   if (buffer < bufend)
      {
      *buffer++ = ':' ;
      }
   if (v->label())
      {
      buffer = v->label()->toCstring(buffer,bufend-buffer,wrap_at,0,wrapped_indent) ;
      }
   if (buffer < bufend)
      {
      *buffer++ = ':' ;
      }
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      {
      if (i) *buffer++ = ' ' ;
      buffer = v->index_c_string(i,buffer,bufend-buffer) ;
      *buffer++ = ':' ;
      buffer = v->value_c_string(i,buffer,bufend-buffer) ;
      }
   *buffer++ = '>' ;
   *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t SparseVector<IdxT,ValT>::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ;
   (void)wrap; (void)indent; //FIXME
   return 0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool SparseVector<IdxT,ValT>::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj ;
   (void)buffer; (void)buflen; (void)indent ;
//FIXME
   return false ;
}

//----------------------------------------------------------------------------


} // end namespace Fr

// end of file sparsevector.cc //
