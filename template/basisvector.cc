/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-08-27					*/
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
   // enforce that at most half of the dimensions are nonzero
   if (num_plus + num_minus > numelts / 2)
      {
      double scl = (numelts / 2) / (double)(num_plus + num_minus) ;
      num_plus = (size_t)(num_plus * scl) ;
      num_minus = (size_t)(num_minus * scl) ;
      }
   RandomInteger rand(numelts) ;
   rand.randomize() ;
   // randomly pick dimensions to have value +1
   for (size_t i = 0 ; i < num_plus ; ++i)
      {
      size_t indx ;
      do {
         indx = rand() ;
	 } while (!this->newElement(indx,1)) ;
      }
   // randomly pick as-yet unused dimensions to have value -1
   for (size_t i = 0 ; i < num_minus ; ++i)
      {
      size_t indx ;
      do {
         indx = rand() ;
	 } while (!this->newElement(indx,-1)) ;
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
