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
   const Array* clusters = va_arg(args,const Array*) ;
   double* similarity = va_arg(args,double*) + index ;
   size_t* neighbor = va_arg(args,size_t*) + index ;
   VM* measure = va_arg(args,VM*) ;
   if (!measure) return false ;
//!!   auto cluster = clusters->getNth(index) ;
   double best_sim = -999.99 ;
   size_t best_neighbor = ~0 ;
   for (size_t i = 0 ; i < clusters->size() ; ++i)
      {
      if (i == index) continue ;
//!!      auto other_clus = clusters->getNth(i) ;
      //TODO
      }
   *similarity = best_sim ;
   *neighbor = best_neighbor ;
   return true ;
}

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
   while (clusters->numSubclusters() > this->desiredClusters())
      {
      auto numclus = clusters->numSubclusters() ;
      ThreadPoolMapFunc* fn = agglom_clustering_best_similarity<IdxT,ValT> ;
      tp->parallelize(fn,numclus,clusters->subclusters(),similarities,
	 neighbors,this->m_measure) ;
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
      prog->incr() ;
      if (best_sim < this->clusterThreshold())
	 {
	 this->log(0,"  terminating: best similarity %g is less than threshold %g",best_sim,this->clusterThreshold());
	 break ;
	 }
      //TODO: merge the cluster we found
      this->log(2,"  merging clusters %lu and %lu (similarity %g)",best_clus,best_neighbor,best_sim) ;
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
