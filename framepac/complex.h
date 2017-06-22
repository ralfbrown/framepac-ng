/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
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

#ifndef _Fr_COMPLEX_H_INCLUDED
#define _Fr_COMPLEX_H_INCLUDED

#include "framepac/number.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

class Complex : public Number
   {
   public:


   private: // static members
      static Allocator s_allocator ;
   private:
      double m_real ;
      double m_imag ;
   protected: // creation/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
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
      static void free_(Object* obj) { delete (Complex*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { delete (Complex*)obj ; }
      // (shallow and deep copies are the same for BigNums)

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object*, char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard access functions ***
      static long int intValue_(const Object* obj) { return (long)((Complex*)obj)->m_real ; }
      static double floatValue_(const Object* obj) { return ((Complex*)obj)->m_real ; }
      static double imagValue_(const Object* obj) { return ((Complex*)obj)->m_imag ; }
      static mpz_t bignumValue_(const Object*) ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* _Fr_COMPLEX_H_INCLUDED */

// end of file complex.h //
