/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-03-30					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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
#include "framepac/threadpool.h"

using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
class ClusteringAlgoKMeans : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoKMeans() { delete this ; }

      virtual ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) ;

      void useMedioids() { m_use_medioids = true ; }

      // accessors
      bool usingMedioids() const { return m_use_medioids ; }
      size_t desiredClusters() const { return m_desired_clusters ; }
      size_t iterations() const { return m_iterations ; }
   protected:
      size_t m_desired_clusters ;
      size_t m_iterations ;
      bool   m_use_medioids { false } ;
   } ;

template <typename IdxT, typename ValT>
class ClusteringAlgoKMedioids : public ClusteringAlgoKMeans<IdxT,ValT>
   {
   public:
      ClusteringAlgoKMedioids() : ClusteringAlgoKMeans<IdxT,ValT>()
	 {
	    this->useMedioids() ;
	    return ;
	 }
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
bool update_centroid(const void* o, size_t id, va_list args)
{
   auto inf = reinterpret_cast<const ClusterInfo*>(o) + id ;
   auto centers = va_arg(args,RefArray*) ;
   int sparse = va_arg(args,int) ;
   if (sparse)
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createSparseCentroid<IdxT,ValT>() ;
      // make the centroid the new center for the cluster
      centers->setNth(id,centroid) ;
      }
   else
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createDenseCentroid<ValT>() ;
      // make the centroid the new center for the cluster
      centers->setNth(id,centroid) ;
      }
   return true ;			// no errors, safe to continue processing
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool update_medioid(const void* o, size_t id, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto inf = reinterpret_cast<const ClusterInfo*>(o) + id ;
   auto centers = va_arg(args,RefArray*) ;
   int sparse = va_arg(args,int) ;
   auto measure = va_arg(args,VM*) ;
   if (sparse)
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createSparseCentroid<IdxT,ValT>() ;
      auto medioid = ClusteringAlgo<IdxT,ValT>::nearestNeighbor(centroid,inf->members(),measure) ;
      // make the medioid the new center for the cluster
      centers->setNth(id,medioid->clone()) ;
      }
   else
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createDenseCentroid<ValT>() ;
      // find nearest original vector in cluster
      auto medioid = ClusteringAlgo<IdxT,ValT>::nearestNeighbor(centroid,inf->members(),measure) ;
      // make the medioid the new center for the cluster
      centers->setNth(id,medioid->clone()) ;
      }
   return true ;			// no errors, safe to continue processing
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoKMeans<IdxT,ValT>::cluster(ObjectIter& first, ObjectIter& past_end)
{
   // collect the input vectors into an array
   RefArray* vectors = RefArray::create() ;
   for ( ; first != past_end ; ++first)
      {
      Object* obj = *first ;
      if (obj && obj->isVector())
	 vectors->append(obj) ;
      }
   if (!this->checkSparseOrDense(vectors))
      {
      vectors->free() ;
      return nullptr ;			// vectors must be all dense or all sparse
      }
#if 0
   //TODO: select K vectors which are (approximately) maximally separated
   RefArray* centers = RefArray::create() ;
   RefArray* sample = vectors->randomSample(2*desiredClusters()) ;

   sample->free() ;
#else
//for now, we'll just randomly select K vectors
   RefArray* centers = vectors->randomSample(desiredClusters()) ;
#endif
   // until converged or iteration limit:
   //    assign each vector to the nearest center
   //    collect vectors into clusters by assigned center
   //    generate a new centroid for each cluster
   //    make the centroids the new centers
   size_t iteration ;
   ClusterInfo** clusters(nullptr) ;
   size_t num_clusters(0) ;
   ThreadPool* tp = ThreadPool::defaultPool() ;
   for (iteration = 1 ; iteration <= iterations() ; iteration++)
      {
      bool changes = this->assignToNearest(vectors, centers) ;
      this->extractClusters(vectors,clusters,num_clusters) ;
      if (!changes)
	 break ;			// we've converged!
      if (iteration != 1)
	 centers->clearArray(true) ;
      auto fn = update_centroid<IdxT,ValT> ;
      if (usingMedioids())
	 fn = update_medioid<IdxT,ValT> ;
      tp->parallelize(fn,num_clusters,clusters,&centers,this->usingSparseVectors(),this->m_measure) ;
      this->freeClusters(clusters,num_clusters) ;
      }
   // build the final cluster result from the extracted clusters
//TODO
   vectors->free() ;
   centers->free() ;
   return nullptr ;
}

} // end of namespace Fr

// end of file cluster_kmeans.C //
