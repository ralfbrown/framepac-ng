/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-18					*/
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

#include "template/termvector.cc"

namespace Fr
{

// request explicit instantiations
template class TermVectorT<uint32_t> ;
template class TermVectorT<float> ;

// static data for the instantiated templates
template <>
Allocator TermVectorT<uint32_t>::s_allocator(FramepaC::Object_VMT<TermVectorT<uint32_t>>::instance(),
   sizeof(TermVectorT<uint32_t>));
template <>
const char TermVectorT<uint32_t>::s_typename[] = "TermCountVector" ;

template <>
Allocator TermVectorT<float>::s_allocator(FramepaC::Object_VMT<TermVectorT<float>>::instance(),
   sizeof(TermVectorT<float>));
template <>
const char TermVectorT<float>::s_typename[] = "TermVector" ;

} // end namespace Fr

// end of file termvector.C //
