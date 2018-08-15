/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-15					*/
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

#include <stdarg.h>
#include "framepac/cluster.h"
#include "framepac/hashtable.h"
#include "framepac/progress.h"
#include "framepac/threadpool.h"

namespace Fr
{

/************************************************************************/
/*	methods for class ClusterInfo					*/
/************************************************************************/

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* ClusterInfo::createSparseCentroid() const
{
   auto centroid = SparseVector<IdxT,ValT>::create() ;
   for (auto vec : *members())
      {
      if (vec)
	 centroid->incr(static_cast<SparseVector<IdxT,ValT>*>(vec)) ;
      }
   centroid->setLabel(this->label()) ;
   return centroid ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* ClusterInfo::createSparseCentroid(const Array* vectors) const
{
   auto centroid = SparseVector<IdxT,ValT>::create() ;
   for (auto vec : *vectors)
      {
      if (vec)
	 centroid->incr(static_cast<SparseVector<IdxT,ValT>*>(vec)) ;
      }
   centroid->setLabel(this->label()) ;
   return centroid ;
}

//----------------------------------------------------------------------------

template <typename ValT>
DenseVector<ValT>* ClusterInfo::createDenseCentroid() const
{
   typedef DenseVector<ValT> DV ;
   auto mem = members() ;
   auto sz = mem ? mem->size() : 0 ;
   if (sz == 0)
      return DV::create() ;
   auto centroid = DV::create(static_cast<DV*>(mem->getNth(0))) ;
   for (size_t i = 1 ; i < sz ; i++)
      {
      auto vec = static_cast<DV*>(mem->getNth(i)) ;
      if (!vec) continue ;
      centroid->incr(vec) ;
      }
   centroid->setLabel(this->label()) ;
   return centroid ;
}

//----------------------------------------------------------------------------

template <typename ValT>
DenseVector<ValT>* ClusterInfo::createDenseCentroid(const Array* vectors) const
{
   typedef DenseVector<ValT> DV ;
   auto sz = vectors->size() ;
   if (sz == 0)
      return DV::create() ;
   auto centroid = DV::create(static_cast<DV*>(vectors->getNth(0))) ;
   for (size_t i = 1 ; i < sz ; i++)
      {
      auto vec = static_cast<DV*>(vectors->getNth(i)) ;
      if (!vec) continue ;
      centroid->incr(vec) ;
      }
   centroid->setLabel(this->label()) ;
   return centroid ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
void ClusterInfo::updateRepresentative(Vector<ValT>* vector)
{
   if (!vector)
      return ;
   if (this->repType() == ClusterRep::centroid)
      {
      if (representative())
	 {
	 //TODO: add vector to existing centroid
	 }
      else // the new vector *is* the centroid
	 this->m_rep = vector->clone().move() ;
      }
   else if (this->repType() == ClusterRep::newest)
      {
      this->m_rep = vector ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
void ClusterInfo::setRepresentative(VectorMeasure<IdxT,ValT>* vm)
{
   if (representative() && this->repType() != ClusterRep::newest)
      return  ;
   switch (this->repType())
      {
      case ClusterRep::centroid:
         {
	 auto vectors = this->allMembers() ;
	 if (vectors->getNth(0)->isSparseVector())
	    this->m_rep = this->createSparseCentroid<IdxT,ValT>(vectors) ;
	 else
	    this->m_rep = this->createDenseCentroid<ValT>(vectors) ;
	 vectors->free() ;
	 }
         break ;
      case ClusterRep::prototype:
	 if (this->members())
	    {
	    this->m_rep = this->members()->getNth(0) ;
	    }
	 else if (this->subclusters())
	    {
	    auto sub = static_cast<ClusterInfo*>(this->subclusters()->getNth(0)) ;
	    if (sub) sub->setRepresentative(vm) ;
	    this->m_rep = sub->representative() ;
	    }
	 break ;
      case ClusterRep::newest:
	 if (this->members() && this->members()->size() > 0)
	    {
	    this->m_rep = this->members()->getNth(this->members()->size()-1) ;
	    }
	 else if (this->subclusters() && this->subclusters()->size() > 0)
	    {
	    auto sub = static_cast<ClusterInfo*>(this->subclusters()->getNth(this->subclusters()->size()-1)) ;
	    if (sub) sub->setRepresentative(vm) ;
	    this->m_rep = sub->representative() ;
	    }
	 break ;
      case ClusterRep::medioid:
         {
	 // compute the centroid of the cluster's members
	 auto vectors = this->allMembers() ;
	 Vector<ValT>* centroid ;
	 if (vectors->getNth(0)->isSparseVector())
	    centroid = this->createSparseCentroid<IdxT,ValT>(vectors) ;
	 else
	    centroid = this->createDenseCentroid<ValT>(vectors) ;
	 // find the member vector closest to the centroid
	 double best_sim = -HUGE_VAL ;
	 Vector<ValT>* mediod = nullptr ;
	 for (auto vec : *vectors)
	    {
	    Vector<ValT>* vector = static_cast<Vector<ValT>*>(vec) ;
	    double sim = vm->similarity(vector,centroid) ;
	    if (sim > best_sim)
	       {
	       best_sim = sim ;
	       mediod = vector ;
	       }
	    }
	 this->m_rep = mediod ;
	 vectors->free() ;
	 }
         break ;
      default:
	 // do nothing, the representative is context-dependent
	 break ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double ClusterInfo::similarity(ClusterInfo* other, VectorMeasure<IdxT,ValT>* vm)
{
   if (!other || !vm) return -999.99 ;
   switch (this->repType())
      {
      case ClusterRep::none:
	 break ;
      case ClusterRep::centroid:
      case ClusterRep::medioid:
      case ClusterRep::prototype:
      case ClusterRep::newest:
	 this->setRepresentative(vm) ;
	 other->setRepresentative(vm) ;
	 return vm->similarity(static_cast<Vector<ValT>*>(this->representative()),
	    static_cast<Vector<ValT>*>(other->representative())) ;
      case ClusterRep::average:
         {
	 auto vectors1 = allMembers() ;
	 auto vectors2 = other->allMembers() ;
	 double sum { 0.0 } ;
	 for (auto vec1 : *vectors1)
	    {
	    auto vector1 = static_cast<const Vector<ValT>*>(vec1) ;
	    for (auto vec2 : *vectors2)
	       {
	       auto vector2 = static_cast<const Vector<ValT>*>(vec2) ;
	       sum += vm->similarity(vector1,vector2) ;
	       }
	    }
	 double combinations = vectors1->size() * vectors2->size() ;
	 vectors1->free() ;
	 vectors2->free() ;
	 return sum / combinations ;
	 }
      case ClusterRep::furthest:
         {
	 auto vectors1 = allMembers() ;
	 auto vectors2 = other->allMembers() ;
	 double furthest { HUGE_VAL } ;
	 for (auto vec1 : *vectors1)
	    {
	    auto vector1 = static_cast<const Vector<ValT>*>(vec1) ;
	    for (auto vec2 : *vectors2)
	       {
	       auto vector2 = static_cast<const Vector<ValT>*>(vec2) ;
	       furthest = std::min(furthest,vm->similarity(vector1,vector2)) ;
	       }
	    }
	 vectors1->free() ;
	 vectors2->free() ;
	 return furthest ;
	 }
      case ClusterRep::nearest:
         {
	 auto vectors1 = allMembers() ;
	 auto vectors2 = other->allMembers() ;
	 double nearest { HUGE_VAL } ;
	 for (auto vec1 : *vectors1)
	    {
	    auto vector1 = static_cast<const Vector<ValT>*>(vec1) ;
	    for (auto vec2 : *vectors2)
	       {
	       auto vector2 = static_cast<const Vector<ValT>*>(vec2) ;
	       nearest = std::max(nearest,vm->similarity(vector1,vector2)) ;
	       }
	    }
	 vectors1->free() ;
	 vectors2->free() ;
	 return nearest ;
	 }
      case ClusterRep::rms:
	 break ;
      default:
	 cerr << "missed case" << endl;
      }
   return 0.0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double ClusterInfo::similarity(const Vector<ValT>* other, VectorMeasure<IdxT,ValT>* vm)
{
   if (!other || !vm) return -999.99 ;
   switch (this->repType())
      {
      case ClusterRep::none:
	 break ;
      case ClusterRep::centroid:
      case ClusterRep::medioid:
      case ClusterRep::prototype:
      case ClusterRep::newest:
	 this->setRepresentative(vm) ;
	 return vm->similarity(static_cast<Vector<ValT>*>(this->representative()),other) ;
      case ClusterRep::average:
         {
	 auto vectors = allMembers() ;
	 double avg { 0.0 } ;
	 double combinations { vectors->size() } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    avg += (vm->similarity(vector,other) / combinations) ;
	    }
	 vectors->free() ;
	 return avg ;
	 }
      case ClusterRep::furthest:
         {
	 auto vectors = allMembers() ;
	 double furthest { HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    furthest = std::min(furthest,vm->similarity(vector,other)) ;
	    }
	 vectors->free() ;
	 return furthest ;
	 }
      case ClusterRep::nearest:
         {
	 auto vectors = allMembers() ;
	 double nearest { -HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    nearest = std::max(nearest,vm->similarity(vector,other)) ;
	    }
	 vectors->free() ;
	 return nearest ;
	 }
      case ClusterRep::rms:
	 break ;
      default:
	 cerr << "missed case" << endl;
      }
   return 0.0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double ClusterInfo::reverseSimilarity(const Vector<ValT>* other, VectorMeasure<IdxT,ValT>* vm)
{
   if (!other || !vm) return -999.99 ;
   switch (this->repType())
      {
      case ClusterRep::none:
	 break ;
      case ClusterRep::centroid:
      case ClusterRep::medioid:
      case ClusterRep::prototype:
      case ClusterRep::newest:
	 this->setRepresentative(vm) ;
	 return vm->similarity(other,static_cast<Vector<ValT>*>(this->representative())) ;
      case ClusterRep::average:
         {
	 auto vectors = allMembers() ;
	 double avg { 0.0 } ;
	 double combinations { vectors->size() } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    avg += (vm->similarity(other,vector) / combinations) ;
	    }
	 vectors->free() ;
	 return avg ;
	 }
      case ClusterRep::furthest:
         {
	 auto vectors = allMembers() ;
	 double furthest { HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    furthest = std::min(furthest,vm->similarity(other,vector)) ;
	    }
	 vectors->free() ;
	 return furthest ;
	 }
      case ClusterRep::nearest:
         {
	 auto vectors = allMembers() ;
	 double nearest { -HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    nearest = std::max(nearest,vm->similarity(other,vector)) ;
	    }
	 vectors->free() ;
	 return nearest ;
	 }
      case ClusterRep::rms:
	 break ;
      default:
	 cerr << "missed case" << endl;
      }
   return 0.0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double ClusterInfo::distance(ClusterInfo* other, VectorMeasure<IdxT,ValT>* vm)
{
   if (!other || !vm) return -999.99 ;
   switch (this->repType())
      {
      case ClusterRep::none:
	 break ;
      case ClusterRep::centroid:
      case ClusterRep::medioid:
      case ClusterRep::prototype:
      case ClusterRep::newest:
	 this->setRepresentative(vm) ;
	 other->setRepresentative(vm) ;
	 return vm->distance(static_cast<Vector<ValT>*>(this->representative()),
	    static_cast<Vector<ValT>*>(other->representative())) ;
      case ClusterRep::average:
         {
	 auto vectors1 = allMembers() ;
	 auto vectors2 = other->allMembers() ;
	 double avg { 0.0 } ;
	 double combinations { vectors1->size() * vectors2->size() } ;
	 for (auto vec1 : *vectors1)
	    {
	    auto vector1 = static_cast<const Vector<ValT>*>(vec1) ;
	    for (auto vec2 : *vectors2)
	       {
	       auto vector2 = static_cast<const Vector<ValT>*>(vec2) ;
	       avg += (vm->distance(vector1,vector2) / combinations) ;
	       }
	    }
	 vectors1->free() ;
	 vectors2->free() ;
	 return avg ;
	 }
      case ClusterRep::furthest:
         {
	 auto vectors1 = allMembers() ;
	 auto vectors2 = other->allMembers() ;
	 double furthest { -HUGE_VAL } ;
	 for (auto vec1 : *vectors1)
	    {
	    auto vector1 = static_cast<const Vector<ValT>*>(vec1) ;
	    for (auto vec2 : *vectors2)
	       {
	       auto vector2 = static_cast<const Vector<ValT>*>(vec2) ;
	       furthest = std::max(furthest,vm->distance(vector1,vector2)) ;
	       }
	    }
	 vectors1->free() ;
	 vectors2->free() ;
	 return furthest ;
	 }
      case ClusterRep::nearest:
         {
	 auto vectors1 = allMembers() ;
	 auto vectors2 = other->allMembers() ;
	 double nearest { HUGE_VAL } ;
	 for (auto vec1 : *vectors1)
	    {
	    auto vector1 = static_cast<const Vector<ValT>*>(vec1) ;
	    for (auto vec2 : *vectors2)
	       {
	       auto vector2 = static_cast<const Vector<ValT>*>(vec2) ;
	       nearest = std::min(nearest,vm->distance(vector1,vector2)) ;
	       }
	    }
	 vectors1->free() ;
	 vectors2->free() ;
	 return nearest ;
	 }
      case ClusterRep::rms:
	 break ;
      default:
	 cerr << "missed case" << endl;
      }
   return 0.0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double ClusterInfo::distance(const Vector<ValT>* other, VectorMeasure<IdxT,ValT>* vm)
{
   if (!other || !vm) return -999.99 ;
   switch (this->repType())
      {
      case ClusterRep::none:
	 break ;
      case ClusterRep::centroid:
      case ClusterRep::medioid:
      case ClusterRep::prototype:
      case ClusterRep::newest:
	 this->setRepresentative(vm) ;
	 return vm->distance(static_cast<Vector<ValT>*>(this->representative()),other) ;
      case ClusterRep::average:
         {
	 auto vectors = allMembers() ;
	 double avg { 0.0 } ;
	 double combinations { vectors->size() } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    avg += (vm->distance(vector,other) / combinations) ;
	    }
	 vectors->free() ;
	 return avg ;
	 }
      case ClusterRep::furthest:
         {
	 auto vectors = allMembers() ;
	 double furthest { -HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    furthest = std::max(furthest,vm->distance(vector,other)) ;
	    }
	 vectors->free() ;
	 return furthest ;
	 }
      case ClusterRep::nearest:
         {
	 auto vectors = allMembers() ;
	 double nearest { HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    nearest = std::min(nearest,vm->distance(vector,other)) ;
	    }
	 vectors->free() ;
	 return nearest ;
	 }
      case ClusterRep::rms:
	 break ;
      default:
	 cerr << "missed case" << endl;
      }
   return 0.0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double ClusterInfo::reverseDistance(const Vector<ValT>* other, VectorMeasure<IdxT,ValT>* vm)
{
   if (!other || !vm) return -999.99 ;
   switch (this->repType())
      {
      case ClusterRep::none:
	 break ;
      case ClusterRep::centroid:
      case ClusterRep::medioid:
      case ClusterRep::prototype:
      case ClusterRep::newest:
	 this->setRepresentative(vm) ;
	 return vm->distance(other,static_cast<Vector<ValT>*>(this->representative())) ;
      case ClusterRep::average:
         {
	 auto vectors = allMembers() ;
	 double avg { 0.0 } ;
	 double combinations { vectors->size() } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    avg += (vm->distance(other,vector) / combinations) ;
	    }
	 vectors->free() ;
	 return avg ;
	 }
      case ClusterRep::furthest:
         {
	 auto vectors = allMembers() ;
	 double furthest { -HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    furthest = std::max(furthest,vm->distance(other,vector)) ;
	    }
	 vectors->free() ;
	 return furthest ;
	 }
      case ClusterRep::nearest:
         {
	 auto vectors = allMembers() ;
	 double nearest { HUGE_VAL } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    nearest = std::min(nearest,vm->distance(other,vector)) ;
	    }
	 vectors->free() ;
	 return nearest ;
	 }
      case ClusterRep::rms:
	 break ;
      default:
	 cerr << "missed case" << endl;
      }
   return 0.0 ;
}

/************************************************************************/
/*	methods for class ClusterAlgo					*/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgo<IdxT,ValT>::cluster(ObjectIter& first, ObjectIter& past_end) const
{
   // collect the input vectors into an array
   RefArray* vectors = RefArray::create() ;
   for ( ; first != past_end ; ++first)
      {
      Object* obj = *first ;
      if (obj && obj->isVector())
	 vectors->append(obj) ;
      }
   ClusterInfo* clusters = cluster(vectors) ;
   vectors->free() ;
   return clusters ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgo<IdxT,ValT>::cluster(ArrayIter first, ArrayIter past_end) const
{
   // collect the input vectors into an array
   RefArray* vectors = RefArray::create() ;
   for ( ; first != past_end ; ++first)
      {
      Object* obj = *first ;
      if (obj && obj->isVector())
	 vectors->append(obj) ;
      }
   ClusterInfo* clusters = cluster(vectors) ;
   vectors->free() ;
   return clusters ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool assign_vector_to_nearest_center(size_t index, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto vectors = va_arg(args,const void*) ;
   auto vecarray = reinterpret_cast<const Array*>(vectors) ;
   auto vector = reinterpret_cast<Vector<ValT>*>(vecarray->getNth(index)) ;
   auto centers = va_arg(args,const Array*) ;
   auto measure = va_arg(args,VM*) ;
   auto threshold = va_arg(args,double) ;
   auto changes = va_arg(args,size_t*) ;
   auto prog = va_arg(args,ProgressIndicator*) ;
   if (!vector) return false ;
   auto best_center = ClusteringAlgo<IdxT,ValT>::nearestNeighbor(vector,centers,measure,threshold) ;
   if (best_center)
      {
      // assign cluster to which best_center belongs to vector
      auto old_label = vector->label() ;
      auto new_label = best_center->label() ;
      if (old_label != new_label)
	 {
	 vector->setLabel(new_label) ;
	 (*changes)++ ;
	 }
      }
   prog->incr() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
size_t ClusteringAlgo<IdxT,ValT>::assignToNearest(const Array* vectors, const Array* centers,
   ProgressIndicator* prog, double threshold) const
{
   ThreadPool *tp = ThreadPool::defaultPool() ;
   if (!tp) return false ;
   size_t changes { 0 } ;
   if (tp->parallelize(assign_vector_to_nearest_center<IdxT,ValT>,vectors->size(),vectors,
	 centers,m_measure,threshold,&changes,prog))
      return changes ;
   else
      return 0 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
Vector<ValT>* ClusteringAlgo<IdxT,ValT>::nearestNeighbor(const Vector<ValT>* vector, const Array* centers,
   VectorMeasure<IdxT,ValT>* measure, double threshold)
{
   Vector<ValT>* best_center = nullptr ;
   double best_sim = -999.99 ;
   for (auto cent : *centers)
      {
      if (!cent) continue ;
      auto center = static_cast<Vector<ValT>*>(cent) ;
      double sim = measure->similarity(vector,center) ;
      if (sim >= threshold && sim > best_sim)
	 {
	 best_center = center ;
	 best_sim = sim ;
	 }
      }
   return best_center ;
}

//----------------------------------------------------------------------------

//TODO: can we parallelize this enough that the speedup is worth the effort?
template <typename IdxT, typename ValT>
bool ClusteringAlgo<IdxT,ValT>::extractClusters(const Array* vectors, ClusterInfo**& clusters, size_t& num_clusters,
   RefArray* unassigned) const
{
   clusters = nullptr ;
   // count the number of unique labels on the vectors, and assign each one an index
   ObjCountHashTable label_map ;
   for (auto vec : *vectors)
      {
      if (!vec) continue ;
      auto vector = static_cast<Vector<ValT>*>(vec) ;
      Symbol* label = vector->label() ;
      if (label && !label_map.contains(label))
	 {
	 label_map.add(label,label_map.currentSize()) ;
	 }
      }
   num_clusters = label_map.currentSize() ;
   if (num_clusters)
      {
      clusters = new ClusterInfo*[num_clusters] ;
      for (size_t i = 0 ; i < num_clusters ; ++i)
	 {
	 clusters[i] = ClusterInfo::create() ;
	 }
      // collect the vectors into the appropriate cluster
      for (auto vec : *vectors)
	 {
	 if (!vec) continue ;
	 auto vector = static_cast<Vector<ValT>*>(vec) ;
	 Symbol* label = vector->label() ;
	 if (!label && unassigned)
	    unassigned->append(vector) ;
	 size_t index = label_map.lookup(label) ;
	 clusters[index]->addVector(vector) ;
	 clusters[index]->setLabel(vector->label()) ;
	 }
      }
   return true ;
}

} // end namespace Fr

// end of file cluster.cc //
