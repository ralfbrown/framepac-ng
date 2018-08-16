/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-15					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
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

template <typename IdxT, typename ValT>
class ClusteringAlgoIncr : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoIncr() { delete this ; }
      virtual const char*algorithmName() const { return "Single-Link" ; }

      virtual ClusterInfo* cluster(const Array* vectors) const ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoIncr<IdxT,ValT>::cluster(const Array* vectors) const
{
   RefArray* seed ;
   RefArray* nonseed ;
   if (!this->separateSeeds(vectors,seed,nonseed))
      return nullptr ;			// can't cluster: either no vector or not all same type
   // generate initial clusters by merging all seeds with the same label together
   this->log(1,"Collecting seeds into clusters") ;
   auto clusters = Array::create() ;
   for (auto vec : *seed)
      {
      auto vector { static_cast<Vector<ValT>*>(vec) } ;
      auto found { false } ;
      for (auto cl : *clusters)
	 {
	 auto cluster = static_cast<ClusterInfo*>(cl) ;
	 if (cluster->label() == vector->label())
	    {
	    cluster->addMember(vector) ;
	    found = true ;
	    break ;
	    }
	 }
      if (!found)
	 {
	 auto newclus = ClusterInfo::createSingleton(vector) ;
	 newclus->setLabel(vector->label()) ;
	 clusters->appendNoCopy(newclus) ;
	 }
      }
   this->log(0,"Clustering vectors") ;
   auto prog = this->makeProgressIndicator(nonseed->size()) ;
   // now iterate through the non-seed vectors, creating a new cluster if the nearest existing cluster is too
   //   far away and we haven't yet reached the cluster limit; otherwise, assign to the nearest existing cluster
   for (auto vec : *nonseed)
      {
      size_t best_clus ;
      auto vector = static_cast<Vector<ValT>*>(vec) ;
      auto best_sim = this->findNearestCluster(clusters,vector,best_clus,nullptr) ;
      if (best_sim < this->clusterThreshold() && clusters->size() < this->desiredClusters())
	 {
	 // create a new cluster and add the vector as its initial member
	 auto newclus = ClusterInfo::createSingleton(vector) ;
	 newclus->genLabel() ;
	 clusters->appendNoCopy(newclus) ;
	 }
      else if (best_clus)
	 {
	 // add the vector to the nearest cluster
	 auto best_cluster = static_cast<ClusterInfo*>(clusters->getNth(best_clus)) ;
	 best_cluster->addMember(vector) ;
	 vector->setLabel(best_cluster->label()) ;
	 }
      prog->incr() ;
      }
   delete prog ;
   this->log(0,"  %lu clusters",clusters->size()) ;
   return ClusterInfo::create(clusters) ;
}

} // end of namespace Fr

// end of file cluster_incr.C //
