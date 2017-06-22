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

#include <stdlib.h>
#include "framepac/number.h"

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

size_t Float::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent)
{
   return snprintf(nullptr,0,"%g",obj->floatValue() + indent) ;
}

//----------------------------------------------------------------------------

bool Float::toCstring_(const Object *obj, char *buffer, size_t buflen,
		       size_t /*wrap_at*/, size_t indent)
{
   size_t needed = snprintf(buffer,buflen,"%*s%g",(int)indent,"",obj->floatValue()) ;
   return needed <= buflen ;
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
