/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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
   public: // types
      typedef Number super ;
   public:
      static Rational* create() ;
      static Rational* create(const Object*) ;
      static Rational* create(const Rational*) ;
      static Rational* create(const char*) ;

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Rational() ;
      Rational(long num, long denom) ;
      Rational(const Rational&) ;
      Rational(const Object* value) ;
      Rational(const char* value) ;
      ~Rational() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Rational> ;

      // type determination predicates
      static bool isRational_(const Object*) { return true ; }
      static const char *typeName_(const Object*) { return "Rational" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object* obj)
         { return new Rational(static_cast<const Rational&>(*obj)) ; }
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }
      using Number::subseq_int ;
      using Number::subseq_iter ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<Rational*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { delete static_cast<Rational*>(obj) ; }
      // (shallow and deep copies are the same for BigNums)

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*, char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard access functions ***
      static long int intValue(const Object*) ;
      static double floatValue(const Object*) ;
      static mpz_t bignumValue(const Object*) ;

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

   private: // static members
      static Allocator s_allocator ;
   protected:
      mpq_t m_value ;
} ;

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Rational> ;

} ; // end namespace FramepaC

#endif /* !__Fr_RATIONAL_H_INCLUDED */

// end of file rational.h //
