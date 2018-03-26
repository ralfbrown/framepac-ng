/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-26					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#include "framepac/cluster.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

// define the static members of ClusterInfo
Allocator ClusterInfo::s_allocator(FramepaC::Object_VMT<ClusterInfo>::instance(),sizeof(ClusterInfo)) ;
//FIXME? ClusterInfo::Initializer ClusterInfo::s_init ;

// register initialization and cleanup functions for the ClusterInfo class as a whole
// these will be called by Fr::Initialize() and Fr::Shutdown()
Fr::Initializer<ClusterInfo> static_init ;

/************************************************************************/
/************************************************************************/


//----------------------------------------------------------------------------

size_t ClusterInfo::cStringLength_(const Object* obj, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   (void)obj; (void)wrap_at; (void)indent; (void)wrapped_indent;
   //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

char* ClusterInfo::toCstring_(const Object* obj, char* buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)wrap_at; (void)indent; (void)wrapped_indent;
   //TODO
   return buffer ;
}

//----------------------------------------------------------------------------

size_t ClusterInfo::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent;
   //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::toJSONString_(const Object* obj, char* buffer, size_t buflen,
   bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent;
   return false ; //TODO
}

//----------------------------------------------------------------------------

size_t ClusterInfo::size_(const Object *obj)
{
   (void)obj;
   return 0; //TODO
}

//----------------------------------------------------------------------------

Object *ClusterInfo::front_(Object *obj)
{
   return obj ;
}

//----------------------------------------------------------------------------

const Object *ClusterInfo::front_(const Object *obj)
{
   return obj ;
}

//----------------------------------------------------------------------------

size_t ClusterInfo::hashValue_(const Object* obj)
{
   (void)obj;
   //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int ClusterInfo::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int ClusterInfo::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

void ClusterInfo::StaticInitialization()
{
   //TODO
   return ;
}

//----------------------------------------------------------------------------

void ClusterInfo::StaticCleanup()
{
   //TODO
   return ;
}


} // end of namespace Fr

// end of file clusterinfo.C //
