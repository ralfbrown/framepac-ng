/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-03-30					*/
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

#include "framepac/atomic.h"
#include "framepac/cluster.h"
#include "framepac/texttransforms.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

// define the static members of ClusterInfo
Allocator ClusterInfo::s_allocator(FramepaC::Object_VMT<ClusterInfo>::instance(),sizeof(ClusterInfo)) ;
ClusterInfo::Initializer ClusterInfo::s_init ;

// register initialization and cleanup functions for the ClusterInfo class as a whole
// these will be called by Fr::Initialize() and Fr::Shutdown()
//Fr::Initializer<ClusterInfo> static_init ;

/************************************************************************/
/*	Global data for this module					*/
/************************************************************************/

static Atomic<size_t> next_cluster_ID { 0 } ;

/************************************************************************/
/************************************************************************/

ClusterInfo* ClusterInfo::create(const List* elts, const List* subclus)
{
   //TODO
   (void)elts ; (void)subclus;
   return nullptr ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::create(ClusterInfo** subclus, size_t num_subclus)
{
   ClusterInfo* info = new ClusterInfo ;
   if (!subclus) num_subclus = 0 ;
   for (size_t i = 0 ; i < num_subclus ; ++i)
      {
      if (subclus[i])
	 {
	 //TODO
	 }
      }
   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::create(const ClusterInfo** subclus, size_t num_subclus)
{
   ClusterInfo* info = new ClusterInfo ;
   if (!subclus) num_subclus = 0 ;
   for (size_t i = 0 ; i < num_subclus ; ++i)
      {
      if (subclus[i])
	 {
	 //TODO
	 }
      }
   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::createSingletonClusters(const Array* vectors)
{
   ClusterInfo* info = new ClusterInfo ;
   //TODO

   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::merge(const ClusterInfo* other) const
{
   if (!other)
      return static_cast<ClusterInfo*>(clone().move()) ;

   //TODO
   return nullptr; //FIXME
}

//----------------------------------------------------------------------------

Symbol* ClusterInfo::genLabel()
{
   size_t id = ++next_cluster_ID ;
   char* symname = Fr::aprintf("<CL_%lu>",id) ;
   Symbol *sym = Symbol::create(symname) ;
   delete[] symname ;
   return sym ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::addVector(Object* v)
{
   if (!m_members)
      m_members = RefArray::create() ;
   m_members->append(v) ;
   return true ;
}

//----------------------------------------------------------------------------

void ClusterInfo::setFlag(ClusterInfo::Flags f)
{
   (void)f ; //TODO
   return ;
}

//----------------------------------------------------------------------------

void ClusterInfo::clearFlag(ClusterInfo::Flags f)
{
   (void)f ; //TODO
   return ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::addVectors(const RefArray* vectors)
{
   for (size_t i = 0 ; i < vectors->size() ; ++i)
      {
      addVector(vectors->getNth(i)) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

ObjectPtr ClusterInfo::clone_(const Object*)
{
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr ClusterInfo::subseq_int(const Object*,size_t /*start*/, size_t /*stop*/)
{
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr ClusterInfo::subseq_iter(const Object*, ObjectIter /*start*/, ObjectIter /*stop*/)
{
   return nullptr ; //FIXME
}

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

Object *ClusterInfo::front_(Object* obj)
{
   ClusterInfo* inf = reinterpret_cast<ClusterInfo*>(obj) ;
   if (inf->m_members)
      return inf->m_members->front() ;
   else if (inf->m_subclusters)
      return inf->m_subclusters->front() ;
   else
      return nullptr ;
}

//----------------------------------------------------------------------------

const Object *ClusterInfo::front_(const Object *obj)
{
   const ClusterInfo* inf = reinterpret_cast<const ClusterInfo*>(obj) ;
   if (inf->m_members)
      return inf->m_members->front() ;
   else if (inf->m_subclusters)
      return inf->m_subclusters->front() ;
   else
      return nullptr ;
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
   if (ClusterInfo::size_(obj) != ClusterInfo::size_(other))
      return false ;

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
