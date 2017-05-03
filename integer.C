/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-02					*/
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

#include <stdlib.h>
#include "framepac/number.h"

using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

// request explicit instantiation; we declared it "extern" in the header so this
//   will be the only copy of the non-inlined code generated in object modules
template class Allocator<Integer> ;

static const FramepaC::Object_VMT<Integer> integer_vmt ;
Allocator<Integer> Integer::s_allocator(&integer_vmt) ;

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

Integer::Integer(const char *value, unsigned radix)
   : m_value(value ? strtol(value,nullptr,radix) : 0)
{
   return ;
}

//----------------------------------------------------------------------------

size_t Integer::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent)
{
   return snprintf(nullptr,0,"%ld",obj->intValue() + indent) ;
}

//----------------------------------------------------------------------------

bool Integer::toCstring_(const Object *obj, char *buffer, size_t buflen,
			 size_t /*wrap_at*/, size_t indent)
{
   size_t needed = snprintf(buffer,buflen,"%*s%ld",(int)indent,"",obj->intValue()) ;
   return needed <= buflen ;
}

//----------------------------------------------------------------------------

size_t Integer::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ; (void)wrap; (void)indent ;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Integer::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			    bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //FIXME
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
   const Integer* integer = reinterpret_cast<const Integer*>(obj) ;
//FIXME
   return integer->m_value ;
}

//----------------------------------------------------------------------------

bool Integer::equal_(const Object* obj1, const Object* obj2)
{
   (void)obj1; (void)obj2;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

int Integer::compare_(const Object* obj1, const Object* obj2)
{
   (void)obj1; (void)obj2;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Integer::lessThan_(const Object* obj1, const Object* obj2)
{
   (void)obj1; (void)obj2;
   return 0 ; //FIXME
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
