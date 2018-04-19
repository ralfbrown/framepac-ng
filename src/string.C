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
#include "framepac/fasthash64.h"
#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

Allocator String::s_allocator(FramepaC::Object_VMT<String>::instance(),sizeof(String)) ;
Initializer<String> String::s_initializer ;

static constexpr unsigned allocator_sizes[] =
   { 2, 4, 6, 8, 10, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 112, 128, 144, 160 } ;
static constexpr unsigned num_allocators = lengthof(allocator_sizes) ;
static constexpr unsigned max_small_alloc = allocator_sizes[num_allocators-1] ;

static SmallAlloc* allocators[max_small_alloc] ;

/************************************************************************/
/************************************************************************/

String::String(const char* s, size_t len)
   : Object(),
     m_buffer()
{
   this->init(s,len) ;
   return ;
}

//----------------------------------------------------------------------------

String::String(const Object *o)
   : Object(),
     m_buffer()
{
   if (!o)
      return  ;
   if (o->isString())
      {
      auto s = static_cast<const String*>(o) ;
      const char* str = s->c_str() ;
      size_t len = s->c_len() ;
      this->init(str,len) ;
      }
   else
      {
      const char* str = o->stringValue() ;
      if (!str)
	 return ;
      size_t len = strlen(str)+1 ;
      this->init(str,len) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void String::init(const char* s, size_t len)
{
   uint16_t coded_length = len >= 0xFFFF ? 0xFFFF : len ;
   size_t offset = (coded_length == 0xFFFF) ? sizeof(size_t) : 0 ;
   char* strbuf ;
   if (len < max_small_alloc)
      strbuf = (char*)allocators[len]->allocate() ;
   else
      strbuf = new char[len+1+offset] ;
   if (strbuf)
      {
      if (offset)
	 {
	 *((size_t*)strbuf) = len ;
	 strbuf += offset ;
	 }
      memcpy(strbuf,s,len) ;
      strbuf[len] = '\0' ;
      }
   new (&m_buffer) FramepaC::PointerPlus16<char>(strbuf,coded_length) ;
   return ;
}

//----------------------------------------------------------------------------

String::~String()
{
   if (c_len() < max_small_alloc)
      Allocator::free(m_buffer.pointer()) ;
   else
      {
      char* buf = m_buffer.pointer() ;
      if (buf && m_buffer.extra() == 0xFFFF)
	 buf -= sizeof(size_t) ;
      delete buf ;
      }
   return ;
}

//----------------------------------------------------------------------------

void String::StaticInitialization()
{
   unsigned prev_size = 0 ;
   for (size_t i = 0 ; i < num_allocators ; ++i)
      {
      size_t size = allocator_sizes[i] ;
      SmallAlloc *alloc = SmallAlloc::create(size,1) ;
      for (size_t j = prev_size ; j < size ; ++j)
	 {
	 allocators[j] = alloc ;
	 }
      prev_size = size ;
      }
   return ;
}

//----------------------------------------------------------------------------

ObjectPtr String::clone_(const Object *obj)
{
   if (!obj) return ObjectPtr(nullptr) ;
   return ObjectPtr(new String((String*)obj)) ;
}

//----------------------------------------------------------------------------

ObjectPtr String::subseq_int(const Object *obj,size_t start, size_t stop)
{
   auto str = static_cast<const String*>(obj) ;
   return ObjectPtr(new String(str->c_str()+start,stop-start)) ;
}

//----------------------------------------------------------------------------

ObjectPtr String::subseq_iter(const Object *, ObjectIter start, ObjectIter stop)
{
   auto str = static_cast<String*>(start.baseObject()) ;
   return ObjectPtr(new String(str->c_str()+start.currentIndex(),stop.currentIndex()-start.currentIndex())) ;
}

//----------------------------------------------------------------------------

size_t String::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   const String *str { static_cast<const String*>(obj) };
   const char* cstr = str->c_str() ;
   size_t len = str->c_len() ;
   for (size_t i = 0 ; i < len ; ++i)
      {
      char c = cstr[i] ;
      if (c == '\0' || c == '"' || c == '\n' || c == '\r' || c == '\\')
	 len++ ;			// account for escape characters
      }
   len += indent + 2 ;	    // account for indentation and quotes
   return len ;
}

//----------------------------------------------------------------------------

char* String::toCstring_(const Object *obj, char *buffer, size_t buflen,
   			 size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   const String *str { reinterpret_cast<const String*>(obj) };
   char* dest = buffer ;
   // print the indentation and opening quote
   dest += snprintf(buffer,buflen,"%*s\"",(int)indent,"") ;
   const char* cstr = str->c_str() ;
   size_t len = str->c_len() ;
   for (size_t i = 0 ; i < len && dest < buffer + buflen ; ++i)
      {
      char c = cstr[i] ;
      // escape special characters, as well as the escape character itself
      if (c == '\0' || c == '"' || c == '\n' || c == '\r' || c == '\\')
	 {
	 *dest++ = '\\' ;
	 if (dest >= buffer + buflen) break  ;
	 if (c == '\0') c = '0' ;
	 else if (c == '\r') c = 'r' ;
	 else if (c == '\n') c = 'n' ;
	 }
      *dest++ = c ;
      }
   if (dest < buffer + buflen)
      *dest++ = '"' ;
   if (dest < buffer + buflen)
      *dest = '\0' ;
   return dest ;
}

//----------------------------------------------------------------------------

size_t String::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   return cStringLength_(obj,wrap,indent,indent) ;
}

//----------------------------------------------------------------------------

bool String::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool /*wrap*/, size_t indent)
{
   return toCstring_(obj,buffer,buflen,~0,indent,indent) ;
}

//----------------------------------------------------------------------------

size_t String::hashValue_(const Object* obj)
{
   const char* str = reinterpret_cast<const String*>(obj)->c_str() ;
   size_t len = reinterpret_cast<const String*>(obj)->c_len() ;
   return FramepaC::fasthash64(str,len) ;
}

//----------------------------------------------------------------------------

bool String::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;
   if (!obj || !other) return false ;
   auto str1 = static_cast<const String*>(obj) ;
   auto str2 = static_cast<const String*>(other) ;
   const char* cstr1 = str1->c_str() ;
   const char* cstr2 = str2->c_str() ;
   size_t len1 = str1->c_len() ;
   size_t len2 = str2->c_len() ;
   if (!cstr1 || !cstr2 || len1 != len2) return false ;
   return memcmp(cstr1,cstr2,len1) == 0 ;
}

//----------------------------------------------------------------------------

int String::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;
   if (!other) return +1 ;
   if (!obj) return -1 ;
   auto str1 = static_cast<const String*>(obj) ;
   auto str2 = static_cast<const String*>(other) ;
   const char* cstr1 = str1->c_str() ;
   const char* cstr2 = str2->c_str() ;
   size_t len1 = str1->c_len() ;
   size_t len2 = str2->c_len() ;
   int cmp = memcmp(cstr1,cstr2,std::min(len1,len2)) ;
   if (cmp) return cmp ;
   else if (len1 < len2) return -1 ;
   else if (len1 > len2) return +1 ;
   return 0 ;
}

//----------------------------------------------------------------------------

int String::lessThan_(const Object *obj, const Object *other)
{
   return compare_(obj,other) < 0 ;
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
