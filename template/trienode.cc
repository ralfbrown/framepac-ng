/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-15					*/
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

#include "framepac/atomic.h"
#include "framepac/trie.h"

namespace Fr
{

/************************************************************************/
/*	Methods for template class TrieNodeValueless			*/
/************************************************************************/

template <typename IdxT, unsigned bits>
TrieNodeValueless<IdxT,bits>::TrieNodeValueless()
{
   std::fill(m_children,m_children + (1<<bits),NULL_INDEX) ;
   return  ;
}

//----------------------------------------------------------------------------

template <typename IdxT, unsigned bits>
IdxT TrieNodeValueless<IdxT,bits>::setChild(unsigned N, IdxT ch)
{
   // CAS in the child; if that fails, someone else beat us to the insertion,
   //  so we should then discard the new node and use the one the other thread
   //  inserted
   IdxT expected = NULL_INDEX ;
   if (Atomic<IdxT>::ref(m_children[N]).compare_exchange_strong(expected,ch))
      return ch ;
   else
      return expected ;
}

/************************************************************************/
/*	Methods for template class TrieNode				*/
/************************************************************************/




} // end namespace Fr

// end of file trienode.cc //
