/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#include "framepac/trie.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/


/************************************************************************/
/*	Methods for template class PackedTrie				*/
/************************************************************************/

template <typename T, typename IdxT, unsigned bits>
PackedTrie<T,IdxT,bits>::PackedTrie()
{

   return ;
}

//----------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
PackedTrie<T,IdxT,bits>::PackedTrie(const Trie<T,IdxT,bits>* /*in_trie*/)
{

   return ;
}

//----------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
PackedTrie<T,IdxT,bits>::PackedTrie(const char* /*filename*/)
{

   return ;
}

//----------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
void PackedTrie<T,IdxT,bits>::init(IdxT /*cap*/)
{

   return ;
}

//----------------------------------------------------------------------

} // end namespace Fr


// end of file ptrie.cc //
