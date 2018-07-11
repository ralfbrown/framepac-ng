/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-11					*/
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
void Trie<T,IdxT,bits>::init(IdxT cap)
{
   m_maxkey = 0 ;
   m_size_full = 0 ;
   m_size_valueless = 0 ;
   m_capacity_full = cap ;
   m_capacity_valueless = cap ;
   m_nodes = new Node[cap] ;
   m_valueless = new ValuelessNode[cap] ;
   if (!m_valueless)
      {
      m_capacity_valueless = 0 ;
      }
   if (!m_nodes)
      {
      m_capacity_full = 0 ;
      }
   (void)allocNode() ; // reserve the root node, which must be a full node because we could have an empty key
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
Trie<T,IdxT,bits>::~Trie()
{
   delete[] m_valueless ;
   delete[] m_nodes ;
   m_capacity_full = 0 ;
   m_capacity_valueless = 0 ;
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
IdxT Trie<T,IdxT,bits>::allocValuelessNode()
{
   std::lock_guard<std::mutex> guard(m_valueless_mutex) ;
   // TODO: can we do this more efficiently than with a lock?
   if (m_size_valueless >= m_capacity_valueless)
      {
      // reallocate the node buffer
      IdxT newcap = (m_capacity_valueless <= 500 ? 1000 : 3*m_capacity_valueless/2) ;
      Node* new_nodes = new Node[newcap] ;
      if (new_nodes)
	 {
	 memcpy(new_nodes,m_valueless,m_capacity_valueless*sizeof(Node)) ;
	 delete[] m_valueless ;
	 m_valueless = new_nodes ;
	 m_capacity_valueless = newcap ;
	 }
      }
   if (m_size_valueless < m_capacity_valueless)
      {
      return m_size_valueless++ ;
      }
   return NULL_INDEX ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
IdxT Trie<T,IdxT,bits>::allocNode()
{
   std::lock_guard<std::mutex> guard(m_full_mutex) ;
   // TODO: can we do this more efficiently than with a lock?
   if (m_size_full >= m_capacity_full)
      {
      // reallocate the node buffer
      IdxT newcap = (m_capacity_full <= 500 ? 1000 : 3*m_capacity_full/2) ;
      Node* new_nodes = new Node[newcap] ;
      if (new_nodes)
	 {
	 memcpy(new_nodes,m_nodes,m_capacity_full*sizeof(Node)) ;
	 delete[] m_nodes ;
	 m_nodes = new_nodes ;
	 m_capacity_full = newcap ;
	 }
      }
   if (m_size_full < m_capacity_full)
      {
      return m_size_full++ ;
      }
   return NULL_INDEX ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
void Trie<T,IdxT,bits>::releaseValuelessNode(IdxT index)
{
   std::lock_guard<std::mutex> guard(m_valueless_mutex) ;
   if (index != NULL_INDEX && index + 1 == m_size_valueless)
      --m_size_valueless ;  //TODO: make atomic with CAS?
   return ;
}

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
void Trie<T,IdxT,bits>::releaseNode(IdxT index)
{
   std::lock_guard<std::mutex> guard(m_full_mutex) ;
   if (index != NULL_INDEX && index + 1 == m_size_full)
      --m_size_full ;  //TODO: make atomic with CAS?
   return ;
}

//----------------------------------------------------------------------------

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
	 index = n->setChild(childnum,new_idx) ;
	 if (index != new_idx)
	    {
	    // someone else already inserted a child, so use that one instead
	    //   of the new node we just allocated
	    releaseValuelessNode(new_idx) ;
	    }
	 }
      if (shift > bits)
	 n = valuelessNode(index) ;
      else
	 n = node(index) ;
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
      index = n->setChild(index,new_idx) ;
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
      if (n) n->setValue(value) ;
      return true ;
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
      if (shift > bits)
	 n = valuelessNode(childindex) ;
      else
	 n = node(childindex) ;
      }
   // process the final full node
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
