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
class ClusteringAlgoKMeans : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoKMeans() { delete this ; }

      virtual ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) ;

   protected:
      size_t m_desired_clusters ;
      size_t m_iterations ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoKMeans<IdxT,ValT>::cluster(ObjectIter& first, ObjectIter& past_end)
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
   //    make the centroids the new centers
   size_t iteration ;
   for (iteration = 1 ; iteration <= m_iterations ; iteration++)
      {
      if (!this->assignToNearest(vectors, centers))
	 break ;			// we've converged!
      ClusterInfo* clusters ;
      size_t num_clusters ;
      this->extractClusters(vectors,clusters,num_clusters) ;
      if (this->usingSparseVectors())
	 {
	 for (size_t i = 0 ; i < num_clusters ; ++i)
	    {
	    // create a centroid of the members of the current cluster
	    SparseVector<IdxT,ValT>* centroid = SparseVector<IdxT,ValT>::create() ;
	    const Array* members = clusters[i].members() ;
	    for (size_t j = 0 ; j < members->size() ; j++)
	       {
	       SparseVector<IdxT,ValT>* vec = static_cast<SparseVector<IdxT,ValT>*>(members->getNth(j)) ;
	       if (!vec) continue ;
	       centroid->add(vec) ;
	       }
	    // make the centroid the new center for the cluster
	    if (iteration != 1)
	       centers.getNth(i)->free() ;
	    centers.setNth(i,centroid) ;
	    }
	 }
      else
	 {
	 for (size_t i = 0 ; i < num_clusters ; ++i)
	    {
	    // create a centroid of the members of the current cluster
	    Vector<ValT>* centroid = Vector<ValT>::create() ;
	    const Array* members = clusters[i].members() ;
	    for (size_t j = 0 ; j < members->size() ; j++)
	       {
	       DenseVector<ValT>* vec = static_cast<DenseVector<ValT>*>(members->getNth(j)) ;
	       if (!vec) continue ;
	       centroid->add(vec) ;
	       }
	    // make the centroid the new center for the cluster
	    if (iteration != 1)
	       centers.getNth(i)->free() ;
	    centers.setNth(i,centroid) ;
	    }
	 }
      }
//TODO
   return nullptr ;
}

} // end of namespace Fr

// end of file cluster_kmeans.C //
