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

#include <type_traits>
#include "framepac/byteorder.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename T, typename IdxT = std::uint32_t, unsigned bits=4>
class Trie
   {
   public:
      static constexpr IdxT ROOT_INDEX = (IdxT)0 ;
      static constexpr IdxT NULL_INDEX = (IdxT)0 ;
      class ValuelessNode
         {
	 public:
	    ValuelessNode() ;
	    ValuelessNode(const ValuelessNode&) = default ;
	    ~ValuelessNode() {}
	    ValuelessNode& operator= (const ValuelessNode&) = default ;

	    bool leaf() const { return false ; }  // has no value, so by definition not a leaf
	    bool hasChildren() const
	       {
		  for (size_t i = 0 ; i < lengthof(m_children) ; ++i)
		     {
		     if (m_children[i] != NULL_INDEX) return true ;
		     }
		  return false ;
	       }
	    bool childPresent(unsigned N) const { return m_children[N] != NULL_INDEX ; }
	    IdxT childIndex(unsigned N) const { return m_children[N] ; }

	    template <typename RetT = T>
	    typename std::enable_if<std::is_pointer<T>::value, RetT>::type
	    value() const { return (T)nullptr ; }
	    template <typename RetT = T>
	    typename std::enable_if<!std::is_pointer<T>::value, RetT>::type
	    value() const { return T(0) ; }

	    void  markAsLeaf() {}
	    void  setValue(T) {}
	    bool  insertChild(unsigned N, Trie *) ;
	 protected:
	    IdxT  m_children[1<<bits] ;
         } ;
      class Node : public ValuelessNode
         {
	 public:
	    Node() ;
	    Node(const Node&) = default ;
	    ~Node() {}

	    bool  leaf() const { return m_leaf ; }
	    void  markAsLeaf() { m_leaf = true ; }

	    T value() const { return m_value ; }
	    void setValue(T v) { m_value = v ; }
	 protected:
	    T     m_value ;
	    bool  m_leaf ;
         } ;

      Trie(IdxT capacity) ;
      ~Trie() ;

      bool insert(const uint8_t *key, unsigned keylength, T value) ;
      bool extendKey(IdxT &index, uint8_t keybyte) const ;
      T find(const uint8_t* key, unsigned keylength) const ;

      IdxT size() const { return m_size_valueless + m_size_full ; }
      IdxT capacity() const { return m_capacity_valueless + m_capacity_full ; }
      unsigned longestKey() const { return m_maxkey ; }

      Node* node(IdxT N) const ;
      Node* rootNode() const ;

   protected:
      ValuelessNode** m_valueless ;
      Node**          m_nodes ;
      IdxT            m_capacity_valueless ;
      IdxT            m_size_valueless ;
      IdxT            m_capacity_full ;
      IdxT            m_size_full ;
      unsigned        m_maxkey ;
   private:
      void init(IdxT capacity) ;
      IdxT allocateNode() ;
   } ;

/************************************************************************/
/************************************************************************/

// a trie with a variable list of items as node values

template <typename T, typename IdxT = std::uint32_t>
class MultiTrie
   {
   public:
      MultiTrie() ;
      ~MultiTrie() ;

   protected:

   } ;

/************************************************************************/
/************************************************************************/

template <typename T, typename IdxT>
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

	    template <typename RetT = T>
	    typename std::enable_if<std::is_pointer<T>::value, RetT>::type
	    nodeValue() const { return (T)nullptr ; }
	    template <typename RetT = T>
	    typename std::enable_if<!std::is_pointer<T>::value, RetT>::type
	    nodeValue() const { return T(0) ; }

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
	    T nodeValue() const { return m_value ; }

	 protected:
	    T   m_value ;		// we allow non-leaf nodes to have values
         } ;
      // leaves don't need to store child pointers
      class LeafNode
         {
	 public:
	    LeafNode() ;
	    ~LeafNode() ;
	    T nodeValue() const { return m_value ; }

	 protected:
	    T     m_value ;
         } ;

      PackedTrie() ;
      PackedTrie(const Trie<T,IdxT>*) ;
      PackedTrie(const char *filename) ;
      PackedTrie(const PackedTrie&) = delete ;
      ~PackedTrie() ;
      PackedTrie& operator= (const PackedTrie&) = delete ;

      unsigned longestKey() const { return m_maxkey ; }
      bool extendKey(IdxT& index, uint8_t keybyte) const ;
      bool find(const uint8_t* key, unsigned keylength, T& value) const ;
      bool leafNode(IdxT& index) const ;
      bool nodeHasValue(IdxT& index) const ;
      T nodeValue(IdxT& index) const ;
      bool nodeValue(IdxT& index, T& value) const ;

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

      unsigned       m_maxkey ;
   } ;

/************************************************************************/
/************************************************************************/

// we can store an arbitrary list of same-typed items as the value of
//   a node in a packed trie by storing an index in the node and
//   having a separate array of items

template <typename T, typename IdxT = std::uint32_t, typename ValIdxT = std::uint32_t>
class PackedMultiTrie : public PackedTrie<IdxT,ValIdxT>
   {
   public:
      PackedMultiTrie() ;
      ~PackedMultiTrie() ;

   protected:
      T*          m_values ;

   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !__Fr_TRIE_H_INCLUDED */

// end of file trie.h //
