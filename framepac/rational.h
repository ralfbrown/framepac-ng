/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
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

#ifndef __Fr_RATIONAL_H_INCLUDED
#define __Fr_RATIONAL_H_INCLUDED

#include "framepac/number.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

class Rational : public Number
   {
   public:
      static Rational *create() ;
      static Rational *create(const Object *) ;
      static Rational *create(const Rational *) ;
      static Rational *create(const char *) ;

   private: // static members
      static Allocator<Rational> s_allocator ;
   private:
      mpq_t m_value ;
   protected: // creation/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      Rational() ;
      Rational(long num, long denom) ;
      Rational(const Rational &) ;
      Rational(const Object *value) ;
      Rational(const char *value) ;
      ~Rational() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Rational> ;

      // type determination predicates
      static bool isRational_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "Rational" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *obj)
         { return new Rational(static_cast<const Rational&>(*obj)) ; }
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      using Number::subseq_int ;
      using Number::subseq_iter ;

      // *** destroying ***
      static void free_(Object *obj) { delete (Rational*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete (Rational*)obj ; }
      // (shallow and deep copies are the same for BigNums)

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object *, char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard access functions ***
      static long int intValue(const Object *) ;
      static double floatValue(const Object *) ;
      static mpz_t bignumValue(const Object *) ;
   } ;

//----------------------------------------------------------------------------

extern template class Allocator<Rational> ;

// end of namespace Fr
} ;

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Rational> ;

} ; // end namespace FramepaC

#endif /* !__Fr_RATIONAL_H_INCLUDED */

// end of file rational.h //
