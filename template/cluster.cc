/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-16					*/
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
break;
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
void ClusterInfo::updateRepresentative(Vector<ValT>* vector, VectorMeasure<IdxT,ValT>* vm)
{
   if (!vector)
      return ;
   if (this->repType() == ClusterRep::centroid)
      {
      if (representative())
	 {
	 // add the new vector to the existing centroid
	 m_rep = static_cast<Vector<ValT>*>(m_rep)->incr(vector) ;
	 }
      else // the new vector *is* the centroid
	 this->m_rep = vector->clone().move() ;
      }
   else if (this->repType() == ClusterRep::newest)
      {
      this->m_rep = vector ;
      }
   else if (this->repType() == ClusterRep::prototype && !representative())
      {
      this->m_rep = vector ;
      }
   else if (this->repType() == ClusterRep::medioid)
      {
      //TODO
      (void)vm;
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
	    this->m_rep.release() ;
	    this->m_rep = const_cast<Object*>(sub->representative()) ;
	    }
	 break ;
      case ClusterRep::newest:
	 if (this->members() && this->members()->size() > 0)
	    {
	    this->m_rep.release() ;
	    this->m_rep = this->members()->getNth(this->members()->size()-1) ;
	    }
	 else if (this->subclusters() && this->subclusters()->size() > 0)
	    {
	    auto sub = static_cast<ClusterInfo*>(this->subclusters()->getNth(this->subclusters()->size()-1)) ;
	    if (sub) sub->setRepresentative(vm) ;
	    this->m_rep.release() ;
	    this->m_rep = const_cast<Object*>(sub->representative()) ;
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
	 Vector<ValT>* medioid = nullptr ;
	 for (auto vec : *vectors)
	    {
	    Vector<ValT>* vector = static_cast<Vector<ValT>*>(vec) ;
	    double sim = vm->similarity(vector,centroid) ;
	    if (sim > best_sim)
	       {
	       best_sim = sim ;
	       medioid = vector ;
	       }
	    }
	 this->m_rep.release() ;
	 this->m_rep.acquire(medioid) ;
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
	 return vm->similarity(static_cast<const Vector<ValT>*>(this->representative()),
	    static_cast<const Vector<ValT>*>(other->representative())) ;
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
	 return vm->similarity(static_cast<const Vector<ValT>*>(this->representative()),other) ;
      case ClusterRep::average:
         {
	 auto vectors = allMembers() ;
	 double avg { 0.0 } ;
	 auto combinations { vectors->size() } ;
	 for (auto vec : *vectors)
	    {
	    auto vector = static_cast<const Vector<ValT>*>(vec) ;
	    avg += (vm->similarity(vector,other) / combinations) ;
	    }
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
   ScopedObject<RefArray> vectors ;
   for ( ; first != past_end ; ++first)
      {
      Object* obj = *first ;
      if (obj && obj->isVector())
	 vectors->append(obj) ;
      }
   return cluster(vectors) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgo<IdxT,ValT>::cluster(ArrayIter first, ArrayIter past_end) const
{
   // collect the input vectors into an array
   ScopedObject<RefArray> vectors ;
   for ( ; first != past_end ; ++first)
      {
      Object* obj = *first ;
      if (obj && obj->isVector())
	 vectors->append(obj) ;
      }
   return cluster(vectors) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool ClusteringAlgo<IdxT,ValT>::separateSeeds(const Array* vectors, RefArray*& seed, RefArray*& nonseed) const
{
   if (!vectors || vectors->size() == 0 || !this->checkSparseOrDense(vectors))
      return false ;			// vectors must all be dense or all sparse
   this->log(1,"Separating seed vectors from non-seed vectors") ;
   seed = RefArray::create() ;		// vectors with a cluster assignment at the outset
   nonseed = RefArray::create() ;	// vectors which need to be given a cluster assignment
   for (auto obj : *vectors)
      {
      if (!obj || !obj->isVector())
	 continue ;
      Vector<ValT>* vec = static_cast<Vector<ValT>*>(obj) ;
      if (vec->label())
	 seed->append(vec) ;
      else
	 nonseed->append(vec) ;
      }
   this->log(1,"  %lu seed vectors and %lu non-seed vectors found",seed->size(),nonseed->size()) ;
   return true ;
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
   if (prog) prog->incr() ;
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
bool compute_similarity(size_t index, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto clusters = va_arg(args,const Array*) ;
   auto vector = va_arg(args,const Vector<ValT>*) ;
   auto scores = va_arg(args,double*) ;
   auto measure = va_arg(args,VM*) ;
   auto prog = va_arg(args,ProgressIndicator*) ;
   auto cluster = static_cast<ClusterInfo*>(clusters->getNth(index)) ;
   scores[index] = cluster->similarity(vector,measure) ;
   if (prog) prog->incr() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
double ClusteringAlgo<IdxT,ValT>::findNearestCluster(const Array* clusters, const Vector<ValT>* vector,
   size_t& best_cluster, ProgressIndicator* prog) const
{
   best_cluster = ~0 ;
   ThreadPool *tp = ThreadPool::defaultPool() ;
   if (!tp || !clusters || clusters->size() == 0 || !vector)
      {
      return -HUGE_VAL ;
      }
   size_t num_clusters { clusters->size() } ;
   LocalAlloc<double> scores(num_clusters) ;
   double best_score = -HUGE_VAL ;
   if (tp->parallelize(compute_similarity<IdxT,ValT>,num_clusters,clusters,vector,&scores,m_measure,prog))
      {
      // scan through the list of scores for the best
      for (size_t i = 0 ; i < num_clusters ; ++i)
	 {
	 if (scores[i] > best_score)
	    {
	    best_cluster = i ;
	    best_score = scores[i] ;
	    }
	 }
      }
   return best_score ;
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
   // count the number of unique labels on the vectors, and assign each one an index
   ScopedObject<ObjCountHashTable> label_map ;
   for (auto vec : *vectors)
      {
      if (!vec) continue ;
      auto vector = static_cast<Vector<ValT>*>(vec) ;
      Symbol* label = vector->label() ;
      if (label && !label_map->contains(label))
	 {
	 label_map->add(label,label_map->currentSize()) ;
	 }
      }
   num_clusters = label_map->currentSize() ;
   this->log(2,"   extracting %lu clusters",num_clusters) ;
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
	 {
	 unassigned->append(vector) ;
	 }
      else
	 {
	 size_t index = label_map->lookup(label) ;
	 clusters[index]->addVector(vector) ;
	 clusters[index]->setLabel(vector->label()) ;
	 }
      }
   return true ;
}

} // end namespace Fr

// end of file cluster.cc //
