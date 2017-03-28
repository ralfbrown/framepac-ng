/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
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

#ifndef __Fr_TRIE_H_INCLUDED
#define __Fr_TRIE_H_INCLUDED

#include "framepac/byteorder.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
class Trie
   {
   public:
      class ValuelessNode
         {
	 public:
	    ValuelessNode() ;
	    ValuelessNode(const ValuelessNode&) = default ;
	    ~ValuelessNode() {}
	    ValuelessNode& operator= (const ValuelessNode&) = default ;

	    bool  leaf() const { return m_leaf ; }
	    bool  hasChildren() const ;
	    bool  childPresent(unsigned N) const ;
	    IdxT  childIndex(unsigned N) const ;
	    ValT  value() const { return ValT(0) ; }

	    void  markAsLeaf() { m_leaf = true ; }
	    void  setValue(ValT) {}
	    bool  insertChild(unsigned N, class Trie *) ;
	 protected:
	    IdxT  m_children[1<<4] ;
	    bool  m_leaf ;
         } ;
      class Node : public ValuelessNode
         {
	 public:
	    Node() ;
	    Node(const Node&) = default ;
	    ~Node() {}
	    ValT  value() const { return m_value ; }
	    void  setValue(ValT v) { m_value = v ; }
	 protected:
	    ValT  m_value ;
         } ;

      Trie(IdxT capacity) ;
      ~Trie() ;

      bool insert(const uint8_t *key, unsigned keylength, ValT value) ;
      bool extendKey(IdxT &index, uint8_t keybyte) const ;
      ValT find(const uint8_t* key, unsigned keylength) const ;

      IdxT size() const { return m_size_valueless + m_size_full ; }
      IdxT capacity() const { return m_capacity_valueless + m_capacity_full ; }
      unsigned longestKey() const { return m_maxkey ; }

      Node* node(IdxT N) const ;
      Node* rootNode() const ;

   protected:
      ValuelessNode** m_valueless ;
      Node**          m_nodes ;
      IdxT            m_capacity_valueless ;
      IdXT            m_size_valueless ;
      IdxT            m_capacity_full ;
      IdXT            m_size_full ;
      unsigned        m_maxkey ;
   private:
      void init(IdxT capacity) ;
      IdxT allocateNode() ;
   } ;

/************************************************************************/
/************************************************************************/

// a trie with a variable list of items as node values

template <typename IdxT, typename ValT>
class MultiTrie
   {
   public:
      MultiTrie() ;
      ~MultiTrie() ;

   protected:

   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
class PackedTrie
   {
   public:
      // since we consume only a partial byte of the key with each level in the
      //   trie, intermediate levels can save space by not including a field for the
      //   node's value.  If we allow only leaf nodes to have values, all non-leaf
      //   nodes can be valueless.
      class ValuelessNode
         {
	 public:
	    ValuelessNode() ;
	    ~ValuelessNode() ;
	    ValT nodeValue() const { return ValT(0) ; }

	 protected:
	    IdxT   m_firstchild ;	// children are stored breadth-first, so we need only
	    Int16  m_children ;		//   the index of the first child and an offset from there
         } ;
      // a full node can contain both a value and child pointers
      class Node : public ValuelessNode
         {
	 public:
	    Node() ;
	    ~Node() ;
	    ValT nodeValue() const { return m_value ; }

	 protected:
	    ValT   m_value ;		// we allow non-leaf nodes to have values
         } ;
      // leaves don't need to store child pointers
      class LeafNode
         {
	 public:
	    LeafNode() ;
	    ~LeafNode() ;
	    ValT nodeValue() const { return m_value ; }

	 protected:
	    ValT     m_value ;
         } ;

      PackedTrie() ;
      PackedTrie(const Trie*) ;
      PackedTrie(const char *filename) ;
      PackedTrie(const PackedTrie&) = delete ;
      ~PackedTrie() ;
      PackedTrie& operator= (const PackedTrie&) = delete ;

      unsigned longestKey() const { return m_maxkey ; }
      bool extendKey(IdxT& index, uint8_t keybyte) const ;
      bool find(const uint8_t* key, unsigned keylength, ValT& value) const ;
      bool leafNode(IdxT& index) const ;
      bool nodeHasValue(IdxT& index) const ;
      ValT nodeValue(IdxT& index) const ;
      bool nodeValue(IdxT& index, ValT& value) const ;

   protected:
      // because we have three different types of nodes, we need to
      //   store pointers to the arrays of each type of node
      Node*          m_fullnodes ;
      ValuelessNode* m_nodes ;
      LeafNode*      m_leaves ;
      // we also need to store the starting index of each type except
      //   full nodes, so that we can convert a simple index into the
      //   node type while traversing the trie
      IdxT           m_first_valuelessnode ;
      IdxT           m_first_leaf ;
   } ;

/************************************************************************/
/************************************************************************/

// we can store an arbitrary list of same-typed items as the value of
//   a node in a packed trie by storing an index in the node and
//   having a separate array of items

template <typename IdxT, typename ValIdxT, typename ValT>
class PackedMultiTrie : public PackedTrie<IdxT,ValIdxT>
   {
   public:
      PackedMultiTrie() ;
      ~PackedMultiTrie() ;

   protected:
      ValT*          m_values ;

   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file trie.h //
