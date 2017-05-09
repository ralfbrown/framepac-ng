/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-08					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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
/*	Manifest Constants						*/
/************************************************************************/

#define ROOT_INDEX 0

/************************************************************************/
/*	Methods for template class Trie					*/
/************************************************************************/

template <typename T, typename IdxT, unsigned bits>
Trie<T,IdxT,bits>::ValuelessNode::ValuelessNode()
{
   for (size_t i = 0 ; i < lengthof(m_children) ; ++i)
      m_children[i] = NULL_INDEX ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::ValuelessNode::insertChild(unsigned N, Trie<T,IdxT,bits>* )
{
   (void)N ;
//FIXME
   return false ;
}


//----------------------------------------------------------------------------

/************************************************************************/
/*	Methods for template class MultiTrie				*/
/************************************************************************/

/************************************************************************/
/*	Methods for template class PackedTrie				*/
/************************************************************************/

/************************************************************************/
/*	Methods for template class PackedMultiTrie			*/
/************************************************************************/


} // end namespace Fr


// end of file trie.cc //
