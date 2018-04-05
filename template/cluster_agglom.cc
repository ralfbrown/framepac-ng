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

// the Agglomerative method from the first-gen FramepaC is basically Brown clustering with early exit where
//   the resulting clusters are flattened, so we'll implement Brown clustering with an early-exit option and
//   make Agglomerative a subclass of Brown

template <typename IdxT, typename ValT>
class ClusteringAlgoBrown : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoBrown() { delete this ; }

      virtual ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) ;

   protected:

   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class ClusteringAlgoAgglom : public ClusteringAlgoBrown<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoAgglom() { delete this ; }

      virtual ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) ;

   protected:

   } ;

/************************************************************************/
/************************************************************************/

} // end of namespace Fr

// end of file cluster_agglom.C //