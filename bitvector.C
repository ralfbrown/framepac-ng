/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-08					*/
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

#include "framepac/bitvector.h"
#include "framepac/fasthash64.h"

namespace Fr
{

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

size_t BitVector::cStringLength_(const Object*, size_t wrap_at, size_t indent)
{
   (void)wrap_at; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool BitVector::toCstring_(const Object *, char *buffer, size_t buflen, size_t wrap_at, size_t indent)
{
   (void)buffer; (void)buflen; (void)wrap_at; (void)indent; //FIXME
   //FIXME
   return true ;
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
