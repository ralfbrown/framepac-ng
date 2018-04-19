/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-19					*/
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

#include "template/vector.cc"
#include "template/densevector.cc"
#include "template/sparsevector.cc"

namespace Fr
{

// request explicit instantiations of TermCountVector
template class Vector<uint32_t> ;
template class DenseVector<uint32_t> ;
template class SparseVector<uint32_t,uint32_t> ;

// static data for the instantiated templates
template <>
Allocator Vector<uint32_t>::s_allocator(FramepaC::Object_VMT<Vector<uint32_t>>::instance(), sizeof(Vector<uint32_t>)) ;

template <>
Allocator DenseVector<uint32_t>::s_allocator(FramepaC::Object_VMT<DenseVector<uint32_t>>::instance(),
   sizeof(DenseVector<uint32_t>)) ;

template <>
Allocator SparseVector<uint32_t,uint32_t>::s_allocator(FramepaC::Object_VMT<SparseVector<uint32_t,uint32_t>>::instance(),
   sizeof(SparseVector<uint32_t,uint32_t>)) ;


} // end namespace Fr

// end of file vector_u32_u32.C //
