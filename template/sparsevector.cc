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
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create() ;

   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* SparseVector<IdxT,ValT>::add(const SparseVector<IdxT,ValT>* other) const
{
   if (!other)
      return static_cast<SparseVector<IdxT,ValT>*>(&*this->clone()) ;
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create() ;

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
