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

#include <map>
#include "framepac/map.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

// request explicit instantiation; we declared it "extern" in the header so this
//   will be the only copy of the non-inlined code generated in object modules
template class Allocator<Map> ;

static const FramepaC::Object_VMT<Map> map_vmt ;
Allocator<Map> Map::s_allocator(&map_vmt) ;

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

size_t Map::cStringLength_(const Object *, size_t wrap_at, size_t indent)
{
   (void)wrap_at; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool Map::toCstring_(const Object *, char *buffer, size_t buflen, size_t wrap_at, size_t indent)
{
   (void)buffer; (void)buflen; (void)wrap_at; (void)indent; //FIXME
   //FIXME
   return true ;
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
