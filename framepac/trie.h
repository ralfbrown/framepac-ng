/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-10					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018,2019 Carnegie Mellon University		*/
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

#include <cstdarg>
#include <mutex>
#include <type_traits>
#include "framepac/byteorder.h"
#include "framepac/itempool.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdxT, unsigned bits>
class TrieNodeValueless
   {
   public:
      static constexpr IdxT NULL_INDEX = (IdxT)0 ;
   public:
      TrieNodeValueless() { std::fill_n(m_children,1<<bits,NULL_INDEX) ; }
      TrieNodeValueless(const TrieNodeValueless&) = default ;
      ~TrieNodeValueless() {}
      TrieNodeValueless& operator= (const TrieNodeValueless&) = default ;

      bool hasChildren() const
	 {
	    for (size_t i = 0 ; i < lengthof(m_children) ; ++i)
	       {
	       if (m_children[i] != NULL_INDEX) return true ;
	       }
	    return false ;
	 }
      bool hasChild(unsigned N) const { return m_children[N] != NULL_INDEX ; }
      IdxT childIndex(unsigned N) const { return m_children[N] ; }

      IdxT  setChild(unsigned N, IdxT ch) ;
   protected:
      IdxT  m_children[1<<bits] ;
   } ;

// keep linker happy on debug builds:
template <typename IdxT, unsigned bits>
constexpr IdxT TrieNodeValueless<IdxT,bits>::NULL_INDEX; 

//----------------------------------------------------------------------------

template <typename T, typename IdxT, unsigned bits>
class TrieNode : public TrieNodeValueless<IdxT,bits>
   {
   public: // types
      typedef TrieNodeValueless<IdxT,bits> super ;
   public:
      TrieNode() : super(), m_userdata(0), m_leaf(false) {}
      TrieNode(T v, bool lf = false) : super(), m_value(v), m_userdata(0), m_leaf(lf) {}
      TrieNode(const TrieNode&) = default ;
      ~TrieNode() {}

      bool  leaf() const { return m_leaf ; }
      void  markAsLeaf() { m_leaf = true ; }

      T value() const { return m_value ; }
      void setValue(T v) { m_value = v ; }

      uint16_t userData() const { return m_userdata ; }
      void setUserData(uint16_t u) { m_userdata = u ; }
   protected:
      T        m_value ;
      uint16_t m_userdata ;	// alignment constraints leave padding, so let user store data there
      bool     m_leaf ;
   } ;

//----------------------------------------------------------------------------

template <typename T, typename IdxT = std::uint32_t, unsigned bits=4>
class Trie
   {
   public:
      static constexpr IdxT ROOT_INDEX = (IdxT)0 ;
      static constexpr IdxT NULL_INDEX = TrieNodeValueless<IdxT,bits>::NULL_INDEX ;
      typedef bool EnumFunc(const uint8_t *key, unsigned keylen, T value, std::va_list user_args) ;
      typedef TrieNodeValueless<IdxT,bits> ValuelessNode ;
      typedef TrieNode<T,IdxT,bits> Node ;

      // construct an empty trie, optionally pre-allocating nodes
      Trie(IdxT cap = 4) : m_valueless(cap), m_nodes(cap), m_maxkey(0) { (void) allocNode() ; }
      ~Trie() = default ;

      template <typename RetT = T>
      constexpr static typename std::enable_if<std::is_pointer<T>::value, RetT>::type
      nullVal() { return nullptr ; }
      template <typename RetT = T>
      constexpr static typename std::enable_if<!std::is_pointer<T>::value, RetT>::type
      nullVal() { return (RetT)0 ; }

      bool insert(const uint8_t* key, unsigned keylength, T value) ;
      bool insert(const uint8_t* key, unsigned keylength, T value, uint16_t user_data) ;
      bool extendKey(IdxT& index, uint8_t keybyte) const ;
      IdxT findNode(const uint8_t* key, unsigned keylength) const ;
      T find(const uint8_t* key, unsigned keylength) const ;
      bool contains(const uint8_t* key, unsigned keylength) const ;

      IdxT size() const { return fullNodes() + valuelessNodes() ; }
      IdxT capacity() const { return m_valueless.capacity() + m_nodes.capacity() ; }
      IdxT fullNodes() const { return m_nodes.size() ; }
      IdxT valuelessNodes() const { return m_valueless.size() ; }
      IdxT terminalNodes() const  ; // number of leaf (value-containing) nodes without children
      unsigned longestKey() const { return m_maxkey ; }

      bool enumerateVA(EnumFunc* fn, std::va_list args) const ;
      bool enumerate(EnumFunc* fn, ...) const
	 {
	    std::va_list args ;
	    va_start(args,fn) ;
	    bool status = enumerateVA(fn,args) ;
	    va_end(args) ;
	    return status ;
	 }

      Node* rootNode() const { return m_nodes.item(ROOT_INDEX) ; }
      Node* node(IdxT N) const { return m_nodes.item(N) ; }

   protected:
      ItemPool<ValuelessNode> m_valueless ;
      ItemPool<Node>	      m_nodes ;
      unsigned                m_maxkey ;
   private:
      IdxT allocNode() { return (IdxT)m_nodes.alloc() ; }
      IdxT allocValuelessNode() { return (IdxT)m_valueless.alloc() ; }
      void releaseNode(IdxT index) { m_nodes.release(index) ; }
      void releaseValuelessNode(IdxT index) { m_valueless.release(index) ; }
      ValuelessNode* valuelessNode(IdxT N) const { return m_valueless.item(N) ; }
      void insertChild(IdxT& index, uint8_t keybyte) ;
      bool enumerateVA(uint8_t* keybuf, size_t keylen, IdxT node, EnumFunc* fn, std::va_list args) const  ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename T, typename IdxT = std::uint32_t, unsigned bits=4>
class TrieCursor
   {
   public:
      typedef Trie<T,IdxT,bits> trie ;
   public:
      TrieCursor(const trie* t) : m_trie(t) { resetKey() ; }

      // manipulators
      void resetKey() { m_index = trie::ROOT_INDEX ; m_keylen = 0 ; m_failed = false ; }
      bool extendKey(uint8_t keybyte)
	 {
	    if (m_failed) return false ;
	    bool success = m_trie->extendKey(m_index,keybyte) ;
	    if (success)
	       ++m_keylen ;
	    else
	       m_failed = true ;
	    return success ;
	 }

      // accessors
      bool OK() const
	 {
	    if (m_failed) return false ;
	    auto n = m_trie->node(m_index) ;
	    return n && n->leaf() ;
	 }
      unsigned keyLength() const { return m_keylen ; }
      typename trie::Node* node() const { return m_failed ? nullptr : m_trie->node(m_index) ; }
      bool hasExtension(uint8_t keybyte) const
	 {
	    IdxT index = m_index ;
	    return m_trie->extendKey(index,keybyte) ;
	 }
      
   private:
      const trie* m_trie ;
      IdxT        m_index ;
      unsigned    m_keylen ;
      bool        m_failed ;
   } ;

/************************************************************************/
/************************************************************************/

// a trie with a variable list of items as node values

template <typename T, typename IdxT = std::uint32_t, unsigned bits=4>
class MultiTrie : public Trie<IdxT,IdxT,bits>
   {
   public:
      typedef Trie<IdxT,IdxT,bits> super ;
      class TList
	 {
	 public:
	    T    m_item ;
	    IdxT m_next ;
	 } ;
   public:
      MultiTrie() : super() {}
      ~MultiTrie() ;

   protected:
      ItemPool<TList>  m_values ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename T, typename IdxT, unsigned bits=4>
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
					// m_children is a bitmask of the nonzero children, which we use
					//   to compute the offset from the first child
         } ;
      // a full node can contain both a value and child pointers
      class Node : public ValuelessNode
         {
	 public: // types
	    typedef ValuelessNode super ;
	 public:
	    Node() {}
	    Node(T val) { m_value = val ; }
	    ~Node() {}
	    void setValue(T val) { m_value = val ; }
	    T nodeValue() const { return m_value ; }

	 protected:
	    T   m_value ;		// we allow non-leaf nodes to have values
         } ;
      // leaves don't need to store child pointers
      class LeafNode
         {
	 public:
	    LeafNode() ;
	    LeafNode(T val) { m_value = val ; }
	    ~LeafNode() ;
	    void setValue(T val) { m_value = val ; }
	    T nodeValue() const { return m_value ; }

	 protected:
	    T     m_value ;
         } ;

      PackedTrie() ;
      PackedTrie(const Trie<T,IdxT,bits>*) ;
      PackedTrie(const char* filename) ;
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
      void init(IdxT cap) ;

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

template <typename T, typename IdxT = std::uint32_t, typename ValIdxT = std::uint32_t, unsigned bits=4>
class PackedMultiTrie : public PackedTrie<IdxT,ValIdxT>
   {
   public: // types
      typedef PackedTrie<IdxT,ValIdxT> super ;
   public:
      PackedMultiTrie() ;
      ~PackedMultiTrie() ;

   protected:
      T*          m_values ;

   } ;

//----------------------------------------------------------------------------

// instantiations for which we have separately-compiled modules

typedef Trie<std::uint32_t, std::uint32_t> NybbleTrieInteger ;
extern template class Trie<std::uint32_t, std::uint32_t> ;
typedef Trie<double, std::uint32_t> NybbleTrieFloat ;
extern template class Trie<double, std::uint32_t> ;

typedef PackedTrie<std::uint32_t, std::uint32_t> PackedTrieInteger ;
extern template class PackedTrie<std::uint32_t, std::uint32_t> ;

} // end namespace Fr

#endif /* !__Fr_TRIE_H_INCLUDED */

// end of file trie.h //
