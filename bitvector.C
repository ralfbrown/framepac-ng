/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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
#include "framepac/bitvector.h"
#include "framepac/number.h"
#include "framepac/fasthash64.h"

namespace Fr
{

/************************************************************************/
/*	Static variables for class BitVector				*/
/************************************************************************/

Allocator BitVector::s_allocator(FramepaC::Object_VMT<BitVector>::instance(),sizeof(BitVector)) ;

/************************************************************************/
/*	Methods for class BitVector					*/
/************************************************************************/

BitVector::BitVector(size_t /*capacity*/)
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

BitVector* BitVector::create(size_t /*capacity*/)
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

BitVector* BitVector::create(const char* bits)
{
   size_t capacity = bits ? strlen(bits) : 0 ;
   BitVector* bv = new BitVector(capacity) ;
   for (size_t i = 0 ; i < capacity ; ++i)
      {
      if (bits[i] == '1') bv->setBit(i,true) ;
      }
   return bv ;
}

//----------------------------------------------------------------------------

bool BitVector::getBit(size_t N) const
{
   if (N >= m_size) return false ;
   
   return false ; //FIXME
}

//----------------------------------------------------------------------------

void BitVector::setBit(size_t N, bool state)
{
   if (N < m_size)
      {
      (void)state ;
//FIXME
      }
   return;
}

//----------------------------------------------------------------------------

ObjectPtr BitVector::clone_(const Object*)
{

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

Object* BitVector::shallowCopy_(const Object*)
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

void BitVector::free_(Object* obj)
{
   (void)obj ; //FIXME
   return;
}

//----------------------------------------------------------------------------

void BitVector::shallowFree_(Object* obj)
{
   (void)obj ; //FIXME
   return;
}

//----------------------------------------------------------------------------

#if 0
BitVector* BitVector::subseq(...)
{

}
#endif

//----------------------------------------------------------------------------

ObjectPtr BitVector::subseq_int(const Object* obj, size_t start, size_t stop)
{
   (void)obj; (void)start; (void)stop ; //FIXME
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

ObjectPtr BitVector::subseq_iter(const Object* obj, ObjectIter start, ObjectIter stop)
{
   (void)obj; (void)start; (void)stop ; //FIXME
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

size_t BitVector::cStringLength_(const Object* obj, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   const BitVector* bv = static_cast<const BitVector*>(obj) ;
   return indent + 2 + bv->size() ;
}

//----------------------------------------------------------------------------

char* BitVector::toCstring_(const Object* obj, char* buffer, size_t buflen, size_t /*wrap_at*/, size_t indent,
   size_t /*wrapped_indent*/)
{
   const BitVector* bv = static_cast<const BitVector*>(obj) ;
   if (buflen < indent+3+bv->size()) return buffer ;
   buffer += snprintf(buffer,buflen,"%*s",(int)indent,"#*") ;
   buflen -= (indent + 2) ;
   for (size_t i = 0 ; i < bv->size() && buflen > 0 ; ++i)
      {
      *buffer++ = (bv->getBit(i) ? '1' : '0') ;
      --buflen ;
      }
   return buffer ;
}

//----------------------------------------------------------------------------

size_t BitVector::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ; (void)wrap; (void)indent ;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool BitVector::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

Object* BitVector::front_(Object* obj)
{
   bool bit = static_cast<BitVector*>(obj)->getBit(0) ;
   return Integer::create(bit ? 1 : (uint32_t)0) ;
}

//----------------------------------------------------------------------------

size_t BitVector::hashValue_(const Object* obj)
{
   const BitVector* bv = reinterpret_cast<const BitVector*>(obj) ;
   (void)bv ; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool BitVector::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int BitVector::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int BitVector::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

} ; // end of namespace Fr

// end of file 
