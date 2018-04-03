/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-02					*/
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

#include "framepac/termvector.h"
#include "template/sparsevector.cc"
#include "template/vector_arith.cc"

namespace Fr
{

// request explicit instantiations
template class DenseVector<float> ;
template class SparseVector<uint32_t,float> ;

// static data for the instantiated templates
template <>
Allocator DenseVector<float>::s_allocator(FramepaC::Object_VMT<DenseVector<float>>::instance(),
   sizeof(DenseVector<float>)) ;

template <>
Allocator SparseVector<uint32_t,float>::s_allocator(FramepaC::Object_VMT<SparseVector<uint32_t,float>>::instance(),
   sizeof(SparseVector<uint32_t,float>)) ;


} // end namespace Fr

// end of file vector_u32_flt.C //
