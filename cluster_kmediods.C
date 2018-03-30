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
using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
class ClusteringAlgoKMediods : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoKMediods() { delete this ; }

      virtual ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) ;

   protected:
      size_t m_desired_clusters ;
      size_t m_iterations ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
bool update_mediod(const void* o, size_t id, const void* user_data, bool sparse)
{
   const ClusterInfo* inf = reinterpret_cast<const ClusterInfo*>(o) ;
   RefArray* centers = reinterpret_cast<RefArray*>(user_data) ;
   if (sparse)
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createSparseCentroid<IdxT,ValT>() ;
//TODO: find nearest original vector
      // make the centroid the new center for the cluster
      centers->setNth(id,centroid) ;
      }
   else
      {
      // create a centroid of the members of the current cluster
      auto centroid = inf->createDenseCentroid<ValT>() ;
//TODO: find nearest original vector
      // make the centroid the new center for the cluster
      centers->setNth(id,centroid) ;
      }
   return true ;			// no errors, safe to continue processing
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoKMediods<IdxT,ValT>::cluster(ObjectIter& first, ObjectIter& past_end)
{
   // collect the input vectors into an array
   RefArray vectors ;
   for ( ; first != past_end ; ++first)
      {
      Object* obj = *first ;
      if (obj && obj->isVector())
	 vectors.append(obj) ;
      }
   if (!this->checkSparseOrDense(vectors))
      return nullptr ;			// vectors must be all dense or all sparse
   // select K vectors which are (approximately) maximally separated
   RefArray centers ;
//TODO

   // until converged or iteration limit:
   //    assign each vector to the nearest center
   //    collect vectors into clusters by assigned center
   //    generate a new centroid for each cluster
   //    find the nearest original vector to each centroid and make it a new center
   size_t iteration ;
   ClusterInfo* clusters(nullptr) ;
   size_t num_clusters(0) ;
   for (iteration = 1 ; iteration <= m_iterations ; iteration++)
      {
      bool changes = this->assignToNearest(vectors, centers) ;
      this->extractClusters(vectors,clusters,num_clusters) ;
      if (!changes)
	 break ;			// we've converged!
      if (iteration != 1)
	 centers.clearArray(true) ;
      for (size_t i = 0 ;  i < num_clusters ; ++i)
	 {
	 update_mediod<IdxT,ValT>(clusters[i],i,&centers,this->usingSparseVectors()) ;
	 }
      }
   // build the final cluster result from the extracted clusters
//TODO
   return nullptr ;
}

} // end of namespace Fr

// end of file cluster_kmeans.C //
