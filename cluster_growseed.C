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

   protected:

   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoGrowseed<IdxT,ValT>::cluster(ObjectIter& first, ObjectIter& past_end)
{
   RefArray seed ;
   RefArray nonseed ;
   for (ObjectIter it = first ; it != past_end ; ++it)
      {
      Object* obj = *it ;
      if (!obj || !obj->isVector())
	 continue ;
      Vector<ValT>* vec = static_cast<Vector<ValT>*>(obj) ;
      if (1) // TODO
	 seed.append(vec) ;
      else
	 nonseed.append(vec) ;
      }
   //TODO
   return nullptr ;
}

} // end of namespace Fr

// end of file cluster_growseed.cc //
