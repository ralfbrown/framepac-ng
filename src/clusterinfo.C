/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-07					*/
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
#include "framepac/utility.h"

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
   size_t num_elts = elts ? elts->size() : 0 ;
   ClusterInfo* info = new ClusterInfo ;
   if (num_elts)
      {
      info->m_members = RefArray::create(num_elts) ;
      for (auto obj : *elts)
	 {
	 info->m_members->append(obj) ;
	 }
      }
   size_t num_subclus = subclus ? subclus->size() : 0 ;
   if (num_subclus)
      {
      info->m_subclusters = RefArray::create(num_subclus) ;
      for (auto obj : *subclus)
	 {
	 info->m_subclusters->append(obj) ;
	 }
      }
   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::create(Object** vectors, size_t num_vectors)
{
   ClusterInfo* info = new ClusterInfo ;
   if (!vectors || num_vectors == 0)
      return info ;
   info->m_members = RefArray::create(num_vectors) ;
   for (size_t i = 0 ; i < num_vectors ; ++i)
      {
      info->m_members->append(vectors[i]) ;
      }
   info->m_size = num_vectors ;
   info->setFlag(Flags::flat) ;		// direct members only, no subclusters
   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::create(ClusterInfo** subclus, size_t num_subclus)
{
   ClusterInfo* info = new ClusterInfo ;
   if (!subclus || num_subclus == 0)
      return info ;
   info->m_subclusters = RefArray::create(num_subclus) ;
   size_t count { 0 } ;
   for (size_t i = 0 ; i < num_subclus ; ++i)
      {
      if (subclus[i])
	 {
	 count += subclus[i]->size() ;
	 info->m_subclusters->append(subclus[i]) ;
	 }
      }
   info->m_size = count ;
   info->setFlag(Flags::group) ;	// subclusters only, no direct members
   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::create(const ClusterInfo** subclus, size_t num_subclus)
{
   ClusterInfo* info = new ClusterInfo ;
   if (!subclus || num_subclus == 0)
      return info ;
   info->m_subclusters = RefArray::create(num_subclus) ;
   size_t count { 0 } ;
   for (size_t i = 0 ; i < num_subclus ; ++i)
      {
      if (subclus[i])
	 {
	 count += subclus[i]->size() ;
	 info->m_subclusters->append(const_cast<ClusterInfo*>(subclus[i])) ;
	 }
      }
   info->m_size = count ;
   info->setFlag(Flags::group) ;	// subclusters only, no direct members
   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::createSingleton(const Object* vector)
{
   ClusterInfo* info = new ClusterInfo ;
   info->m_members = RefArray::create(1) ;
   info->m_members->append(const_cast<Object*>(vector)) ;
   if (vector)
      info->setLabel(vector->label()) ;
   info->m_size = 1 ;
   info->setFlag(Flags::flat) ;		// no subclusters, only direct members
   return info  ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::createSingletonClusters(const Array* vectors)
{
   ClusterInfo* info = new ClusterInfo ;
   if (vectors)
      {
      size_t num_clusters = vectors->size() ;
      info->m_subclusters = Array::create(num_clusters) ;
      for (auto obj : *vectors)
	 {
	 ClusterInfo* subcluster = ClusterInfo::createSingleton(obj) ;
	 if (!subcluster->label())
	    {
	    subcluster->setLabel(genLabel()) ;
	    }
	 info->m_subclusters->append(subcluster) ;
	 }
      info->m_size = num_clusters ;
      info->setFlag(Flags::group) ;	// subclusters only, no direct members
      }
   return info ;
}

//----------------------------------------------------------------------------

ClusterInfo* ClusterInfo::merge(const ClusterInfo* other, bool flatten) const
{
   if (!other)
      return static_cast<ClusterInfo*>(clone().move()) ;
   ClusterInfo* info = ClusterInfo::create() ;
   info->setLabel(genLabel()) ;
   info->m_size = this->m_size + other->m_size ;
   if (flatten)
      {
      RefArray* our_vectors = this->allMembers() ;
      RefArray* other_vectors = other->allMembers() ;
      RefArray* vectors = RefArray::create(info->m_size) ;
      for (auto vec : *our_vectors)
	 {
	 vectors->append(vec) ;
	 }
      for (auto vec : *other_vectors)
	 {
	 vectors->append(vec) ;
	 }
      info->m_members = vectors ;
      }
   else
      {
      Array* subclus = Array::create(2) ;
      subclus->append(this) ;
      subclus->append(other) ;
      info->m_subclusters = subclus ;
      }
   return info  ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::merge(size_t clusternum1, size_t clusternum2, bool flatten)
{
   if (clusternum1 == clusternum2)
      return false ;			// nothing to be merged
   if (clusternum1 > clusternum2) std::swap(clusternum1,clusternum2) ;
   auto cluster1 = static_cast<ClusterInfo*>(subclusters()->getNth(clusternum1)) ;
   auto cluster2 = static_cast<ClusterInfo*>(subclusters()->getNth(clusternum2)) ;
   if (!cluster1 || !cluster2)
      return false ;
   auto merged = cluster1->merge(cluster2,flatten) ;
   m_subclusters->setNth(clusternum1,merged) ;
   m_subclusters->elide(clusternum2) ;
   return true;
}

//----------------------------------------------------------------------------

bool ClusterInfo::allMembers(RefArray* mem) const
{
   if (members())
      {
      for (auto member : *members())
	 {
	 mem->append(member) ;
	 }
      }
   if (subclusters())
      {
      // recursively add members from subclusters
      for (auto subclus : *subclusters())
	 {
	 if (subclus)
	    static_cast<ClusterInfo*>(subclus)->allMembers(mem) ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

RefArray* ClusterInfo::allMembers() const
{
   RefArray* mem = RefArray::create(this->m_size) ;
   (void)allMembers(mem) ;
   return mem ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::allKeys(Array* keys) const
{
   if (members())
      {
      for (auto member : *members())
	 {
	 VectorBase* vec = static_cast<VectorBase*>(member) ;
	 keys->append(vec->key()) ;
	 }
      }
   if (subclusters())
      {
      // recursively add keys from members in subclusters
      for (auto subclus : *subclusters())
	 {
	 if (subclus)
	    static_cast<ClusterInfo*>(subclus)->allKeys(keys) ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

Array* ClusterInfo::allKeys() const
{
   Array* keys = Array::create(this->m_size) ;
   (void)allKeys(keys) ;
   return keys ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::flattenSubclusters()
{
   if (!m_subclusters)
      return true ;			// trivially successful
   for (auto subclus : *m_subclusters)
      {
      auto subcluster = static_cast<ClusterInfo*>(subclus) ;
      RefArray* members = subcluster->allMembers() ;
      if (subcluster->m_members)
	 subcluster->m_members->free() ;
      subcluster->m_members = members ;
      if (subcluster->m_subclusters)
	 {
	 subcluster->m_subclusters->free() ;
	 subcluster->m_subclusters = nullptr ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::labelSubclusterPaths(bool (*fn)(Object*, const char* label), const char* prefix,
   const char* sep)
{
   if (!fn)
      return false ;
   if (!prefix)
      prefix = "" ;
   if (!sep)
      sep = " " ;
   bool success { true } ;
   int count { 0 } ;
   // apply the current path prefix to all direct members of this cluster
   if (members())
      {
      for (auto v : *members())
	 {
	 success = fn(v,prefix) ;
	 if (!success)
	    return false ;
	 }
      }
   // now recurse down the subclusters, extending the given path prefix with the relative subcluster number
   if (subclusters())
      {
      for (auto sub : *subclusters())
	 {
	 ScopedCharPtr new_prefix { aprintf("%s%s%d%c",prefix,*prefix?sep:"",count++,'\0') } ;
	 auto subcluster = static_cast<ClusterInfo*>(sub) ;
	 success = subcluster->labelSubclusterPaths(fn,new_prefix,sep) ;
	 if (!success)
	    break ;
	 }
      }
   return success ;
}

//----------------------------------------------------------------------------

Symbol* ClusterInfo::genLabel()
{
   size_t id = ++next_cluster_ID ;
   ScopedCharPtr symname { Fr::aprintf("<CL_%04lu>",id) } ;
   return Symbol::create(symname) ;
}

//----------------------------------------------------------------------------

static Symbol* number ;

Symbol* ClusterInfo::numberLabel()
{
   if (!number) number = Symbol::create("<NUMBER>") ;
   return number ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::isGeneratedLabel(const char* name)
{
   if (!name || strncmp(name,"<CL_",4) != 0)
      return false ;
   name += 4 ;  // skip the prefix
   // skip over disambiguating digits
   while (*name && isdigit(*name))
      name++ ;
   // verify the suffix following the digits
   if (name[0] != '>' || name[1] != '\0')
      return false ;
   // all checks passed, this is indeed a cluster label produced by genLabel()
   return true ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::isNumberLabel(const char* name)
{
   return name && strcmp(name,"<NUMBER>") == 0 ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::isNumberLabel(const Symbol* name)
{
   return name && name == number ;
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
   m_flags |= (1>>(int)f) ;
   return ;
}

//----------------------------------------------------------------------------

void ClusterInfo::clearFlag(ClusterInfo::Flags f)
{
   m_flags &= ~(1>>(int)f) ;
   return ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::addVectors(const RefArray* vectors)
{
   for (auto vec : *vectors)
      {
      addVector(vec) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

void ClusterInfo::sortMembers(ObjectCompareFn* cmp)
{
   if (m_members)
      m_members->sort(cmp) ;
   return ;
}

//----------------------------------------------------------------------------

void ClusterInfo::sortSubclusters(ObjectCompareFn* cmp)
{
   if (m_subclusters)
      m_subclusters->sort(cmp) ;
   return ;
}

//----------------------------------------------------------------------------

ObjectPtr ClusterInfo::clone_(const Object* orig)
{
   auto info = static_cast<const ClusterInfo*>(orig) ;
   auto copy = ClusterInfo::create() ;
   copy->m_members = info->m_members ? static_cast<RefArray*>(info->m_members->clone().move()) : nullptr ;
   copy->m_subclusters = info->m_subclusters ? static_cast<RefArray*>(info->m_subclusters->clone().move()) : nullptr ;
   copy->m_rep = info->m_rep ? info->m_rep->clone().move() : nullptr ;
   copy->m_label = info->m_label ;
   copy->m_size = info->m_size ;
   copy->m_flags = info->m_flags ;
   copy->m_cluster_rep = info->m_cluster_rep ;
   return copy ;
}

//----------------------------------------------------------------------------

ObjectPtr ClusterInfo::subseq_int(const Object* obj,size_t start, size_t stop)
{
   if (!obj || start > stop)
      return nullptr ;
   auto clus = static_cast<const ClusterInfo*>(obj) ;
   if (clus->m_members && !clus->m_subclusters)
      {
      LocalAlloc<Object*> vectors(stop-start) ;
      for (size_t i = start ; i < stop ; ++i)
	 {
	 vectors[i-start] = clus->m_members->getNth(i) ;
	 }
      return ClusterInfo::create(vectors,stop-start) ;
      }
   else if (clus->m_subclusters && !clus->m_members)
      {
      LocalAlloc<ClusterInfo*> orig(stop-start) ;
      for (size_t i = start ; i < stop ; ++i)
	 {
	 orig[i-start] = static_cast<ClusterInfo*>(clus->m_subclusters->getNth(i)) ;
	 }
      return ClusterInfo::create(orig,stop-start) ;
      }
   else
      {
      //TODO
      }
   return nullptr ;
}

//----------------------------------------------------------------------------

ObjectPtr ClusterInfo::subseq_iter(const Object*, ObjectIter start, ObjectIter stop)
{
   return subseq_int(start.baseObject(),start.currentIndex(),stop.currentIndex()) ;
}

//----------------------------------------------------------------------------

size_t ClusterInfo::cStringLength_(const Object* obj, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   size_t len = indent + 5 + strlen(obj->typeName()) ;
   auto info = static_cast<const ClusterInfo*>(obj) ;
   if (info->label())
      {
      len += info->label()->cStringLength(wrap_at,0,wrapped_indent) ;
      }
   if (info->members())
      {
      len += info->members()->cStringLength(wrap_at,0,wrapped_indent) ;
      if (info->subclusters())
	 len++ ;			// account for separating blank
      }
   if (info->subclusters() && info->subclusters()->size() > 0)
      {
      len += info->subclusters()->size() - 1 ; // account for separating blanks
      for (auto sub : *info->subclusters())
	 {
	 len += sub->cStringLength(wrap_at,indent,wrapped_indent) ;
	 }
      }
   return len ;
}

//----------------------------------------------------------------------------

char* ClusterInfo::toCstring_(const Object* obj, char* buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   size_t count = snprintf(buffer,buflen,"%*s#<%s:",(int)indent,"",obj->typeName()) ;
   buffer += count ;
   buflen -= count ;
   auto info = static_cast<const ClusterInfo*>(obj) ;
   if (info->label())
      {
      char* rest = info->label()->toCstring(buffer,buflen,wrap_at,0,wrapped_indent) ;
      buflen -= (rest - buffer) ;
      buffer = rest ;
      }
   if (buflen > 0)
      {
      *buffer++ = ':' ;
      buflen-- ;
      }
   // print out the direct members
   if (info->members() && info->members()->size() > 0)
      {
      char* rest = info->members()->toCstring(buffer,buflen,wrap_at,0,wrapped_indent) ;
      buflen -= (rest - buffer) ;
      buffer = rest ;
      if (buflen > 0 && info->subclusters())
	 {
	 *buffer++ = ' ' ;
	 buflen-- ;
	 }
      }
   if (info->subclusters())
      {
      bool first = true ;
      for (auto sub : *info->subclusters())
	 {
	 if (buflen == 0)
	    break ;
	 // print out a separating blank if needed
	 if (first)
	    first = false ;
	 else
	    {
	    *buffer++ = ' ' ;
	    buflen-- ;
	    }
	 // print out the subcluster
	 char* rest = sub->toCstring(buffer,buflen,wrap_at,indent,wrapped_indent) ;
	 buflen -= (rest - buffer) ;
	 buffer = rest ;
	 }
      }
   if (buflen > 0)
      {
      *buffer++ = '>' ;
      buflen-- ;
      }
   if (buflen > 0)
      *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t ClusterInfo::jsonStringLength_(const Object* obj, bool /*wrap*/, size_t indent)
{
   // for now, represent as a JSON string whose content is the C string representation
   return 2 + cStringLength_(obj,~0,indent,indent) ;
}

//----------------------------------------------------------------------------

bool ClusterInfo::toJSONString_(const Object* obj, char* buffer, size_t buflen,
   bool /*wrap*/, size_t indent)
{
   if (!buffer || buflen == 0)
      return false ;
   // for now, represent as a JSON string whose content is the C string representation
   *buffer++ = '"' ;
   buflen-- ;
   const char* bufend = buffer + buflen  ;
   buffer = toCstring_(obj,buffer,buflen,~0,indent,indent) ;
   if (buffer < bufend)
      {
      *buffer++ = '"' ;
      return true ;
      }
   return false ;
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
   auto info1 = static_cast<const ClusterInfo*>(obj) ;
   auto info2 = static_cast<const ClusterInfo*>(other) ;
   if (info1->m_flags != info2->m_flags || info1->m_label != info2->m_label ||
      info1->m_cluster_rep != info2->m_cluster_rep)
      return false ;
   if (info1->m_members || info2->m_members)
      {
      if (!info1->m_members || !info1->m_members->equal(info2->m_members))
	 return false ;
      }
   if (info1->m_subclusters || info2->m_subclusters)
      {
      if (!info1->m_subclusters || !info1->m_subclusters->equal(info2->m_subclusters))
	 return false ;
      }
   if (info1->m_rep || info2->m_rep)
      {
      if (!info1->m_rep || !info1->m_rep->equal(info2->m_rep))
	 return false ;
      }
   // all fields compare equal
   return true ;
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
   return compare_(obj,other) < 0 ;
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
