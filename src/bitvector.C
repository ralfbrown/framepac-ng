/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-08-27					*/
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
#include "framepac/bitvector.h"
#include "framepac/number.h"
#include "framepac/fasthash64.h"

namespace Fr
{

/************************************************************************/
/*	Static variables for class BitVector				*/
/************************************************************************/

Allocator BitVector::s_allocator(FramepaC::Object_VMT<BitVector>::instance(),sizeof(BitVector)) ;
const char BitVector::s_typename[] = "BitVector" ;

static constexpr size_t bits_per_sizet = (CHAR_BIT * sizeof(size_t)) ;

/************************************************************************/
/*	Methods for class BitVector					*/
/************************************************************************/

BitVector::BitVector(size_t cap)
{
   m_size = cap ;
   cap = (cap + bits_per_sizet - 1) / bits_per_sizet ;
   m_bits = new size_t[cap] ;
   if (m_bits)
      {
      m_capacity = cap * bits_per_sizet ;
      std::fill(m_bits,m_bits+cap,0) ;
      }
   else
      {
      m_size = m_capacity = 0 ;
      }
   return ;
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

BitVector::~BitVector()
{
   delete[] m_bits ;
   m_bits = nullptr ;
   m_size = 0 ;
   m_capacity = 0 ;
   return ;
}

//----------------------------------------------------------------------------

bool BitVector::getBit(size_t N) const
{
   if (N >= m_size) return false ;
   size_t idx = N / bits_per_sizet ;
   size_t bit = N % bits_per_sizet ;
   return (m_bits[idx] & (1UL<<bit)) != 0 ;
}

//----------------------------------------------------------------------------

void BitVector::setBit(size_t N, bool state)
{
   if (N < m_size)
      {
      size_t idx = N / bits_per_sizet ;
      size_t bit = N % bits_per_sizet ;
      if (state)
	 m_bits[idx] |= (1UL << bit) ;
      else
	 m_bits[idx] &= ~(1UL << bit) ;
      }
   return;
}

//----------------------------------------------------------------------------

ObjectPtr BitVector::clone_(const Object* obj)
{
   auto bv = static_cast<const BitVector*>(obj) ;
   size_t len = bv->size() ;
   auto copy = new BitVector(len) ;
   for (size_t i = 0 ; i < len ; ++i)
      {
      //TODO: make a more efficient version
      copy->setBit(i,bv->getBit(i)) ;
      }
   return ObjectPtr(copy) ;
}

//----------------------------------------------------------------------------

Object* BitVector::shallowCopy_(const Object* obj)
{
   return clone_(obj).move() ;
}

//----------------------------------------------------------------------------

void BitVector::free_(Object* obj)
{
   delete static_cast<BitVector*>(obj) ;
   return;
}

//----------------------------------------------------------------------------

void BitVector::shallowFree_(Object* obj)
{
   free_(obj) ;
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
   (void)obj; (void)start; (void)stop ; //TODO
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

ObjectPtr BitVector::subseq_iter(const Object* obj, ObjectIter start, ObjectIter stop)
{
   (void)obj; (void)start; (void)stop ; //TODO
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
   buffer += snprintf(buffer,buflen,"%*s%s",(int)indent,"","#*") ;
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
   return 0 ; //TODO
}

//----------------------------------------------------------------------------

bool BitVector::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //TODO
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
   FastHash64 hash(bv->size()) ;
   hash.add(bv->m_bits,bv->size()/8) ;
   return *hash ;
}

//----------------------------------------------------------------------------

bool BitVector::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;
   if (!other || !other->isBitVector())
      return false ;
   const BitVector* bv1 = static_cast<const BitVector*>(obj) ;
   const BitVector* bv2 = static_cast<const BitVector*>(other) ;
   if (bv1->size() != bv2->size())
      return false ;
   return memcmp(bv1->m_bits,bv2->m_bits,(bv1->size()+7)/8) == 0 ;
}

//----------------------------------------------------------------------------

int BitVector::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;
   if (!other || !other->isBitVector())
      return +1 ;
   const BitVector* bv1 = static_cast<const BitVector*>(obj) ;
   const BitVector* bv2 = static_cast<const BitVector*>(other) ;
   int cmp = memcmp(bv1->m_bits, bv2->m_bits, (std::min(bv1->size(),bv2->size())+7)/8) ;
   if (cmp)
      return cmp ;
   if (bv1->size() < bv2->size())
      return -1 ;
   else if (bv1->size() > bv2->size())
      return +1 ;
   return 0 ;
}

//----------------------------------------------------------------------------

int BitVector::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;
   if (!other || !other->isBitVector())
      return 1 ;
   const BitVector* bv1 = static_cast<const BitVector*>(obj) ;
   const BitVector* bv2 = static_cast<const BitVector*>(other) ;
   int cmp = memcmp(bv1->m_bits, bv2->m_bits, (std::min(bv1->size(),bv2->size())+7)/8) ;
   if (cmp < 1)
      return 1 ;
   if (cmp == 0 && obj->size() < other->size())
      return 1 ;
   return 0 ;
}

//----------------------------------------------------------------------------

} ; // end of namespace Fr

// end of file 
