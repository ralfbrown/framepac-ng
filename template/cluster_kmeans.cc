/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-26					*/
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

   protected:
      void clearCenters(Array&) const ;

   protected: // data members
      bool   m_use_medioids { false } ;
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
   auto centers = va_arg(args,Array*) ;
   auto sparse = va_arg(args,int) ;
   auto prog = va_arg(args,ProgressIndicator*) ;
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
   if (prog)
      ++(*prog) ;
   return true ;			// no errors, safe to continue processing
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
static bool update_medioid(size_t id, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto o = va_arg(args,const ClusterInfo**) ;
   auto inf = o[id] ;
   auto centers = va_arg(args,Array*) ;
   auto sparse = va_arg(args,int) ;
   auto measure = va_arg(args,VM*) ;
   auto prog = va_arg(args,ProgressIndicator*) ;
   Ptr<Vector<ValT>> centroid ;
   if (sparse)
      {
      // create a centroid of the members of the current cluster
      centroid =  inf->createSparseCentroid<IdxT,ValT>() ;
      }
   else
      {
      // create a centroid of the members of the current cluster
      centroid = inf->createDenseCentroid<ValT>() ;
      }
   // find nearest original vector in cluster
   auto medioid = ClusteringAlgo<IdxT,ValT>::nearestNeighbor(centroid,inf->members(),measure) ;
   // make the medioid the new center for the cluster
   centers->setNthNoCopy(id,medioid) ;
   if (prog)
      ++(*prog) ;
   return true ;			// no errors, safe to continue processing
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
static void find_least_most_similar(const Array* vectors, const Array* refs, VectorMeasure<IdxT,ValT>* vm,
   size_t& least_similar, size_t& most_similar)
{
   size_t selected = (size_t)~0 ;
   size_t discarded = (size_t)~0 ;
   double best_sim = -HUGE_VAL ;
   double worst_sim = HUGE_VAL ;
   //TODO: parallelize this loop
   for (size_t i = 0 ; i < vectors->size() ; ++i)
      {
      auto vector = static_cast<Vector<ValT>*>(vectors->getNth(i)) ;
      if (!vector || vector->length() == 0.0) continue ;
      double sim = -999.99 ;
      for (auto ref : *refs)
	 {
	 sim = std::max(sim,vm->similarity(vector,static_cast<Vector<ValT>*>(ref))) ;
	 }
      if (sim > best_sim)
	 {
	 best_sim = sim ;
	 selected = i ;
	 }
      if (sim < worst_sim)
	 {
	 worst_sim = sim ;
	 discarded = i ;
	 }
      }
   least_similar = discarded ;
   most_similar = selected ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
void ClusteringAlgoKMeans<IdxT,ValT>::clearCenters(Array& centers) const
{
   if (usingMedioids())
      {
      // clear the pointers in the 'centers' array so that they don't get freed when the array is freed,
      //   since they are just references to vectors which are still in use
      for (size_t i = 0 ; i < centers.size() ; ++i)
	 centers.clearNth(i) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoKMeans<IdxT,ValT>::cluster(const Array* vectors) const
{
   if (!vectors || vectors->size() == 0 || !this->checkSparseOrDense(vectors))
      {
      return nullptr ;			// vectors must be all dense or all sparse
      }
   ScopedObject<RefArray> nonempty(vectors->size()) ;
   for (auto v : *vectors)
      {
      auto vec = static_cast<Vector<ValT>*>(v) ;
      if (vec && vec->length() > 0.0)
	 nonempty->append(vec) ;
      }
   if (nonempty->size() == 0)
      {
      return nullptr ;			// nothing to be clustered
      }
   bool using_sparse_vectors = nonempty->getNth(0)->isSparseVector() ;
   if (!this->m_measure)
      {
      return nullptr ;			// we need a similarity measure
      }
   // trap signals to allow graceful early termination
   this->trapSigInt() ;
   size_t num_clusters = this->desiredClusters() ;
   ScopedObject<Array> centers(num_clusters) ;
   this->log(0,"Initializing %lu centers",num_clusters) ;
   if (this->m_fast_init)
      {
      // do a quick and dirty init -- just randomly select K vectors
      this->log(1,"Selecting random sample of vectors as centers") ;
      Ptr<RefArray> sample{ nonempty->randomSample(num_clusters) } ;
      for (auto v : *sample)
	 {
	 auto vec = static_cast<Vector<ValT>*>(v) ;
	 if (vec->length() == 0)
	    continue ;			// ignore empty vectors
	 centers->append(v) ;		// make a copy of the vector as the initial center
	 }
      }
   else
      {
      // select K vectors which are (approximately) maximally separated
      Ptr<RefArray> sample { nonempty->randomSample(2*num_clusters+1) } ;
      // start by arbitrarily picking the first vector in the sample
      Vector<ValT>* vec = static_cast<Vector<ValT>*>(sample->getNth(0)) ;
      this->log(2,"center: %s",*vec->cString()) ;
      centers->append(sample->getNth(0)) ;
      sample->clearNth(0) ;
      auto prog = this->makeProgressIndicator(num_clusters) ;
      ++(*prog) ;
      // until we've accumulated desiredClusters() vectors, search for
      //   the as-yet-unselected vector with the smallest maximal
      //   similarity to any already-selected vector
      for (size_t i = 1 ; i < num_clusters ; ++i)
	 {
	 size_t selected, discarded ;
	 find_least_most_similar<IdxT,ValT>(sample, centers,this->m_measure,selected,discarded) ;
	 Vector<ValT>* v = static_cast<Vector<ValT>*>(sample->getNth(selected)) ;
	 this->log(2,"center: %s",*v->cString()) ;
	 centers->append(sample->getNth(selected)) ;
	 sample->clearNth(selected) ;
	 // also zap the most similar, to speed up the search
	 sample->clearNth(discarded) ;
	 ++(*prog) ;
	 }
      delete prog ;
      }
   // assign a label to each of the selected centers
   for (auto v : *centers)
      {
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
      auto prog = this->makeProgressIndicator(nonempty->size()) ;
      size_t changes = this->assignToNearest(nonempty, centers, prog) ;
      delete prog ;
      this->log(0,"  %lu vectors changed cluster",changes) ;
      this->freeClusters(clusters,num_clusters) ;
      this->extractClusters(nonempty,clusters,num_clusters) ;
      if (!changes)
	 break ;			// we've converged!
      auto fn = update_centroid<IdxT,ValT> ;
      if (usingMedioids())
	 fn = update_medioid<IdxT,ValT> ;
      this->log(1,"  updating centers") ;
      clearCenters(*centers) ;
      centers = Array::create(num_clusters) ;
      prog = (nonempty->size() > 1000) ? this->makeProgressIndicator(num_clusters) : nullptr ;
      tp->parallelize(fn,num_clusters,clusters,(Array*)centers,using_sparse_vectors,this->m_measure,prog) ;
      delete prog ;
      }
   // build the final cluster result from the extracted clusters
   ClusterInfo* result_clusters = ClusterInfo::create(clusters,num_clusters) ;
   clearCenters(*centers) ;
   this->freeClusters(clusters,num_clusters) ;
   // the subclusters are the actual result
   result_clusters->setFlag(ClusterInfo::Flags::group) ;
   // cleanup: untrap signals
   this->untrapSigInt() ;
   return result_clusters ;
}

} // end of namespace Fr

// end of file cluster_kmeans.C //
