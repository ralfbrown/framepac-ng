/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-16					*/
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

#include "framepac/complex.h"
#include "framepac/fasthash64.h"

using namespace FramepaC ;

namespace Fr
{

/************************************************************************/
/*	Methods for class Complex					*/
/************************************************************************/

Complex::Complex()
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

Complex* Complex::create()
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

#if 0
Complex* Complex::subseq(...)
{

}
#endif

//----------------------------------------------------------------------------

size_t Complex::cStringLength_(const Object*, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   size_t len = indent ;
   //FIXME
   return len ;
}

//----------------------------------------------------------------------------

char* Complex::toCstring_(const Object *, char *buffer, size_t buflen, size_t /*wrap_at*/, size_t indent,
   size_t /*wrapped_indent*/)
{
   if (buflen < indent) return buffer ;
   //FIXME
   return buffer ;
}

//----------------------------------------------------------------------------

size_t Complex::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ; (void)wrap; (void)indent ;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Complex::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

size_t Complex::hashValue_(const Object* obj)
{
   // the hash value of a complex number is the hash of the hashes of its two components
   const Complex* c = reinterpret_cast<const Complex*>(obj) ;
   uint64_t hashstate = fasthash64_init(2) ;
   hashstate = fasthash64_add(hashstate,fasthash64_float(c->m_value.real())) ;
   hashstate = fasthash64_add(hashstate,fasthash64_float(c->m_value.imag())) ;
   return fasthash64_finalize(hashstate) ;
}

//----------------------------------------------------------------------------

bool Complex::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int Complex::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Complex::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------



} // end namespace Fr

// end of file complex.C //
