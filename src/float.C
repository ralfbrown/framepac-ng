/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-12					*/
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
#include "framepac/fasthash64.h"

using namespace FramepaC ;
using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

Allocator Float::s_allocator(FramepaC::Object_VMT<Float>::instance(),sizeof(Float)) ;

/************************************************************************/
/************************************************************************/

Float::Float(const char *value)
   : m_value(value ? strtod(value,nullptr) : 0.0)
{
   return ;
}

//----------------------------------------------------------------------------

Float* Float::create(const char* value)
{
   return new Float(value) ;
}

//----------------------------------------------------------------------------

size_t Float::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   return indent + snprintf(nullptr,0,"%g",obj->floatValue()) ;
}

//----------------------------------------------------------------------------

char* Float::toCstring_(const Object *obj, char *buffer, size_t buflen,
   size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   size_t needed = snprintf(buffer,buflen,"%*s%g",(int)indent,"",obj->floatValue()) ;
   return (needed <= buflen) ? buffer + needed : buffer ;
}

//----------------------------------------------------------------------------

size_t Float::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Float::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool wrap, size_t indent)
{
   (void)obj; (void)buflen; (void)wrap; (void)indent; //FIXME
   if (!buffer)
      return false ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

size_t Float::hashValue_(const Object* obj)
{
   const Float* f = static_cast<const Float*>(obj) ;
   return fasthash64_float(f->m_value) ;
}

//----------------------------------------------------------------------------

bool Float::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;
   if (!other || !other->isNumber()) return false ;
   return obj->floatValue() == other->floatValue() ;
}

//----------------------------------------------------------------------------

int Float::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;
   if (!other || !other->isNumber()) return -1 ;
   if (obj->floatValue() < other->floatValue())
      return -1 ;
   else if (obj->floatValue() > other->floatValue())
      return +1 ;
   return 0 ;
}

//----------------------------------------------------------------------------

int Float::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;
   if (!other || !other->isNumber() || obj->floatValue() < other->floatValue()) return 1 ;
   return 0 ;
}

//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Float>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Float> ;

} // end namespace FramepaCC

// end of file float.C //
