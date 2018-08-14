/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-14					*/
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

#ifndef _Fr_BIDINDEX_H_INCLUDED
#define _Fr_BIDINDEX_H_INCLUDED

#include "framepac/hashtable.h"

namespace Fr
{

// forward declaration to avoid pulling in all of framepac/file.h
class CFile ;

// The naive approach to having a bidirectional mapping between keys
// and integral indices is to have an array for the index->key
// direction and a hashmap from key->index.  But by passing a pointer
// to the array into the hashmap code, we can instead use a hashset of
// the indices and indirect through the array to get the key whenever
// the hash lookup needs the key's value, saving us a copy of (the
// pointer to) the key.  TODO
//

template <class keyT, typename idxT>
class BidirIndex : public HashTable<keyT,idxT>
   {
   public:
      typedef HashTable<keyT,idxT> super ;

   public:
      static BidirIndex* create(size_t initial_size = 1000) { return new BidirIndex(initial_size) ; }
      BidirIndex(const BidirIndex&) = delete ;
      BidirIndex& operator= (const BidirIndex&) = delete ;

      bool load(const char* filename, bool allow_mmap = true) ;
      bool load(CFile&, const char* filename, bool allow_mmap = true) ;
      bool loadMapped(const char* filename) ;
      bool loadFromMmap(const void* mmap_base, size_t mmap_len) ;
      bool save(const char* filename) const ;
      bool save(CFile&) const ;

      void clear() ;

      bool findKey(keyT key, idxT* id) const { return lookup(key,id) ; }
      idxT addKey(keyT key) ;
      void addKeySilent(keyT key) ;
      bool finalize() ; // generate the reverse index from the hash table

      idxT indexSize() const { return m_max_index ; }
      bool readonly() const { return m_readonly ; }

      idxT getIndex(keyT key) const { idxT index ; return lookup(key,&index) ? index : m_errorID ; }
      keyT getKey(idxT index) const { return index < m_max_index ? m_reverse_index[index] : keyT(0) ; }

   protected: // construction/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      BidirIndex(size_t initial_size = 1000) : super(initial_size) {}
      ~BidirIndex() { delete[] m_reverse_index ; }

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<BidirIndex> ;

      // type determination predicates
      static const char *typeName_(const Object*) { return "BidirIndex" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) { return nullptr ; } //TODO
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<BidirIndex*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { free_(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*, char* buffer, size_t buflen,
	 size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
	 size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return static_cast<const BidirIndex*>(obj)->indexSize() ; }
      static bool empty_(const Object* obj) { return static_cast<const BidirIndex*>(obj)->indexSize() == 0 ; }

      // *** standard access functions ***
      static Object* front_(Object* obj)
	 { return (Object*)static_cast<const BidirIndex*>(obj)->m_reverse_index[0] ; }
      static const Object* front_(const Object *obj)
	 { return (Object*)static_cast<const BidirIndex*>(obj)->m_reverse_index[0] ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

   protected:
      using HashTable<keyT,idxT>::lookup ;
      using HashTable<keyT,idxT>::contains ;
      void clearReverseIndex(keyT*, idxT common, idxT total) ;
      static void clearReverseElement(keyT*) ;
      static void releaseCommonBuffer(keyT*) ;

   protected: // data members
      atomic<idxT>  m_next_index { 0 } ;
      idxT          m_max_index { 0 } ;
      idxT          m_errorID { (idxT)-1 } ;
      keyT*         m_reverse_index { nullptr } ;
      idxT	    m_common_buffer { 0 } ;	// the first N elts of m_reverse_index share storage
      bool	    m_readonly { false } ;
      bool          m_external_storage { false } ;

      // magic values for serializing
      static constexpr auto signature = "\x7F""BiDIndex" ;
      static constexpr unsigned file_format = 1 ;
      static constexpr unsigned min_file_format = 1 ;

   private: // static members
      static Allocator s_allocator ;
   } ;

} // end namespace Fr

#endif /* !_Fr_BIDINDEX_H_INCLUDED */

// end of file bidindex.h //
