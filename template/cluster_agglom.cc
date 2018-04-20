/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-18					*/
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
using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

// the Agglomerative method from the first-gen FramepaC is basically Brown clustering with early exit where
//   the resulting clusters are flattened, so we'll implement Brown clustering with an early-exit option and
//   make Agglomerative a subclass of Brown

template <typename IdxT, typename ValT>
class ClusteringAlgoBrown : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      ClusteringAlgoBrown() { this->desiredClusters(1) ; }
      virtual ~ClusteringAlgoBrown() { delete this ; }
      virtual const char*algorithmName() const { return "Brown" ; }

      virtual ClusterInfo* cluster(const Array* vectors) const ;

   protected:
      bool   m_flatten { false } ;
   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class ClusteringAlgoAgglom : public ClusteringAlgoBrown<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoAgglom() { delete this ; }
      virtual const char*algorithmName() const { return "Agglomerative" ; }

      //virtual ClusterInfo* cluster(const Array* vectors) const ;  // inherited from Brown clustering

   protected:

   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
bool agglom_clustering_best_similarity(size_t index, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto clusters = va_arg(args,const Array*) ;
   double* similarity = va_arg(args,double*) + index ;
   size_t* neighbor = va_arg(args,size_t*) + index ;
   VM* measure = va_arg(args,VM*) ;
   ProgressIndicator* prog = va_arg(args,ProgressIndicator*) ;
   if (!measure) return false ;
   auto cluster = static_cast<ClusterInfo*>(clusters->getNth(index))->members() ;
   if (!cluster || cluster->size() != 1)
      return false ;
   double best_sim = -999.99 ;
   size_t best_neighbor = ~0 ;
   auto vec1 = static_cast<Vector<ValT>*>(cluster->front()) ;
   for (size_t i = 0 ; i < clusters->size() ; ++i)
      {
      if (i == index) continue ;
      auto other_clus = static_cast<ClusterInfo*>(clusters->getNth(i))->members() ;
      if (!other_clus || other_clus->size() != 1)
	 continue ;
      auto vec2 = static_cast<Vector<ValT>*>(other_clus->front()) ;
      double sim = measure->similarity(vec1,vec2) ;
      if (sim > best_sim)
	 {
	 best_sim = sim ;
	 best_neighbor = i ;
	 }
      }
   *similarity = best_sim ;
   *neighbor = best_neighbor ;
   prog->incr() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
bool update_nearest_neighbors(size_t index, va_list args)
{
   typedef VectorMeasure<IdxT,ValT> VM ;
   auto clusters = va_arg(args,const Array*) ;
   double* similarity = va_arg(args,double*) + index ;
   size_t* neighbor = va_arg(args,size_t*) + index ;
   VM* measure = va_arg(args,VM*) ;
   size_t clus1 = va_arg(args,size_t) ;
   size_t clus2 = va_arg(args,size_t) ;
   (void)similarity; (void)measure; //TODO
   if (*neighbor == clus1 || *neighbor == clus2)
      {
      // the nearest neighbor was one of the two merged clusters, so we need to check whether the merged result
      //   is closer; if not, we must recompute all similarities
      //TODO
      }
   else if (index == clus1)
      {
      // the resulting merged cluster requires recomputing similarities to all other clusters
      for (size_t i = 0 ; i < clusters->size() ; ++i)
	 {
	 if (i == index) continue ;
	 //TODO
	 }
      }
   else
      {
      // account for the merged-away cluster
      if (*neighbor > clus2) (*neighbor)-- ;
      // compare the similarity with the merged cluster against the previous best, and use the better of the two
      //TODO
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoBrown<IdxT,ValT>::cluster(const Array* vectors) const
{
   if (!vectors || vectors->size() == 0)
      return ClusterInfo::create() ;
   size_t num_vectors = vectors->size() ;
   ClusterInfo* clusters = ClusterInfo::createSingletonClusters(vectors) ;
   if (clusters->numSubclusters() <= this->desiredClusters())
      {
      this->log(0,"Nothing to be clustered - want %lu clusters, have only %lu vectors",
	 this->desiredClusters(),clusters->numSubclusters()) ;
      return clusters ;
      }
   this->log(0,"Starting %s clustering using %s measure; %lu vectors to cluster",
      this->algorithmName(),this->measureName(),num_vectors) ;
   // until we've reached the desired number of clusters or the best similarity is below the threshold:
   //   merge the two most similar clusters
   auto prog = this->makeProgressIndicator(num_vectors - this->desiredClusters()) ;
   auto tp = ThreadPool::defaultPool() ;
   double* similarities = new double[num_vectors] ;
   size_t* neighbors = new size_t[num_vectors] ;
   this->log(0,"Computed nearest neighbor for each vector") ;
   tp->parallelize(agglom_clustering_best_similarity<IdxT,ValT>,num_vectors,clusters->subclusters(),
      similarities,neighbors,this->m_measure,prog) ;
   delete prog ;
   prog = this->makeProgressIndicator(num_vectors - this->desiredClusters()) ;
   for ( ; ; )
      {
      auto numclus = clusters->numSubclusters() ;
      double best_sim = similarities[0] ;
      size_t best_clus = 0 ;
      size_t best_neighbor = neighbors[0] ;
      for (size_t i = 1 ; i < numclus ; ++i)
	 {
	 if (similarities[i] > best_sim)
	    {
	    best_sim = similarities[i] ;
	    best_clus = i ;
	    best_neighbor = neighbors[i] ;
	    }
	 }
      if (best_sim < this->clusterThreshold())
	 {
	 this->log(0,"  terminating: best similarity %g is less than threshold %g",best_sim,this->clusterThreshold());
	 break ;
	 }
      // merge the two clusters we found to be nearest neighbors
      this->log(2,"  merging clusters %lu and %lu (similarity %g)",best_clus,best_neighbor,best_sim) ;
      if (best_clus > best_neighbor) std::swap(best_clus,best_neighbor) ;
      clusters->merge(best_clus,best_neighbor) ;
      if (clusters->numSubclusters() <= this->desiredClusters())
	 break ;
      // update nearest neighbors
      this->log(2,"  updating nearest neighbors") ;
      tp->parallelize(update_nearest_neighbors<IdxT,ValT>,numclus,clusters->subclusters(),similarities,
	 neighbors,this->m_measure,best_clus,best_neighbor) ;
      prog->incr() ;
      }
   delete[] neighbors ;
   delete[] similarities ;
   delete prog ;
   if (this->m_flatten && clusters->numSubclusters() > 1)
      {
      this->log(0,"  flattening clusters") ;
      clusters->flattenSubclusters() ;
      }
   else
      {
      this->log(0,"  relabeling vectors with paths") ;
      //TODO
      }
   this->log(0,"Clustering complete") ;
   return clusters ;
}

} // end of namespace Fr

// end of file cluster_agglom.C //
