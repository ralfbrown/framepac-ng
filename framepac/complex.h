/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-17					*/
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

#ifndef _Fr_COMPLEX_H_INCLUDED
#define _Fr_COMPLEX_H_INCLUDED

#include <complex>
#include "framepac/number.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

class Complex : public Number
   {
   public:
      typedef Number super ;
   public:
      // object factories
      static Complex* create() ;
      static Complex* create(long num, long denom) ;
      static Complex* create(double num, double denom) ;
      static Complex* create(const Complex&) ;
      static Complex* create(const Object*) ;
      static Complex* create(const char*) ;

   private: // static members
      static Allocator s_allocator ;
   private:
      std::complex<double> m_value ;
   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Complex() ;
      Complex(long num, long denom) ;
      Complex(const Complex&) ;
      Complex(const Object* value) ;
      Complex(const char* value) ;
      ~Complex() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Complex> ;

      // type determination predicates
      static bool isComplex_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "Complex" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object* obj)
         { return new Complex(static_cast<const Complex&>(*obj)) ; }
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }
      using Number::subseq_int ;
      using Number::subseq_iter ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<Complex*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { Complex::free_(obj) ; }
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
      static long int intValue_(const Object* obj) { return (long)((Complex*)obj)->m_value.real() ; }
      static double floatValue_(const Object* obj) { return ((Complex*)obj)->m_value.real() ; }
      static double imagValue_(const Object* obj) { return ((Complex*)obj)->m_value.imag() ; }
      static mpz_t bignumValue_(const Object*) ;

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

} ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* _Fr_COMPLEX_H_INCLUDED */

// end of file complex.h //
