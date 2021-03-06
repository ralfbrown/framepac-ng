/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-17					*/
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
#include "framepac/progress.h"
#include "framepac/vector.h"

using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
class ClusteringAlgoGrowseed : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoGrowseed() = default ;
      virtual const char*algorithmName() const { return "GrowSeeds" ; }

      virtual ClusterInfo* cluster(const Array* vectors) const ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoGrowseed<IdxT,ValT>::cluster(const Array* vectors) const
{
   RefArray* seed ;	// vectors with a cluster assignment at the outset
   RefArray* nonseed ;	// vectors which need to be given a cluster assignment
   if (!this->separateSeeds(vectors,seed,nonseed))
      {
      return nullptr ;			// vectors must be all dense or all sparse
      }
   // assign each of the non-seed vectors to the same cluster as the
   //   nearest of the seed vectors, provided the similarity measure
   //   is above threshold
   this->log(0,"Assigning vectors to nearest seed") ;
   ProgressIndicator* prog = this->makeProgressIndicator(nonseed->size()) ;
   this->assignToNearest(nonseed, seed, prog, this->clusterThreshold()) ;
   delete prog ;
   seed->free() ;
   nonseed->free() ;
   // collect the vectors into clusters based on the assignment stored
   //   in the vector
   // many vectors will not be assigned to any cluster (because they
   //   weren't close enough to a seed); those will be assigned to a
   //   "null" cluster
   this->log(0,"Collecting vectors into clusters") ;
   ClusterInfo** clusters ;
   size_t num_clusters ;
   ScopedObject<RefArray> unassigned ;
   this->extractClusters(vectors,clusters,num_clusters,unassigned) ;
   this->log(0,"  %lu vectors were not assigned to a seed",unassigned->size()) ;
   // build a ClusterInfo structure with the subclusters, and all unassigned vectors inserted at the top level
   ClusterInfo* result_clusters = ClusterInfo::create(clusters,num_clusters) ;
   this->freeClusters(clusters,num_clusters) ;
   result_clusters->addVectors(unassigned) ;
   // the subclusters are the actual result, the unassigned vectors at the top level should (normally) be ignored
   result_clusters->setFlag(ClusterInfo::Flags::group) ;
   return result_clusters ;
}

} // end of namespace Fr

// end of file cluster_growseed.cc //
