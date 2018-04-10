/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-10					*/
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
#include "framepac/vector.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class Vector					*/
/************************************************************************/

template <typename ValT>
Vector<ValT>::Vector(size_t capacity)
{
   reserve(capacity) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename ValT>
Vector<ValT>::Vector(const Vector&)
   : Object()
{
   //TODO
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
ObjectPtr Vector<ValT>::clone_(const Object*)
{
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr Vector<ValT>::subseq_int(const Object*, size_t /*start*/, size_t /*stop*/)
{
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr Vector<ValT>::subseq_iter(const Object*, ObjectIter /*start*/, ObjectIter /*stop*/)
{
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename ValT>
bool Vector<ValT>::equal_(const Object*, const Object*)
{
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename ValT>
int Vector<ValT>::compare_(const Object*, const Object*)
{
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename ValT>
int Vector<ValT>::lessThan_(const Object*, const Object*)
{
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename ValT>
bool Vector<ValT>::reserve(size_t N)
{
   if (N < m_capacity) return true ;  // nothing to do
   //TODO
   return false ;
}

//----------------------------------------------------------------------------

template <typename ValT>
size_t Vector<ValT>::cStringLength_(const Object* obj, size_t /*wrap_at*/,
   size_t indent, size_t /*wrapped_indent*/)
{
   auto v = static_cast<const Vector*>(obj) ;
   // format of printed rep is #<type:v1 v2 .... vN>
   size_t len = indent + 4 + strlen(v->typeName()) ;
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
   size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   auto v = static_cast<const Vector*>(obj) ;
   if (buflen < indent + 4 + strlen(v->typeName()))
      return buffer ;
   char* bufend = buffer + buflen ;
   buffer += snprintf(buffer,buflen,"%*s#<%s:",(int)indent,"",v->typeName()) ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      {
      if (i) *buffer++ = ' ' ;
      buffer = v->value_c_string(i,buffer,bufend-buffer) ;
      }
   *buffer++ = '>' ;
   *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

template <typename ValT>
size_t Vector<ValT>::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ;
   (void)wrap; (void)indent; //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

template <typename ValT>
bool Vector<ValT>::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj ;
   (void)buffer; (void)buflen; (void)indent ;
//TODO
   return false ;
}



} // end namespace Fr

// end of file vector.cc //
