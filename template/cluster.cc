/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-30					*/
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
      centroid->add(vec) ;
      }
   return centroid ;
}

//----------------------------------------------------------------------------

template <typename ValT>
DenseVector<ValT>* ClusterInfo::createDenseCentroid() const
{
   auto centroid = DenseVector<ValT>::create() ;
   for (size_t i = 0 ; i < members()->size() ; i++)
      {
      auto vec = static_cast<DenseVector<ValT>*>(members()->getNth(i)) ;
      if (!vec) continue ;
      centroid->add(vec) ;
      }
   return centroid ;
}

/************************************************************************/
/*	methods for class ClusterAlgo					*/
/************************************************************************/

template <typename IdxT, typename ValT>
bool assign_vector_to_nearest_center(const void* vectors, size_t index, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto vector = reinterpret_cast<Vector<ValT>*>(vectors) + index ;
   auto centers = va_arg(args,const Array*) ;
   auto measure = va_arg(args,VM*) ;
   auto threshold = va_arg(args,double) ;
   if (!vector) return false ;
   auto best_center = ClusteringAlgo<IdxT,ValT>::nearestNeighbor(vector,centers,measure,threshold) ;
   if (best_center)
      {
      //TODO: assign cluster to which best_center belongs to vector
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool ClusteringAlgo<IdxT,ValT>::assignToNearest(Array& vectors, const Array& centers, double threshold) const
{
   ThreadPool *tp = ThreadPool::defaultPool() ;
   if (!tp) return false ;
   return tp->parallelize(assign_vector_to_nearest_center<IdxT,ValT>,vectors,&centers,m_measure,threshold) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
Vector<ValT>* ClusteringAlgo<IdxT,ValT>::nearestNeighbor(const Vector<ValT>* vector, const Array& centers,
   VectorMeasure<IdxT,ValT>* measure, double threshold)
{
   Vector<ValT>* best_center = nullptr ;
   double best_sim = -999.99 ;
   for (size_t i = 0 ; i < centers.size() ; ++i)
      {
      auto center = static_cast<Vector<ValT>*>(centers.getNth(i)) ;
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
bool ClusteringAlgo<IdxT,ValT>::extractClusters(Array& vectors, ClusterInfo*& clusters, size_t& num_clusters,
   RefArray* unassigned) const
{
   clusters = nullptr ;
   // count the number of unique labels on the vectors, and assign each one an index
   ObjCountHashTable label_map ;
   for (size_t i = 0 ; i < vectors.size() ; ++i)
      {
      auto vector = static_cast<Vector<ValT>*>(vectors.getNth(i)) ;
      if (!vector) continue ;
      Symbol* label = vector->label() ;
      if (!label) continue ;
      if (!label_map.contains(label))
	 {
	 label_map.add(label,label_map.size()) ;
	 }
      }
   num_clusters = label_map.size() ;
   if (num_clusters)
      {
      clusters = new ClusterInfo*[num_clusters] ;
      // collect the vectors into the appropriate cluster
      for (size_t i = 0 ; i < vectors.size() ; ++i)
	 {
	 auto vector = static_cast<Vector<ValT>*>(vectors.getNth(i)) ;
	 if (!vector) continue ;
	 Symbol* label = vector->label() ;
	 if (!label && unassigned)
	    unassigned->append(vector) ;
	 size_t index = label_map.lookup(label) ;
	 clusters[index].addVector(vector) ;
	 }
      }
   return true ;
}

} // end namespace Fr

// end of file cluster.cc //
