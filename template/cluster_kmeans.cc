/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-15					*/
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
      virtual ~ClusteringAlgoKMeans() {}
      virtual const char*algorithmName() const { return "K-Means" ; }

      virtual ClusterInfo* cluster(const Array* vectors) const ;

      void useMedioids() { m_use_medioids = true ; }

      // accessors
      bool usingMedioids() const { return m_use_medioids ; }

   protected: // data members
      bool   m_use_medioids { false } ;
      bool   m_fast_init { false } ;
   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class ClusteringAlgoKMedioids : public ClusteringAlgoKMeans<IdxT,ValT>
   {
   public:
      ClusteringAlgoKMedioids() : ClusteringAlgoKMeans<IdxT,ValT>()
	 {
	    this->useMedioids() ;
	    return ;
	 }
      virtual const char*algorithmName() const { return "K-Medioids" ; }
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
static bool update_centroid(size_t id, va_list args)
{
   auto o = va_arg(args,const ClusterInfo**) ;
   auto inf = o[id] ;
   auto centers = va_arg(args,RefArray*) ;
   int sparse = va_arg(args,int) ;
   if (sparse)
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createSparseCentroid<IdxT,ValT>() ;
      // make the centroid the new center for the cluster
      centers->setNthNoCopy(id,centroid) ;
      }
   else
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createDenseCentroid<ValT>() ;
      // make the centroid the new center for the cluster
      centers->setNthNoCopy(id,centroid) ;
      }
   return true ;			// no errors, safe to continue processing
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
static bool update_medioid(size_t id, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto o = va_arg(args,const ClusterInfo**) ;
   auto inf = o[id] ;
   auto centers = va_arg(args,RefArray*) ;
   int sparse = va_arg(args,int) ;
   auto measure = va_arg(args,VM*) ;
   if (sparse)
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createSparseCentroid<IdxT,ValT>() ;
      auto medioid = ClusteringAlgo<IdxT,ValT>::nearestNeighbor(centroid,inf->members(),measure) ;
      // make the medioid the new center for the cluster
      centers->setNth(id,medioid) ;
      centroid->free() ;
      }
   else
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createDenseCentroid<ValT>() ;
      // find nearest original vector in cluster
      auto medioid = ClusteringAlgo<IdxT,ValT>::nearestNeighbor(centroid,inf->members(),measure) ;
      // make the medioid the new center for the cluster
      centers->setNth(id,medioid) ;
      centroid->free() ;
      }
   return true ;			// no errors, safe to continue processing
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
static size_t find_least_similar(const Array* vectors, const Array* refs, VectorMeasure<IdxT,ValT>* vm)
{
   size_t selected = (size_t)~0 ;
   double best_sim = 999.99 ;
   for (size_t i = 0 ; i < vectors->size() ; ++i)
      {
      auto vector = static_cast<Vector<ValT>*>(vectors->getNth(i)) ;
      if (!vector) continue ;
      double sim = -999.99 ;
      for (auto ref : *refs)
	 {
	 sim = std::max(sim,vm->similarity(vector,static_cast<Vector<ValT>*>(ref))) ;
	 }
      if (sim < best_sim)
	 {
	 best_sim = sim ;
	 selected = i ;
	 }
      }
   return selected ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoKMeans<IdxT,ValT>::cluster(const Array* vectors) const
{
   if (!vectors || vectors->size() == 0 || !this->checkSparseOrDense(vectors))
      {
      return nullptr ;			// vectors must be all dense or all sparse
      }
   bool using_sparse_vectors = vectors->getNth(0)->isSparseVector() ;
   if (!this->m_measure)
      {
      return nullptr ;			// we need a similarity measure
      }
   // trap signals to allow graceful early termination
   this->trapSigInt() ;
   RefArray* centers ;
   size_t num_clusters = this->desiredClusters() ;
   this->log(0,"Initializing %lu centers",num_clusters) ;
   if (this->m_fast_init)
      {
      // do a quick and dirty init -- just randomly select K vectors
      this->log(1,"Selecting random sample of vectors as centers") ;
      centers = vectors->randomSample(num_clusters) ;
      }
   else
      {
      // select K vectors which are (approximately) maximally separated
      centers = RefArray::create() ;
      RefArray* sample = vectors->randomSample(2*num_clusters) ;
      // start by arbitrarily picking the first vector in the sample
      centers->append(sample->getNth(0)) ;
      sample->setNth(0,nullptr) ;
      // until we've accumulated desiredClusters() vectors, search for
      //   the as-yet-unselected vector with the smallest maximal
      //   similarity to any already-selected vector
      for (size_t i = 1 ; i < num_clusters ; ++i)
	 {
	 size_t selected = find_least_similar<IdxT,ValT>(sample, centers,this->m_measure) ;
	 centers->append(sample->getNth(selected)) ;
	 sample->setNth(selected,nullptr) ;
	 }
      sample->free() ;
      }
   // assign a label to each of the selected centers
   for (auto v : *centers)
      {
      if (v)
	 static_cast<Vector<ValT>*>(v)->setLabel(ClusterInfo::genLabel()) ;
      }
   // until converged or iteration limit:
   //    assign each vector to the nearest center
   //    collect vectors into clusters by assigned center
   //    generate a new centroid for each cluster
   //    make the centroids the new centers
   size_t iteration ;
   ClusterInfo** clusters(nullptr) ;
   num_clusters = 0 ;
   ThreadPool* tp = ThreadPool::defaultPool() ;
   for (iteration = 1 ; iteration <= this->maxIterations() && !this->abortRequested() ; iteration++)
      {
      this->log(0,"Iteration %lu",iteration) ;
      auto prog = this->makeProgressIndicator(vectors->size()) ;
      size_t changes = this->assignToNearest(vectors, centers, prog) ;
      delete prog ;
      this->log(0,"  %lu vectors changed cluster",changes) ;
      this->extractClusters(vectors,clusters,num_clusters) ;
      if (!changes)
	 break ;			// we've converged!
      if (iteration != 1)
	 centers->clearArray(true) ;
      auto fn = update_centroid<IdxT,ValT> ;
      if (usingMedioids())
	 fn = update_medioid<IdxT,ValT> ;
      this->log(1,"  updating centers") ;
      tp->parallelize(fn,num_clusters,clusters,centers,using_sparse_vectors,this->m_measure) ;
      this->freeClusters(clusters,num_clusters) ;
      }
   // build the final cluster result from the extracted clusters
   ClusterInfo* result_clusters = ClusterInfo::create(clusters,num_clusters) ;
   this->freeClusters(clusters,num_clusters) ;
   // the subclusters are the actual result
   result_clusters->setFlag(ClusterInfo::Flags::group) ;
   centers->free() ;
   // cleanup: untrap signals
   this->untrapSigInt() ;
   return result_clusters ;
}

} // end of namespace Fr

// end of file cluster_kmeans.C //
