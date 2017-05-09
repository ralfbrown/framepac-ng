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

#include "framepac/atomic.h"
#include "framepac/trie.h"

namespace Fr
{

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

/************************************************************************/
/*	Methods for template class Trie					*/
/************************************************************************/

template <typename T, typename IdxT, unsigned bits>
void Trie<T,IdxT,bits>::init(IdxT cap)
{
   (void)cap;
//FIXME
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
Trie<T,IdxT,bits>::~Trie()
{
   
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
IdxT Trie<T,IdxT,bits>::allocValuelessNode()
{
   
   return NULL_INDEX ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
IdxT Trie<T,IdxT,bits>::allocNode()
{
   
   return NULL_INDEX ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
void Trie<T,IdxT,bits>::releaseValuelessNode(IdxT index)
{
   (void)index ;
//FIXME
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
void Trie<T,IdxT,bits>::releaseNode(IdxT index)
{
   (void)index ;
//FIXME
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::insert(const uint8_t* key, unsigned keylength, T value)
{
   (void)key; (void)keylength; (void)value ;
//FIXME
   return false ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::extendKey(IdxT& index, uint8_t keybyte) const
{
   (void)index; (void)keybyte;
//FIXME
   return false ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
T Trie<T,IdxT,bits>::find(const uint8_t* key, unsigned keylength) const
{
   (void)key; (void)keylength ;
//FIXME
   return T(0) ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
typename Trie<T,IdxT,bits>::Node* Trie<T,IdxT,bits>::node(IdxT N) const
{
   (void)N ;
//FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
typename Trie<T,IdxT,bits>::ValuelessNode* Trie<T,IdxT,bits>::valuelessNode(IdxT N) const
{
   (void)N ;
//FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
typename Trie<T,IdxT,bits>::Node* Trie<T,IdxT,bits>::rootNode() const
{
//FIXME
   return nullptr ;
}

/************************************************************************/
/*	Methods for template class Trie::ValuelessNode			*/
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
IdxT Trie<T,IdxT,bits>::ValuelessNode::insertChild(unsigned N, Trie<T,IdxT,bits>* trie)
{
   if (!childPresent(N))
      {
      IdxT new_index = trie->allocNode() ;
      if (new_index != NULL_INDEX)
	 {
	 // try to atomically insert the index of the new node
	 IdxT expected = NULL_INDEX ;
	 if (Atomic<IdxT>::ref(m_children[N]).compare_exchange_strong(expected,new_index))
	    {
	    return new_index ;
	    }
	 // if the insertion failed, that means someone else has already
	 //   added a child, so we can release the node we just allocated
	 //   and return the one the other thread inserted
	 trie->releaseNode(new_index) ;
	 }
      }
   return childIndex(N) ;
}

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
