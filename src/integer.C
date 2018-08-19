/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-18					*/
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

#include <stdlib.h>
#include "framepac/number.h"
#include "framepac/fasthash64.h"

using namespace FramepaC ;
using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

Allocator Integer::s_allocator(FramepaC::Object_VMT<Integer>::instance(),sizeof(Integer)) ;
const char Integer::s_typename[] = "Integer" ;

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

Integer::Integer(const char *value, unsigned radix)
   : m_value(value ? strtol(value,nullptr,radix) : 0)
{
   return ;
}

//----------------------------------------------------------------------------

size_t Integer::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   return indent + snprintf(nullptr,0,"%ld",obj->intValue()) ;
}

//----------------------------------------------------------------------------

char* Integer::toCstring_(const Object *obj, char *buffer, size_t buflen,
   size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   size_t needed = snprintf(buffer,buflen,"%*s%ld",(int)indent,"",obj->intValue()) ;
   return (needed <= buflen) ? buffer + needed : buffer ;
}

//----------------------------------------------------------------------------

size_t Integer::jsonStringLength_(const Object* obj, bool /*wrap*/, size_t indent)
{
   return cStringLength_(obj,~0,indent,indent) ;
}

//----------------------------------------------------------------------------

bool Integer::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			    bool /*wrap*/, size_t indent)
{
   return toCstring_(obj,buffer,buflen,~0,indent,indent) != buffer ;
}

//----------------------------------------------------------------------------

Object* Integer::front_(Object* obj)
{
   return obj ;
}

//----------------------------------------------------------------------------

const Object* Integer::front_const(const Object* obj)
{
   return obj ;
}

//----------------------------------------------------------------------------

size_t Integer::hashValue_(const Object* obj)
{
   const Integer* integer = static_cast<const Integer*>(obj) ;
   return fasthash64_int(integer->m_value) ;
}

//----------------------------------------------------------------------------

bool Integer::equal_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2) return true ;
   if (!obj1 || !obj2) return false ;
   return obj1->intValue() == obj2->intValue() ;
}

//----------------------------------------------------------------------------

int Integer::compare_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2) return 0 ;		// identical object, so value must be the  same
   if (!obj1) return  1 ;
   if (!obj2 || !obj2->isNumber()) return +1 ;
   auto val1 = obj1->intValue() ;
   auto val2 = obj2->intValue() ;
   if (val1 < val2) return -1 ;
   else if (val1 > val2) return +1 ;
   return 0 ;				// values are the same
}

//----------------------------------------------------------------------------

int Integer::lessThan_(const Object* obj1, const Object* obj2)
{
   return compare_(obj1,obj2) < 0 ;
}

/************************************************************************/
/************************************************************************/

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Integer>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Integer> ;

} // end namespace FramepaCC

// end of file integer.C //
