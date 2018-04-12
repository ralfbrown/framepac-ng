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

#include <map>
#include "framepac/map.h"
#include "framepac/fasthash64.h"

using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

Allocator Map::s_allocator(FramepaC::Object_VMT<Map>::instance(),sizeof(Map)) ;

/************************************************************************/
/************************************************************************/

Map::Map(size_t initial_size)
   : m_size(0), m_map()
{
   (void)initial_size ; //FIXME
   //FIXME

   return ;
}

//----------------------------------------------------------------------------

Map::Map(const Map *) : Object(), m_size(0), m_map()
{

   return ;
}

//----------------------------------------------------------------------------

Map::Map(const Map &) : Object(), m_size(0), m_map()
{

   return ;
}

//----------------------------------------------------------------------------

Map::~Map()
{

   return ;
}

//----------------------------------------------------------------------------

ObjectPtr Map::clone_(const Object *)
{

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Map::subseq_int(const Object *,size_t start, size_t stop)
{
   (void)start; (void)stop ; //FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr Map::subseq_iter(const Object *,ObjectIter start, ObjectIter stop)
{
   (void)start; (void)stop ; //FIXME
   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

size_t Map::cStringLength_(const Object* obj, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   size_t len = indent + 4 ;
   bool first { true } ;
   for (const auto key : static_cast<const Map*>(obj)->m_map)
      {
      if (first)
	 first = false ;
      else
	 ++len ;
      len += key.first->cStringLength(wrap_at,indent,wrapped_indent) ;
      ++len ;
      len += key.second->cStringLength(wrap_at,indent,wrapped_indent) ;
      }
   return len ;
}

//----------------------------------------------------------------------------

char* Map::toCstring_(const Object* obj, char* buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   if (buflen < indent + 4) return buffer ;
   char* bufend = buffer + buflen ;
   buffer += snprintf(buffer,buflen,"%*s",(int)indent,"#M(") ;
   bool first { true } ;
   for (const auto key : static_cast<const Map*>(obj)->m_map)
      {
      if (first)
	 first = false ;
      else
	 *buffer++ = ' ' ;
      buffer = key.first->toCstring(buffer,bufend-buffer,wrap_at,0,wrapped_indent) ;
      *buffer++ = ' ' ;
      buffer = key.second->toCstring(buffer,bufend-buffer,wrap_at,0,wrapped_indent) ;
      }
   *buffer++ = ')' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t Map::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Map::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool wrap, size_t indent)
{
   (void)obj; (void)buflen; (void)wrap; (void)indent; //FIXME
   if (!buffer)
      return false ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

size_t Map::hashValue_(const Object* obj)
{
   const Map* map = static_cast<const Map*>(obj) ;
   uint64_t hashstate = fasthash64_init(map->size()) ;
//FIXME: add in the hash values of each of the elements in the map

   return fasthash64_finalize(hashstate) ;
}

//----------------------------------------------------------------------------

bool Map::equal_(const Object *obj,const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int Map::compare_(const Object *obj,const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Map::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

/************************************************************************/
/************************************************************************/


} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Map>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Map> ;

} // end namespace FramepaCC


// end of file map.C //
