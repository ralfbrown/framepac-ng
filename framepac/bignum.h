/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
/*	by Ralf Brown <ralf@cs.cmu.sedu>				*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
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

#ifndef __Fr_BIGNUM_H_INCLUDED
#define __Fr_BIGNUM_H_INCLUDED

#include "framepac/number.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

class BigNum : public Number
   {
   public:
      static BigNum *create() ;
      static BigNum *create(long) ;
      static BigNum *create(double) ;
      static BigNum *create(const BigNum *) ;
      static BigNum *create(const Object *) ;
      static BigNum *create(const char *) ;

   private: // static members
      static Allocator s_allocator ;
   private:
      mpz_t m_value ;
   protected: // creation/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      BigNum() ;
      BigNum(long value) ;
      BigNum(const BigNum &) ;
      BigNum(const Object *value) ;
      BigNum(const char *value) ;
      ~BigNum() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<BigNum> ;

      // type determination predicates
      static bool isBigNum_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "BigNum" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *obj)
         { return new BigNum(static_cast<const BigNum&>(*obj)) ; }
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      using Number::subseq_int ;
      using Number::subseq_iter ;

      // *** destroying ***
      static void free_(Object *obj) { delete (BigNum*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete (BigNum*)obj ; }
      // (shallow and deep copies are the same for BigNums)

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object *, char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard access functions ***
      static long int intValue(const Object *) ;
      static double floatValue(const Object *) ;
      static mpz_t bignumValue(const Object *) ;
   } ;

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::BigNum> ;

} ; // end namespace FramepaC

#endif /* !__Fr_BIGNUM_H_INCLUDED */

// end of file bignum.h //
