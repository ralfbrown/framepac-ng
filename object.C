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

#include <iostream>
#include "framepac/object.h"

using namespace FramepaC ;
using namespace Fr ;

namespace Fr
{

// /*static*/ const FramepaC::Object_VMT<Object> object_vmt ;

/************************************************************************/
/*	Methods for class Object					*/
/************************************************************************/

ObjectPtr Object::clone_(const Object *)
{
   return ObjectPtr(new Object) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::subseq_int(const Object *obj, size_t start, size_t stop)
{
   if (start <= stop)
      return obj->clone() ;
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::subseq_iter(const Object *obj, ObjectIter start, ObjectIter stop)
{
   (void)obj; (void)start; (void)stop; //FIXME
//   if (start <= stop)
//      return clone() ;
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

char *Object::cString(size_t wrap_at, size_t indent) const
{
   size_t buflen { cStringLength(wrap_at,indent) };
   char *buffer { new char[buflen+1] };
   if (toCstring(buffer,buflen+1,wrap_at,indent))
      return buffer ;
   else
      {
      delete [] buffer ;
      return nullptr ;
      }
}

//----------------------------------------------------------------------------

size_t Object::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent)
{
   return snprintf(nullptr,0,"%*s#Object<%lu>",(int)indent,"",(unsigned long)obj) ;
}

//----------------------------------------------------------------------------

bool Object::toCstring_(const Object *obj, char *buffer, size_t buflen, size_t /*wrap_at*/, size_t indent)
{
   if (!buffer)
      return false ;
   size_t count = snprintf(buffer,buflen,"%*s#Object<%lu>%c",(int)indent,"",(unsigned long)obj,'\0') ;
   return count <= buflen ;
}

//----------------------------------------------------------------------------

char *Object::jsonString(bool wrap, size_t indent) const
{
   size_t buflen { jsonStringLength(wrap,indent) };
   char *buffer { new char[buflen+1] };
   if (toJSONString(buffer,buflen+1,wrap,indent))
      return buffer ;
   else
      {
      delete [] buffer ;
      return nullptr ;
      }
}

//----------------------------------------------------------------------------

size_t Object::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Object::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool wrap, size_t indent)
{
   (void)obj; (void)buflen; (void)wrap; (void)indent; //FIXME
   if (!buffer)
      return false ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

mpz_t Object::bignumValue_(const Object *)
{
   return mpz_t(0) ;
}

//----------------------------------------------------------------------------

mpq_t Object::rationalValue_(const Object *)
{
   return mpq_t(0) ;
}

//----------------------------------------------------------------------------

size_t Object::hashValue_(const Object *obj)
{
   // since base object have no contents, the hash value is actually meaningless,
   //   but we need this function as the base of inheritance, so just return the
   //   object's address
   return (size_t)obj ;
}

//----------------------------------------------------------------------------

int Object::compare_(const Object *obj1, const Object *obj2)
{
   if (lessThan_(obj1,obj2)) return -1 ;
   else if (lessThan_(obj2,obj1)) return +1 ;
   return 0 ;
}

//----------------------------------------------------------------------------

int Object::lessThan_(const Object *obj1, const Object *obj2)
{
   return obj1 < obj2 ;
}

//----------------------------------------------------------------------------

void Object::_() const
{
   char *printed { cString() };
   cerr << printed << endl ;
   delete [] printed ;
   return ;
}

//----------------------------------------------------------------------------

#if 0
mpz_t Object::bignumValue_(const Object *)
{
}
#endif

//----------------------------------------------------------------------------

#if 0
mpq_t Object::rationalValue(const Object *)
{
}
#endif

//----------------------------------------------------------------------------

/************************************************************************/
/*	Procedural Interface functions					*/
/************************************************************************/

bool equal(const Object* obj1, const Object* obj2)
{
   if (!obj1)
      {
      return !obj2 ;
      }
   else if (!obj2)
      return false ;
   else
      return obj1->equal(obj2) ;
}

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Object>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Object> ;

} // end namespace FramepaCC

// end of file object.C //
