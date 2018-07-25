/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-25					*/
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

#include <iostream>
#include "framepac/object.h"
#include "framepac/objreader.h"
#include "framepac/utility.h"

using namespace FramepaC ;
using namespace Fr ;

namespace Fr
{

// /*static*/ const FramepaC::Object_VMT<Object> object_vmt ;

/************************************************************************/
/*	Methods for class Object					*/
/************************************************************************/

ObjectPtr Object::create(const Object* obj)
{
   return obj ? obj->clone() : nullptr ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::clone_(const Object* obj)
{
   // pure virtual, must be overridden by each derived class
   return const_cast<Object*>(obj) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::subseq_int(const Object *obj, size_t start, size_t stop)
{
   if (start <= stop)
      return obj->clone() ;
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::subseq_iter(const Object *, ObjectIter /*start*/, ObjectIter /*stop*/)
{
   // base objects are not collections, so there is no subsequence to be taken
   return ObjectPtr(nullptr) ;
}

//----------------------------------------------------------------------------

ostream& Object::print(ostream& out) const
{
   const Object* o = this ;	// work around "this is never null" compiler warning
   if (o)
      {
      // TODO: make a virtual function that dispatches to the proper
      //   type so that we don't need to go via a string conversion
      //   first
      ScopedCharPtr printed { cString() } ;
      out << *printed << flush ;
      }
   else
      {
      out << "#N<>" << flush ;
      }
   return out ;
}

//----------------------------------------------------------------------------

char* Object::cString(size_t wrap_at, size_t indent, size_t wrapped_indent) const
{
   size_t buflen { cStringLength(wrap_at,indent,wrapped_indent) };
   char* buffer { new char[buflen+1] };
   if (toCstring(buffer,buflen+1,wrap_at,indent,wrapped_indent))
      return buffer ;
   else
      {
      delete [] buffer ;
      return nullptr ;
      }
}

//----------------------------------------------------------------------------

size_t Object::cStringLength_(const Object* obj, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   const char* type = obj ? obj->typeName() : "" ;
   return snprintf(nullptr,0,"%*s#<%s:%lu>",(int)indent,"",type,(unsigned long)obj) ;
}

//----------------------------------------------------------------------------

char* Object::toCstring_(const Object *obj, char *buffer, size_t buflen, size_t /*wrap_at*/, size_t indent,
   size_t /*wrapped_indent*/)
{
   if (!buffer)
      return buffer ;
   const char* type = obj ? obj->typeName() : "" ;
   size_t count = snprintf(buffer,buflen,"%*s#<%s:%lu>%c",(int)indent,"",type,(unsigned long)obj,'\0') ;
   return (count <= buflen) ? buffer + count : buffer ;
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

size_t Object::jsonStringLength_(const Object *obj, bool /*wrap*/, size_t indent)
{
   const char* type = obj ? obj->typeName() : "" ;
   return snprintf(nullptr,0,"%*s\"#<%s:%lu>\"",(int)indent,"",type,(unsigned long)obj) ;
}

//----------------------------------------------------------------------------

bool Object::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool /*wrap*/, size_t indent)
{
   if (!buffer)
      return buffer ;
   const char* type = obj ? obj->typeName() : "" ;
   size_t count = snprintf(buffer,buflen,"%*s\"#<%s:%lu>\"%c",(int)indent,"",type,(unsigned long)obj,'\0') ;
   return (count <= buflen) ;
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
   // since base objects have no contents, the hash value is actually meaningless,
   //   but we need this function as the base of inheritance, so just return the
   //   object's address
   return (size_t)obj ;
}

//----------------------------------------------------------------------------

int Object::compare_(const Object *obj1, const Object *obj2)
{
   if (obj1 < obj2) return -1 ;
   else if (obj1 > obj2) return +1 ;
   return 0 ;
}

//----------------------------------------------------------------------------

int Object::lessThan_(const Object *obj1, const Object *obj2)
{
   return obj1->compare(obj2) < 0 ;
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
   //TODO
}
#endif

//----------------------------------------------------------------------------

#if 0
mpq_t Object::rationalValue(const Object *)
{
   //TODO
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
