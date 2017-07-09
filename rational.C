/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-09					*/
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

Allocator Rational::s_allocator(FramepaC::Object_VMT<Rational>::instance(),sizeof(Rational)) ;

/************************************************************************/
/************************************************************************/

Rational::Rational()
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

Rational::Rational(const char *value)
   : m_value(0)
{
   //FIXME
   (void)value ;
   return ;
}

//----------------------------------------------------------------------------

Rational::Rational(const Rational&)
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

Rational* Rational::create()
{
   return new Rational ;
}

//----------------------------------------------------------------------------

Rational* Rational::create(const Object*)
{
   return nullptr ;  //FIXME
}

//----------------------------------------------------------------------------

Rational* Rational::create(const Rational*)
{
   return nullptr ;  //FIXME
}

//----------------------------------------------------------------------------

Rational* Rational::create(const char*)
{
   return nullptr ;  //FIXME
}

//----------------------------------------------------------------------------

Rational::~Rational()
{
   //FIXME
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

size_t Rational::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ; (void)wrap; (void)indent ;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Rational::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			    bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

size_t Rational::hashValue_(const Object* obj)
{
   const Rational* rat = static_cast<const Rational*>(obj) ;
   (void)rat; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Rational::equal_(const Object* obj1, const Object* obj2)
{
   (void)obj1; (void)obj2;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

int Rational::compare_(const Object* obj1, const Object* obj2)
{
   (void)obj1; (void)obj2;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Rational::lessThan_(const Object* obj1, const Object* obj2)
{
   (void)obj1; (void)obj2;
   return 0 ; //FIXME
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
