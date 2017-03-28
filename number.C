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

#include <cstring>
#include "framepac/bignum.h"
#include "framepac/rational.h"

namespace Fr
{

/************************************************************************/
/*	Helper functions						*/
/************************************************************************/

static bool is_rational(const char *numstring)
{
   return strchr(numstring,'/') != nullptr ;
}

//----------------------------------------------------------------------------

static bool is_bignum(const char *numstring)
{
   (void)numstring;
   return false ; //FIXME
}

/************************************************************************/
/*	Methods for class Number					*/
/************************************************************************/

Number *Number::create(const char *numstring)
{
   if (is_rational(numstring))
      {
      return Rational::create(numstring) ;
      // if we haven't linked in rational-number support, emulate it as a float
      // char *afterslash = strchr(buf,'/') ;
      // *afterslash++ = '\0' ;
      // Float numerator(buf) ;
      // Float denom(afterslash) ;
      // obj = new Float(numerator.floatValue() / denom.floatValue()) ;
      }
   else if (is_bignum(numstring))
      {
      // if we haven't linked in bignum support, truncate it to an integer
      //return new Integer(buf) ;
      return BigNum::create(numstring) ;
      }
   else
      return Integer::create(numstring) ;
}

//----------------------------------------------------------------------------

ObjectPtr Number::subseq_int(const Object *obj, size_t start, size_t stop)
{
   if (start != stop)
      return obj->clone() ;
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

ObjectPtr Number::subseq_iter(const Object *obj, ObjectIter start, ObjectIter stop)
{
   if (start != stop)
      return obj->clone() ;
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------

Object *Number::front_(Object *obj)
{
   return obj ;
}

//----------------------------------------------------------------------

const Object *Number::front_const(const Object *obj)
{
   return obj ;
}

//----------------------------------------------------------------------

#if 0
mpz_t Number::bignumValue_(const Object *)
{
   //FIXME
}
#endif

//----------------------------------------------------------------------

#if 0
mpq_t Number::rationalValue_(const Object *)
{
   //FIXME
}
#endif

//----------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Number>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Number> ;

} // end namespace FramepaC

// end of file number.C //
