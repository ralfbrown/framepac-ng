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

static constexpr unsigned allocator_sizes[] =
   { 2, 4, 6, 8, 10, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 112, 128, 144, 160 } ;
static constexpr unsigned num_allocators = lengthof(allocator_sizes) ;
static constexpr unsigned max_small_alloc = allocator_sizes[num_allocators-1] ;

static SmallAlloc* allocators[max_small_alloc] ;

/************************************************************************/
/************************************************************************/

String::String(const char *s, size_t len)
   : Object(),
     m_buffer()
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
