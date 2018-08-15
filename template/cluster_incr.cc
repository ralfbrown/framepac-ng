/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-13					*/
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

   protected: // data
      double m_clusterthresh ; //???
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
   Array* clusters = Array::create() ;
   for (auto vec : *seed)
      {
      auto vector { static_cast<Vector<ValT>*>(vec) } ;
      bool found { false } ;
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
	 ClusterInfo* newclus = ClusterInfo::createSingleton(vector) ;
	 clusters->appendNoCopy(newclus) ;
	 }
      }
   // now iterate through the non-seed vectors, creating a new cluster if the nearest existing cluster is too
   //   far away and we haven't yet reached the cluster limit; otherwise, assign to the nearest existing cluster
   for (auto vec : *nonseed)
      {
      auto vector = static_cast<Vector<ValT>*>(vec) ;
      double best_sim { -HUGE_VAL } ;
      ClusterInfo* best_clus { nullptr } ;
      for (auto clus : *clusters)
	 {
	 auto cluster = static_cast<ClusterInfo*>(clus) ;
	 double sim = cluster->similarity(vector,this->m_measure) ;
	 if (sim > best_sim)
	    {
	    best_sim = sim ;
	    best_clus = cluster ;
	    }
	 }
      if (best_sim < m_clusterthresh && clusters->size() < this->desiredClusters())
	 {
	 // create a new cluster and add the vector as its initial member
	 ClusterInfo* newclus = ClusterInfo::createSingleton(vector) ;
	 clusters->appendNoCopy(newclus) ;
	 }
      else if (best_clus)
	 {
	 // add the vector to the nearest cluster
	 best_clus->addMember(vector) ;
	 }
      }
   return ClusterInfo::create(clusters) ;
}

} // end of namespace Fr

// end of file cluster_incr.C //
