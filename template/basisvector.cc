/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-04-27					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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

#include "framepac/basisvector.h"
#include "framepac/random.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

template <typename IdxT, typename ValT>
BasisVector<IdxT,ValT>::BasisVector(size_t numelts, size_t num_plus, size_t num_minus)
   : SparseVector<IdxT,ValT>(num_plus+num_minus)
{
   if (num_plus + num_minus >= numelts)
      {
      double scale = numelts / (double)(num_plus + num_minus) ;
      num_plus = (size_t)(num_plus * scale) ;
      num_minus = (size_t)(num_minus * scale) ;
      }
   RandomInteger rand(numelts) ;
   for (size_t i = 0 ; i < num_plus ; ++i)
      {

      }
   for (size_t i = 0 ; i < num_minus ; ++i)
      {

      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
BasisVector<IdxT,ValT>::BasisVector(const BasisVector& orig) : SparseVector<IdxT,ValT>(orig)
{
   //TODO
   return ;
}


} // end namespace Fr

// end of file basisvector.cc //
