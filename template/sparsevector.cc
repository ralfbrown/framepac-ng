/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-02					*/
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
#include "framepac/vector.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class SparseVector					*/
/************************************************************************/

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::add(const Vector<ValT>* other) const
{
   if (!other)
      return static_cast<SparseVector<IdxT,ValT>*>(&*this->clone()) ;
   size_t count { 0 } ;
   size_t elts1 { this->numElements() } ;
   size_t elts2 { other->numElements() } ;
   size_t pos1 { 0 } ;
   size_t pos2 { 0 } ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 { this->elementIndex(pos1) } ;
      auto elt2 { other->elementIndex(pos2) } ;
      if (elt1 <= elt2)
	 {
	 pos1++ ;
	 }
      if (elt1 >= elt2)
	 {
	 pos2++ ;
	 }
      count++ ;
      }
   count += (elts1 - pos1) + (elts2 - pos2) ;
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create(count) ;
   pos1 = 0 ;
   pos2 = 0 ;
   count = 0 ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 { (IdxT)(this->elementIndex(pos1)) } ;
      auto elt2 { (IdxT)(other->elementIndex(pos2)) } ;
      if (elt1 < elt2)
	 {
	 result->m_indices[count] = elt1 ;
	 result->m_values[count++] = this->elementValue(pos1++) ;
	 }
      else if (elt1 > elt2)
	 {
	 result->m_indices[count] = elt2 ;
	 result->m_values[count++] = other->elementValue(pos2++) ;
	 }
      else // elt1 == elt2
	 {
	 result->m_indices[count] = elt1 ;
	 result->m_values[count++] = this->elementValue(pos1++) + other->elementValue(pos2++) ;
	 }
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::add(const SparseVector<IdxT,ValT>* other) const
{
   if (!other)
      return static_cast<SparseVector<IdxT,ValT>*>(&*this->clone()) ;
   size_t count { 0 } ;
   size_t elts1 { this->numElements() } ;
   size_t elts2 { other->numElements() } ;
   size_t pos1 { 0 } ;
   size_t pos2 { 0 } ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 { this->elementIndex(pos1) } ;
      auto elt2 { other->elementIndex(pos2) } ;
      if (elt1 <= elt2)
	 {
	 pos1++ ;
	 }
      if (elt1 >= elt2)
	 {
	 pos2++ ;
	 }
      count++ ;
      }
   count += (elts1 - pos1) + (elts2 - pos2) ;
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create(count) ;
   pos1 = 0 ;
   pos2 = 0 ;
   count = 0 ;
   while (pos1 < elts1 && pos2 < elts2)
      {
      auto elt1 { (IdxT)(this->elementIndex(pos1)) } ;
      auto elt2 { (IdxT)(other->elementIndex(pos2)) } ;
      if (elt1 < elt2)
	 {
	 result->m_indices[count] = elt1 ;
	 result->m_values[count++] = this->elementValue(pos1++) ;
	 }
      else if (elt1 > elt2)
	 {
	 result->m_indices[count] = elt2 ;
	 result->m_values[count++] = other->elementValue(pos2++) ;
	 }
      else // elt1 == elt2
	 {
	 result->m_indices[count] = elt1 ;
	 result->m_values[count++] = this->elementValue(pos1++) + other->elementValue(pos2++) ;
	 }
      }
   while (pos1 < elts1)
      {
      result->m_indices[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values[count++] = this->elementValue(pos1++) ;
      }
   while (pos2 < elts2)
      {
      result->m_indices[count] = (IdxT)(other->elementIndex(pos2)) ;
      result->m_values[count++] = other->elementValue(pos2++) ;
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::add(const OneHotVector<IdxT,ValT>* other) const
{
   size_t count = this->numElements() ;
   
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create(count) ;
   auto elt2 { (IdxT)other->elementIndex(0) } ;
   count = 0 ;
   size_t elts1 { this->numElements() } ;
   size_t pos1 { 0 } ;
   while ((IdxT)this->elementIndex(pos1) < elt2)
      {
      result->m_indices[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values[count++] = this->elementValue(pos1++) ;
      }
   if ((IdxT)this->elementIndex(pos1) == elt2)
      {
      result->m_indices[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values[count++] = this->elementValue(pos1++) + other->elementValue(0) ;
      }
   else
      {
      result->m_indices[count] = (IdxT)(other->elementIndex(0)) ;
      result->m_values[count++] = other->elementValue(0) ;
      }
   while (pos1 < elts1)
      {
      result->m_indices[count] = (IdxT)(this->elementIndex(pos1)) ;
      result->m_values[count++] = this->elementValue(pos1++) ;
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t SparseVector<IdxT,ValT>::cStringLength_(const Object* obj, size_t /*wrap_at*/,
   size_t indent, size_t /*wrapped_indent*/)
{
   auto v = static_cast<const SparseVector*>(obj) ;
   // format of printed rep is #<type:i1:v1 i2:v2 .... iN:vN>
   size_t len = indent + 4 + strlen(v->typeName()) ;
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
   size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   auto v = static_cast<const SparseVector*>(obj) ;
   if (buflen < indent + 4 + strlen(v->typeName()))
      return buffer ;
   char* bufend = buffer + buflen ;
   buffer += snprintf(buffer,buflen,"%*s#<%s:",(int)indent,"",v->typeName()) ;
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
