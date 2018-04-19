/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-19					*/
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

#include "template/vector.cc"
#include "template/densevector.cc"
#include "template/sparsevector.cc"

namespace Fr
{

// request explicit instantiations
template class Vector<float> ;
template class DenseVector<float> ;
template class SparseVector<uint32_t,float> ;

// static data for the instantiated templates
template <>
Allocator Vector<float>::s_allocator(FramepaC::Object_VMT<Vector<float>>::instance(), sizeof(Vector<float>)) ;
template <>
const char* Vector<float>::s_typename = "Vector_flt" ;

template <>
Allocator DenseVector<float>::s_allocator(FramepaC::Object_VMT<DenseVector<float>>::instance(),
   sizeof(DenseVector<float>)) ;
template <>
const char* DenseVector<float>::s_typename = "DenseVector_flt" ;

template <>
Allocator SparseVector<uint32_t,float>::s_allocator(FramepaC::Object_VMT<SparseVector<uint32_t,float>>::instance(),
   sizeof(SparseVector<uint32_t,float>)) ;
template <>
const char* SparseVector<uint32_t,float>::s_typename = "SparseVector_u32flt" ;


} // end namespace Fr

// end of file vector_u32_flt.C //
