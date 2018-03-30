/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-30					*/
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

#include "framepac/cluster.h"

namespace Fr
{

/************************************************************************/
/*	methods for class ClusterInfo					*/
/************************************************************************/

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* ClusterInfo::createSparseCentroid() const
{
   SparseVector<IdxT,ValT>* centroid = SparseVector<IdxT,ValT>::create() ;
   const Array* members = clusters[i].members() ;
   for (size_t j = 0 ; j < members->size() ; j++)
      {
      auto vec = static_cast<SparseVector<IdxT,ValT>*>(members->getNth(j)) ;
      if (!vec) continue ;
      centroid->add(vec) ;
      }
   return centroid ;
}

//----------------------------------------------------------------------------

template <typename ValT>
DenseVector<ValT>* ClusterInfo::createDenseCentroid() const
{
   DenseVector<ValT>* centroid = DenseVector<ValT>::create() ;
   const Array* members = clusters[i].members() ;
   for (size_t j = 0 ; j < members->size() ; j++)
      {
      auto vec = static_cast<DenseVector<ValT>*>(members->getNth(j)) ;
      if (!vec) continue ;
      centroid->add(vec) ;
      }
   return centroid ;
}


} // end namespace Fr

// end of file cluster.cc //
