/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#include "template/bidindex.cc"
#include "framepac/cstring.h"

namespace Fr
{

// specializations
template <>
CString BidirIndex<CString,uint32_t>::getKey(uint32_t index) const
{
   return index < m_max_index ? m_reverse_index[index] : CString(nullptr) ;
}

template <>
void BidirIndex<CString,uint32_t>::clearReverseElement(CString* elt)
{
   elt->release() ;
   return ;
}

template <>
void BidirIndex<CString,uint32_t>::releaseCommonBuffer(CString* elt0)
{
   elt0->release() ;
   return ;
}

// request explicit instantiation
template class BidirIndex<CString,uint32_t> ;

} // end namespace Fr

// end of file bidindex_cstr.C //
