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
ClusterInfo* ClusteringAlgoBrown<IdxT,ValT>::cluster(const Array* vectors) const
{
   if (!vectors || vectors->size() == 0)
      return ClusterInfo::create() ;
   ClusterInfo* clusters = ClusterInfo::createSingletonClusters(vectors) ;
   if (clusters->numSubclusters() <= this->desiredClusters())
      {
      this->log(0,"Nothing to be clustered - want %lu clusters, have only %lu vectors",
	 this->desiredClusters(),clusters->numSubclusters()) ;
      return clusters ;
      }
   this->log(0,"Starting %s clustering using %s measure; %lu vectors to cluster",
      this->algorithmName(),this->measureName(),vectors->size()) ;
   // until we've reached the desired number of clusters or the best similarity is below the threshold:
   //   merge the two most similar clusters
   auto prog = this->makeProgressIndicator(vectors->size() - this->desiredClusters()) ;
   while (clusters->numSubclusters() > this->desiredClusters())
      {
      double best_sim = -999.99 ;
   //TODO
      prog->incr() ;
      if (best_sim < this->clusterThreshold())
	 {
	 this->log(0,"  terminating: best similarity %g is less than threshold %g",best_sim,this->clusterThreshold());
	 break ;
	 }
      }
   delete prog ;
   this->log(0,"Clustering complete") ;
   return clusters ;
}

} // end of namespace Fr

// end of file cluster_agglom.C //
