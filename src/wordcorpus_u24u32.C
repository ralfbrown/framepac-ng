/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-15					*/
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

#include "template/concbuilder.cc"
#include "template/wordcorpus.cc"

namespace Fr
{

// request an explicit instantiation
template class BufferBuilder<UInt24,1> ;
template class ConcurrentBufferBuilder<UInt24,1> ;
template class WordCorpusT<UInt24,uint32_t> ;

} // end of namespace Fr

// end of file wordcorpus_u24u32.C //
