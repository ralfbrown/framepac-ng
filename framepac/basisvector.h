/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-09-03					*/
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

#ifndef __FrBASISVECTOR_H_INCLUDED
#define __FrBASISVECTOR_H_INCLUDED

#include "framepac/vector.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT = int8_t>
class BasisVector : public SparseVector<IdxT,ValT>
   {
   public:
      typedef SparseVector<IdxT,ValT> super ;
   public:
      static BasisVector* create(size_t numelts, size_t num_plus, size_t num_minus = (size_t)~0)
	 { return new BasisVector(numelts,num_plus,num_minus) ; }

   protected: // creation/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      BasisVector(size_t numelts, size_t num_plus, size_t num_minus) ;
      BasisVector(const BasisVector&) ;
      ~BasisVector() {}

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<BasisVector> ;

      // type determination predicates
      static const char* typeName_(const Object*) { return s_typename ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *obj) { return new BasisVector(*static_cast<const BasisVector*>(obj)) ; }
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<BasisVector*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete static_cast<BasisVector*>(obj) ; }

   private:
      static Allocator s_allocator ;

   protected:
      static const char s_typename[] ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !__FrBASISVECTOR_H_INCLUDED */

// end of file basisvector.h
