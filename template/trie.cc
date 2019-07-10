/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-13					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018,2019 Carnegie Mellon University		*/
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

#include <memory.h>
#include "framepac/atomic.h"
#include "framepac/memory.h"
#include "framepac/trie.h"
#include "template/trienode.cc"

namespace Fr
{

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

/************************************************************************/
/*	Methods for template class Trie					*/
/************************************************************************/

template <typename T, typename IdxT, unsigned bits>
void Trie<T,IdxT,bits>::insertChild(IdxT &index, uint8_t keybyte)
{
   static_assert( bits >= 2 && bits <= 4, "Trie only supports 2/3/4 bits per level") ;
   constexpr uint8_t mask = (1<<bits) - 1 ;
   constexpr unsigned iter = (7 + bits) / bits ;
   ValuelessNode* n = node(index) ;
   for (unsigned shift = (iter-1)*bits ; shift > 0 ; shift -= bits)
      {
      unsigned childnum = (keybyte >> shift) & mask ;
      if (n->hasChild(childnum))
	 {
	 index = n->childIndex(childnum) ;
	 }
      else
	 {
	 // insert a ValuelessNode
	 IdxT new_idx = allocValuelessNode() ;
	 // allocValuelessNode can move the nodes array, so we need to reload 'n' if it's a valueless node
	 if (shift != (iter-1)*bits)
	    n = valuelessNode(index) ;
	 index = n->setChild(childnum,new_idx) ;
	 if (index != new_idx)
	    {
	    // someone else already inserted a child, so use that one instead
	    //   of the new node we just allocated
	    releaseValuelessNode(new_idx) ;
	    }
	 }
      n = valuelessNode(index) ;
      }
   // insert the final full Node
   keybyte &= mask ;
   if (n->hasChild(keybyte))
      {
      index = n->childIndex(keybyte) ;
      }
   else
      {
      IdxT new_idx = allocNode() ;
      index = n->setChild(keybyte,new_idx) ;
      if (index != new_idx)
	 {
	 // someone else already inserted a child, so use that one instead
	 //   of the new node we just allocated
	 releaseNode(new_idx) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::insert(const uint8_t* key, unsigned keylength, T value)
{
   static_assert( bits >= 2 && bits <= 4, "Trie only supports 2/3/4 bits per level") ;
   IdxT index = ROOT_INDEX ;
   for (unsigned i = 0 ; i < keylength ; ++i)
      {
      insertChild(index,key[i]) ;
      }
   if (index != NULL_INDEX || keylength > 0)
      {
      Node* n = node(index) ;
      if (n)
	 {
	 n->setValue(value) ;
	 n->markAsLeaf() ;
	 if (keylength > m_maxkey)
	    m_maxkey = keylength ;
	 return true ;
	 }
      }
   return false ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::extendKey(IdxT& index, uint8_t keybyte) const
{
   static_assert( bits >= 2 && bits <= 4, "Trie only supports 2/3/4 bits per level") ;
   constexpr uint8_t mask = (1<<bits) - 1 ;
   constexpr unsigned iter = (7 + bits) / bits ;
   IdxT childindex = index ;
   ValuelessNode* n = node(childindex) ;
   // descend through the intermediate valuelessNodes
   for (unsigned shift = (iter-1)*bits ; shift > 0 ; shift -= bits)
      {
      unsigned childnum = (keybyte >> shift) & mask ;
      childindex = n->childIndex(childnum) ;
      if (childindex == NULL_INDEX)
	 return false ;
      n = valuelessNode(childindex) ;
      }
   // retrieve the final full node
   childindex = n->childIndex(keybyte & mask) ;
   if (childindex == NULL_INDEX)
      return false ;
   index = childindex ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
IdxT Trie<T,IdxT,bits>::findNode(const uint8_t* key, unsigned keylength) const
{
   IdxT index = ROOT_INDEX ;
   for (size_t i = 0 ; i < keylength ; ++i)
      {
      if (!extendKey(index,key[i]))
	 return NULL_INDEX ;
      }
   return index ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
T Trie<T,IdxT,bits>::find(const uint8_t* key, unsigned keylength) const
{
   IdxT n = findNode(key,keylength) ;
   if (n != ROOT_INDEX || node(n)->leaf())
      return node(n)->value() ;
   return nullVal() ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::contains(const uint8_t* key, unsigned keylength) const
{
   IdxT n = findNode(key,keylength) ;
   if (n != ROOT_INDEX || node(n)->leaf())
      return true ;
   return false ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
IdxT Trie<T,IdxT,bits>::terminalNodes() const
{
   IdxT count = 0 ;
   // scan the full (value-containing) nodes for those marked as leaves which have no children
   for (IdxT i = 0 ; i < fullNodes() ; ++i)
      {
      if (m_nodes[i].leaf() && !m_nodes[i].hasChildren())
	 ++count ;
      }
   return count ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::enumerateVA(uint8_t* keybuf, size_t keylen, IdxT node_id,
				    EnumFunc* fn, std::va_list args) const
{
   static_assert( bits >= 2 && bits <= 4, "Trie only supports 2/3/4 bits per level") ;
//FIXME: need to recursively descend
   Node* n = node(node_id) ;
   if (n->leaf())
      {
      std::va_list argcopy ;
      va_copy(args,argcopy) ;
      if (!fn(keybuf,keylen,n->value(),argcopy))
	 return false ;
      }
   return true ; 

}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
bool Trie<T,IdxT,bits>::enumerateVA(EnumFunc* fn, std::va_list args) const
{
   if (!fn)
      return false ;
   LocalAlloc<uint8_t> key(longestKey()+1) ;
   return enumerateVA(key,0,ROOT_INDEX,fn,args) ;
}


} // end namespace Fr

// end of file trie.cc //
