/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-23					*/
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
   for (size_t i = 0 ; i < members()->size() ; i++)
      {
      auto vec = static_cast<SparseVector<IdxT,ValT>*>(members()->getNth(i)) ;
      if (!vec) continue ;
      centroid->incr(vec) ;
      }
   centroid->setLabel(this->label()) ;
   return centroid ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* ClusterInfo::createSparseCentroid(const Array* vectors) const
{
   auto centroid = SparseVector<IdxT,ValT>::create() ;
   for (size_t i = 0 ; i < vectors->size() ; i++)
      {
      auto vec = static_cast<SparseVector<IdxT,ValT>*>(vectors->getNth(i)) ;
      if (!vec) continue ;
      centroid->incr(vec) ;
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
void ClusterInfo::setRepresentative()
{
   if (!representative())
      {
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
	 case ClusterRep::medioid:
	    //TODO
	    break ;
	 case ClusterRep::prototype:
	    if (this->members())
	       {
	       auto rep = this->members()->getNth(0) ;
	       this->m_rep = rep ? rep->clone().move() : nullptr ;
	       }
	    else if (this->subclusters())
	       {
	       auto sub = static_cast<ClusterInfo*>(this->subclusters()->getNth(0)) ;
	       if (sub) sub->setRepresentative<IdxT,ValT>() ;
	       auto rep = sub->representative() ;
	       this->m_rep = rep? rep->clone().move() : nullptr ;
	       }
	    break ;
	 default:
	    // do nothing, the representative is context-dependent
	    break ;
	 }
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
	 this->setRepresentative<IdxT,ValT>() ;
	 other->setRepresentative<IdxT,ValT>() ;
	 return vm->similarity(static_cast<Vector<ValT>*>(this->representative()),
	    static_cast<Vector<ValT>*>(other->representative())) ;
      case ClusterRep::average:
	 //TODO
	 break ;
      case ClusterRep::furthest:
	 break ;
      case ClusterRep::nearest:
	 break ;
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
   for (size_t i = 0 ; i < centers->size() ; ++i)
      {
      auto center = static_cast<Vector<ValT>*>(centers->getNth(i)) ;
      if (!center) continue ;
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
   for (size_t i = 0 ; i < vectors->size() ; ++i)
      {
      auto vector = static_cast<Vector<ValT>*>(vectors->getNth(i)) ;
      if (!vector) continue ;
      Symbol* label = vector->label() ;
      if (!label) continue ;
      if (!label_map.contains(label))
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
      for (size_t i = 0 ; i < vectors->size() ; ++i)
	 {
	 auto vector = static_cast<Vector<ValT>*>(vectors->getNth(i)) ;
	 if (!vector) continue ;
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
