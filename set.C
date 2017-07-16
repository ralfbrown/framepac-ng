/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-15					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

#include "framepac/set.h"
#include "framepac/fasthash64.h"

using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

Allocator Set::s_allocator(FramepaC::Object_VMT<Set>::instance(),sizeof(Set)) ;

/************************************************************************/
/************************************************************************/

Set::Set(size_t initial_size)
   : m_size(0), m_set()
{
   (void)initial_size ; //FIXME
   //FIXME

   return ;
}

//----------------------------------------------------------------------------

Set::Set(const Set *) : Object(), m_size(0), m_set()
{

   return ;
}

//----------------------------------------------------------------------------

Set::Set(const Set &) : Object(), m_size(0), m_set()
{

   return ;
}

//----------------------------------------------------------------------------

Set::~Set()
{

   return ;
}

//----------------------------------------------------------------------------

ObjectPtr Set::clone_(const Object *)
{

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Set::subseq_int(const Object *,size_t start, size_t stop)
{
   (void)start; (void)stop ; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Set::subseq_iter(const Object *,ObjectIter start, ObjectIter stop)
{
   (void)start; (void)stop ; //FIXME
   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

size_t Set::cStringLength_(const Object *, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   size_t len = indent + 4 ;
   (void)wrap_at; (void)wrapped_indent; //FIXME
   return len ;
}

//----------------------------------------------------------------------------

char* Set::toCstring_(const Object *, char *buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   if (buflen < indent + 4) return buffer ;
   char* bufend = buffer + buflen ;
   buffer += snprintf(buffer,buflen,"%*s",(int)indent,"#H(") ;
   
   (void)bufend; (void)wrap_at; (void)wrapped_indent; //FIXME
   //FIXME
   *buffer++ = ')' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t Set::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Set::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool wrap, size_t indent)
{
   (void)obj; (void)buflen; (void)wrap; (void)indent; //FIXME
   if (!buffer)
      return false ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

size_t Set::hashValue_(const Object* obj)
{
   const Set* set = static_cast<const Set*>(obj) ;
   uint64_t hashstate = fasthash64_init(set->size()) ;
//FIXME: add in the hash values of each of the elements in the set

   return fasthash64_finalize(hashstate) ;
}

//----------------------------------------------------------------------------

bool Set::equal_(const Object *obj,const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int Set::compare_(const Object *obj,const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Set::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

/************************************************************************/
/************************************************************************/


} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Set>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Set> ;

} // end namespace FramepaCC


// end of file set.C //
