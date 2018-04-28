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

#include "template/basisvector.cc"

namespace Fr
{

// request explicit instantiation
template class BasisVector<uint32_t> ;
template <>
Allocator BasisVector<uint32_t>::s_allocator(FramepaC::Object_VMT<BasisVector<uint32_t>>::instance(),
   sizeof(BasisVector<uint32_t>)) ;

} // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<BasisVector>			*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::BasisVector<uint32_t>> ;

} // end namespace FramepaCC

// end of file basisvector_u32.C //
