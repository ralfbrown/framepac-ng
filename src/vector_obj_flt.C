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

#include "template/sparsevector.cc"

namespace Fr
{

// request explicit instantiation
template class SparseVector<Object*,float> ;

// static data for the instantiated template
template <>
Allocator SparseVector<Object*,float>::s_allocator(FramepaC::Object_VMT<SparseVector<Object*,float>>::instance(),
   sizeof(SparseVector<Object*,float>)) ;
template <>
const char* SparseVector<Object*,float>::s_typename = "SparseVector_objflt" ;

} // end namespace Fr

// end of file vector_obj_flt.C //
