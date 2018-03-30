/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-29					*/
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
      virtual ~ClusteringAlgoGrowseed() { delete this ; }

      virtual ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) ;

   protected: // data
      double m_clusterthresh ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoGrowseed<IdxT,ValT>::cluster(ObjectIter& first, ObjectIter& past_end)
{
   RefArray seed ;			// vectors with a cluster assignment at the outset
   RefArray nonseed ;			// vectors which need to be given a cluster assignment
   RefArray vectors ;			// all vectors
   for (ObjectIter it = first ; it != past_end ; ++it)
      {
      Object* obj = *it ;
      if (!obj || !obj->isVector())
	 continue ;
      Vector<ValT>* vec = static_cast<Vector<ValT>*>(obj) ;
      vectors.append(vec) ;
      if (1) // TODO
	 seed.append(vec) ;
      else
	 nonseed.append(vec) ;
      }
   if (!this->checkSparseOrDense(vectors))
      return nullptr ;			// vectors must be all dense or all sparse
   // assign each of the non-seed vectors to the same cluster as the
   //   nearest of the seed vectors, provided the similarity measure
   //   is above threshold
   this->assignToNearest(nonseed, seed, m_clusterthresh) ;
   // collect the vectors into clusters based on the assignment stored
   //   in the vector
   // many vectors will not be assigned to any cluster (because they
   //   weren't close enough to a seed); those will be assigned to a
   //   "null" cluster
   ClusterInfo* clusters ;
   size_t num_clusters ;
   RefArray unassigned ;
   this->extractClusters(vectors,clusters,num_clusters,&unassigned) ;
   // build a ClusterInfo structure with the subclusters, and all unassigned vectors inserted at the top level
   ClusterInfo* result_clusters = ClusterInfo::create(clusters,num_clusters) ;
   delete[] clusters ;
   result_clusters->addVectors(unassigned) ;
   // the subclusters are the actual result, the unassigned vectors at the top level should (normally) be ignored
   result_clusters->setFlag(ClusterInfo::Flags::group) ;
   return result_clusters ;
}

} // end of namespace Fr

// end of file cluster_growseed.cc //
