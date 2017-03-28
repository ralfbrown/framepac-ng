/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

#ifndef __FrBASISVECTOR_H_INCLUDED
#define __FrBASISVECTOR_H_INCLUDED

#include "framepac/vector.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

template <typename IdxT>
class BasisVector : public SparseVector<IdxT,int8_t>
   {
   public:
      static BasisVector* create(size_t numelts) ;
      static BasisVector* create(size_t numelts, size_t num_plus, size_t num_minus = (size_t)~0) ;

      bool generateRandomBasis(size_t num_plus, size_t num_minus) ;

   protected:
      BasisVector(size_t numelts) ;
      BasisVector(size_t numelts, size_t num_plus, size_t num_minus) ;
      BasisVector(const BasisVector&) ;
      ~BasisVector() ;

   private:

   } ;

#endif /* !__FrBASISVECTOR_H_INCLUDED */

// end of file basisvector.h
