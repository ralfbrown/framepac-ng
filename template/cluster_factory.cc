/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-03					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#include "template/cluster.cc"
#include "template/cluster_agglom.cc"
#include "template/cluster_anneal.cc"
#include "template/cluster_dbscan.cc"
#include "template/cluster_growseed.cc"
#include "template/cluster_incr.cc"
#include "template/cluster_kmeans.cc"
#include "template/cluster_optics.cc"

namespace Fr
{

template <typename IdxT, typename ValT>
ClusteringAlgo<IdxT,ValT>* ClusteringAlgo<IdxT,ValT>::instantiate(const char* algo_name, const char* options,
   VectorMeasure<IdxT,ValT>* measure)
{
   if (!algo_name || !*algo_name)
      return nullptr ;
   if (!options)
      options = "" ;
   ClusteringAlgorithm algo = parse_cluster_algo_name(algo_name) ;
   ClusteringAlgo* clusterer = nullptr ;
   switch (algo)
      {
      case ClusteringAlgorithm::agglomerative:
	 clusterer = new ClusteringAlgoAgglom<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::annealing:
	 clusterer = new ClusteringAlgoAnneal<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::brown:
	 clusterer = new ClusteringAlgoBrown<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::dbscan:
	 clusterer = new ClusteringAlgoDBScan<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::growseeds:
	 clusterer = new ClusteringAlgoGrowseed<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::kmeans:
	 clusterer = new ClusteringAlgoKMeans<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::kmediods:
	 clusterer = new ClusteringAlgoKMedioids<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::optics:
	 clusterer = new ClusteringAlgoOPTICS<IdxT,ValT> ; break  ;
//      case ClusteringAlgorithm::multipass_single_link:
//	 clusterer = new ClusteringAlgo<IdxT,ValT> ; break  ;
      case ClusteringAlgorithm::single_link:
	 clusterer = new ClusteringAlgoIncr<IdxT,ValT> ; break  ;
      default:
	 // missed case?
	 break ;
      }
   if (clusterer)
      {
      // parse options
      //TODO
      if (measure)
	 {
	 delete clusterer->m_measure ;
	 clusterer->m_measure = measure ;
	 }
      // default to cosine similarity if no other similarity measure is given in the options or by the caller
      if (!clusterer->m_measure)
	 {
	 clusterer->m_measure = VectorMeasure<IdxT,ValT>::create(VectorSimilarityMeasure::cosine) ;
	 }
      }
   return clusterer ;
}


} // end namespace Fr

// end of file cluster_factory.cc //

