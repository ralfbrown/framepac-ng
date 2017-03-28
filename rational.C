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

#include "framepac/rational.h"

using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

// request explicit instantiation; we declared it "extern" in the header so this
//   will be the only copy of the non-inlined code generated in object modules
template class Allocator<Rational> ;

static const FramepaC::Object_VMT<Rational> rational_vmt ;
Allocator<Rational> Rational::s_allocator(&rational_vmt) ;

/************************************************************************/
/************************************************************************/

Rational::Rational(const char *value)
{
   //FIXME
   (void)value ;
   return ;
}

//----------------------------------------------------------------------------

size_t Rational::cStringLength_(const Object *obj, size_t wrap_at, size_t indent)
{
   (void)obj; (void)wrap_at; (void)indent;//FIXME
//   return snprintf(nullptr,0,"%ld",((Rational_*)obj)->m_value + indent) ;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Rational::toCstring_(const Object *obj, char *buffer, size_t buflen,
			  size_t wrap_at, size_t indent)
{
   (void)obj; (void)buffer; (void)wrap_at; (void)indent; //FIXME
//   size_t needed = snprintf(buffer,buflen,"%*s%ld",indent,"",((Rational_*)obj)->m_value) ;
   size_t needed = ~0 ; //FIXME
   return needed <= buflen ;
}

//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Rational>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Rational> ;

} // end namespace FramepaCC

// end of file rational.C //
