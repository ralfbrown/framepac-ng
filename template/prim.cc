/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-04-18					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#include "framepac/graph.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

template <typename V, typename Eidx, typename L>
GraphT<V,Eidx,L>::EdgeList&& GraphT<V,Eidx,L>::minSpanningTreePrims() const
{
   EdgeList* mst = new EdgeList ;
   mst->reserve(numVertices()) ;


   return *mst ;
}

//------------------------------------------------------------------------


}  // end namespace Fr

// end of file prim.cc //
