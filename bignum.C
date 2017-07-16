/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-15					*/
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

#include "framepac/bignum.h"

using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

Allocator BigNum::s_allocator(FramepaC::Object_VMT<BigNum>::instance(),sizeof(BigNum)) ;

/************************************************************************/
/************************************************************************/

BigNum::BigNum()
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

BigNum::BigNum(const char *value)
{
   //FIXME
   (void)value ;
   return ;
}

//----------------------------------------------------------------------------

BigNum::BigNum(const BigNum&)
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

BigNum* BigNum::create()
{
   return new BigNum ;
}

//----------------------------------------------------------------------------

BigNum* BigNum::create(const Object*)
{
   return nullptr ;  //FIXME
}

//----------------------------------------------------------------------------

BigNum* BigNum::create(const BigNum*)
{
   return nullptr ;  //FIXME
}

//----------------------------------------------------------------------------

BigNum* BigNum::create(const char*)
{
   return nullptr ;  //FIXME
}

//----------------------------------------------------------------------------

BigNum::~BigNum()
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

size_t BigNum::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   size_t len = indent ;
#ifdef FRAMEPAC_GPL
   //TODO: using libgmp
#else
   mpz_t val = static_cast<const BigNum*>(obj)->m_value ;
   if (val == 0) return len+1 ;
   while (val > 0)
      {
      val /= 10 ;
      len++ ;
      }
#endif
   return len ;
}

//----------------------------------------------------------------------------

char* BigNum::toCstring_(const Object *obj, char *buffer, size_t buflen,
   size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   mpz_t val = static_cast<const BigNum*>(obj)->m_value ;
   if (buflen < indent + 1 + (val < 0)) return buffer ;
   char* bufend = buffer + buflen ;
   for (size_t i = 0 ; i < indent ; ++i)
      {
      *buffer++ = ' ' ;
      }
   if (val < 0)
      {
      *buffer++ = '-' ;
      val = -val ;
      }
   char* buf = buffer ;
   if (val == 0)
      *buffer++ = '0' ;
   else
      {
      while (val > 0 && buf < bufend)
	 {
	 *buf++ = '0' + (val % 10) ;
	 val /= 10 ;
	 }
      //FIXME: reverse the digits
      
      }
   *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t BigNum::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ; (void)wrap; (void)indent ;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool BigNum::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			   bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<BigNum>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::BigNum> ;

} // end namespace FramepaC

// end of file bignum.C //
