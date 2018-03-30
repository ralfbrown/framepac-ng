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

#include "framepac/cluster.h"

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
   auto vector = reinterpret_cast<Vector<ValT>*>(vectors) + index ;
   auto centers = va_arg(args,const Array*) ;
   auto measure = va_args(args,VectorMeasure<IdxT,ValT>*) ;
   auto threshold = va_arg(args,double) ;
   if (!vector) return false ;
   Object* best_center = nullptr ;
   double best_sim = -999.99 ;
   for (size_t i = 0 ; i < centers->size() ; ++i)
      {
      auto center = static_cast<Vector<ValT>*>(centers->getNth(i)) ;
      double sim = measure->similarity(vector,center) ;
      if (sim >= threshold && sim > best_sim)
	 {
	 best_center = center ;
	 best_sim = sim ;
	 }
      }
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
bool ClusteringAlgo<IdxT,ValT>::extractClusters(Array& vectors, ClusterInfo*& clusters, size_t& num_clusters,
   RefArray* unassigned) const
{
   clusters = nullptr ;
   num_clusters = 0 ;
   // count the number of unique labels on the vectors
   for (size_t i = 0 ; i < vectors.size() ; ++i)
      {
//TODO
      }
   if (num_clusters > 0)
      {
      clusters = new ClusterInfo*[num_clusters] ;
      // collect the vectors into the appropriate cluster
//TODO
      }
   return true ;
}

} // end namespace Fr

// end of file cluster.cc //
