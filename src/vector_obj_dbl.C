/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-18					*/
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
template class SparseVector<Object*,double> ;

// static data for the instantiated template
template <>
Allocator SparseVector<Object*,double>::s_allocator(FramepaC::Object_VMT<SparseVector<Object*,double>>::instance(),
   sizeof(SparseVector<Object*,double>)) ;
template <>
const char SparseVector<Object*,double>::s_typename[] = "SparseVector_objdbl" ;

} // end namespace Fr

// end of file vector_obj_dbl.C //
