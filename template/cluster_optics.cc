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
using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

/* parallel version of OPTICS described in
      Md. Mostofa Ali Patwary, Diana Palsetia, Anikt Agrawal, Wei-keng Liao, Frederik Manne, and Alok Choudhary,
      "Scalable Parallel OPTICS Data Clustering Using Graph Algorithmic Techniques".  In Proceedings fo the
      International Conference on High Performance Computing, Networking, Storage and Analysis (Supercomputing,
      SC'13), pp.49:1-49:12 (2013).
   source code to accompany the paper available at
       http://cucis.ece.northwestern.edu/projects/Clustering/download_code_optics.html
*/

template <typename IdxT, typename ValT>
class ClusteringAlgoOPTICS : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoOPTICS() = default ;
      virtual const char*algorithmName() const { return "OPTICS" ; }

      virtual ClusterInfo* cluster(const Array* vectors) const ;

   protected:

   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoOPTICS<IdxT,ValT>::cluster(const Array* vectors) const
{
   (void)vectors;
   return nullptr ; //TODO
}

} // end of namespace Fr

// end of file cluster_optics.cc //
