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

#include <cstring>
#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

Allocator String::s_allocator(FramepaC::Object_VMT<String>::instance(),sizeof(String)) ;

/************************************************************************/
/************************************************************************/

String::String(const char *s, size_t len)
   : Object(),
     m_buffer()
{
   uint16_t coded_length = len >= 0xFFFF ? 0xFFFF : len ;
   size_t offset = (coded_length == 0xFFFF) ? sizeof(size_t) : 0 ;
   char* strbuf = new char[len+1+offset] ;
   m_buffer.pointer(strbuf) ;
   m_buffer.extra(coded_length) ;
   if (offset)
      {
      *((size_t*)strbuf) = len ;
      strbuf += offset ;
      }
   memcpy(strbuf,s,len) ;
   strbuf[len] = '\0' ;
   return ;
}

//----------------------------------------------------------------------------

String::String(const Object *o)
   : Object(),
     m_buffer()
{
   if (o)
      {
   //FIXME
      }
   return ;
}

//----------------------------------------------------------------------------

ObjectPtr String::clone_(const Object *obj)
{
   return ObjectPtr(new String((String*)obj)) ;
}

//----------------------------------------------------------------------------

ObjectPtr String::subseq_int(const Object *,size_t start, size_t stop)
{
   (void)start; (void)stop; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr String::subseq_iter(const Object *, ObjectIter start, ObjectIter stop)
{
   (void)start; (void)stop; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

size_t String::cStringLength_(const Object *obj, size_t wrap_at, size_t indent)
{
   (void)wrap_at; //FIXME
   const String *str { reinterpret_cast<const String*>(obj) };
   //FIXME: need to quote special characters!
   return snprintf(nullptr,0,"%*s\"%s\"",(int)indent,"",str->stringValue()) ;
}

//----------------------------------------------------------------------------

bool String::toCstring_(const Object *obj, char *buffer, size_t buflen,
			size_t wrap_at, size_t indent)
{
   (void)wrap_at; //FIXME
   const String *str { reinterpret_cast<const String*>(obj) };
   //FIXME: need to quote special characters!
   size_t needed = snprintf(buffer,buflen,"%*s\"%s\"",(int)indent,"",str->stringValue()) ;
   return needed <= buflen ;
}

//----------------------------------------------------------------------------

size_t String::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool String::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool wrap, size_t indent)
{
   (void)obj; (void)buflen; (void)wrap; (void)indent; //FIXME
   if (!buffer)
      return false ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool String::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int String::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int String::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}


//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<String>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::String> ;

} // end namespace FramepaCC


// end of file string.C //
