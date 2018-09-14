/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.12, last edit 2018-09-12					*/
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

#ifndef __Fr_HASHTABLE_H_INCLUDED
#define __Fr_HASHTABLE_H_INCLUDED

#include <cstdarg>
#include <cstdint>
#include <climits>
#include <iostream>
#include <type_traits>
#include <utility>
#include "framepac/counter.h"
#include "framepac/init.h"
#include "framepac/list.h"
#include "framepac/number.h"
#include "framepac/queue_mpsc.h"
#include "framepac/semaphore.h"
#include "framepac/symbol.h"
#include "framepac/synchevent.h"

#ifdef FrHASHTABLE_USE_FASTHASH64
#  include "framepac/fasthash64.h"
#endif /* FrHASHTABLE_USE_FASTHASH64 */

//#undef FrHASHTABLE_VERBOSITY
//#define FrHASHTABLE_VERBOSITY 2

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#define FrHASHTABLE_MIN_INCREMENT 256

#define FrHASHTABLE_MIN_SIZE      1031	// minimum user-specifiable size (number of hash buckets)

// starting up the copying of a segment of the current hash array into
//   a new hash table is fairly expensive, so enforce a minimum size
//   per segment
#define FrHASHTABLE_SEGMENT_SIZE 2048

/************************************************************************/

// if we're trying to eke out every last bit of performance by disabling
//   all debugging code, also turn off statistics collection
#ifdef NDEBUG
#  undef FrHASHTABLE_STATS
#endif /* NDEBUG */

// for RedHat's GCC 4.4.7
#ifndef INT8_MIN 
#  define INT8_MIN (-128)
#  define INT16_MIN (-32768)
#endif

#ifndef INT8_MAX
#  define INT8_MAX 127
#  define INT16_MAX 32767
#endif

#define FrHASHTABLE_SEARCHRANGE 0x0FFE

#ifdef FrHASHTABLE_STATS
#  define if_HASHSTATS(x) x
#  define INCR_COUNT(x) (++ s_stats->x)
#  define INCR_COUNT_if(cond,x) if (cond) {++ s_stats->x ; }
#  define DECR_COUNT(x) (-- s_stats->x)
#else
#  define if_HASHSTATS(x)
#  define INCR_COUNT(x)
#  define INCR_COUNT_if(cond,x)
#  define DECR_COUNT(x)
#endif

#ifndef ANNOTATE_BENIGN_RACE_STATIC
# define ANNOTATE_BENIGN_RACE_STATIC(var, desc) static var ;
# define ANNOTATE_UNPROTECTED_READ(x) x
#endif

#ifdef FrHASHTABLE_INTERLEAVED_ENTRIES
#  define if_INTERLEAVED(x) x
#  define ifnot_INTERLEAVED(x)
#else
#  define if_INTERLEAVED(x)
#  define ifnot_INTERLEAVED(x) x
#endif /* FrHASHTABLE_INTERLEAVED_ENTRIES */

#ifdef FrHASHTABLE_USE_FASTHASH64
#  define FrHASHTABLE_INT_HASH(k) (size_t)FramepaC::fasthash64_int(k)
#else
#  define FrHASHTABLE_INT_HASH(k) ((size_t)k)
#endif /* FrHASHTABLE_USE_FASTHASH64 */

/************************************************************************/

namespace FramepaC
{

extern size_t initial_indent ;
extern size_t display_width ;
extern size_t small_primes[] ;

extern thread_local size_t my_job_id ;

/************************************************************************/
/*	Structure to store HashTable Statistics				*/
/************************************************************************/

class HashTable_Stats
   {
   public:
      size_t insert ;
      size_t insert_attempt ;
      size_t insert_forwarded ;
      size_t insert_resize ;
      size_t remove ;
      size_t remove_found ;
      size_t remove_forwarded ;
      size_t contains ;
      size_t contains_found ;
      size_t contains_forwarded ;
      size_t lookup ;
      size_t lookup_found ;
      size_t lookup_forwarded ;
      size_t reclaim ;
      size_t spin ;
      size_t yield ;
      uint32_t sleep ;
      uint32_t insert_dup ;
      uint32_t CAS_coll ;
      uint32_t neighborhood_full ;
      uint32_t resize ;
      uint32_t resize_assist ;
      uint32_t resize_cleanup ;
      uint32_t resize_wait ;
      uint32_t none ;			// dummy for use in macros that require a counter name
   public:
      void clear() ;
      void add(const HashTable_Stats *other_stats) ;
   } ;

/************************************************************************/
/*      Auxiliary class for hash pointer heads				*/
/************************************************************************/

// encapsulate the chaining links
typedef uint16_t Link ;

static constexpr Link NULLPTR = 0x0FFF ;

class HashPtr
   {
   public:
      HashPtr() : m_first(NULLPTR), m_next(NULLPTR) {}
      ~HashPtr() {}
      // accessors
      Link first() const { return m_first.load() & link_mask ; }
      Link firstAndStatus(Link& stat) const
	 {
	 Link val = m_first.load() ;
	 stat = val & ~link_mask ;
	 return val & link_mask ;
	 }
      Link next() const { return m_next.load() & link_mask ; }
      static bool stale(Link stat) { return (stat & stale_mask) != 0 ; }
      bool copyDone() const { return (m_first.load() & copied_mask) != 0 ; }
      static bool copyDone(Link stat) { return (stat & copied_mask) != 0 ; }
      bool inUse() const { return (m_first.load() & inuse_mask) != 0 ; }
      bool reclaiming() const { return (m_first.load() & reclaim_mask) != 0 ; }

      // modifiers
      void first(Link ofs) { m_first.store((m_first.load() & ~link_mask) | ofs) ; }
      bool first(Link new_ofs, Link expected)
	 {
	    Link stat = m_first.load() & ~link_mask ;
	    new_ofs = (new_ofs /*& link_mask*/) | stat ;
	    expected = (expected /*& link_mask*/) | stat ;
	    return m_first.compare_exchange_weak(expected,new_ofs) ;
	 }
      void next(Link ofs) { m_next.store((m_next.load() & ~link_mask) | ofs) ; }
#if 0
      bool next(Link new_ofs, Link expected)
	 {
	    Link stat = m_next.load() & ~link_mask ;
	    new_ofs = (new_ofs /*& link_mask*/) | stat ;
	    expected = (expected /*& link_mask*/) | stat ;
	    return m_next.compare_exchange_strong(expected,new_ofs) ;
	 }
#endif
      bool markStale() { return m_first.test_and_set_bit(stale_bit) ; }
      bool markUsed() { return m_first.test_and_set_bit(inuse_bit) ; }
      void markFree() { (void)m_first.fetch_and_relax(~inuse_mask) ; }
      Link markStaleGetStatus() { return m_first.fetch_or(stale_mask) & ~link_mask ; }
      void markCopyDone() { (void)m_first.fetch_or(copied_mask) ; }
      bool markReclaiming() { return m_first.test_and_set_bit(reclaim_bit) ; }
      void markReclaimed() { m_first.fetch_and_relax((Link)~reclaim_mask) ; }

      
   protected:
      Fr::Atomic<Link> m_first ;	// offset of first entry in hash bucket
      Fr::Atomic<Link> m_next ;		// pointer to next entry in current chain

      static constexpr Link link_mask = 0x0FFF ;
      // flag bits in m_first
      static constexpr unsigned stale_bit = 15 ;
      static constexpr Link stale_mask = (1U << stale_bit) ;
      static constexpr unsigned copied_bit = 14 ;
      static constexpr Link copied_mask = (1U << copied_bit) ;
      static constexpr unsigned inuse_bit = 13 ;
      static constexpr Link inuse_mask = (1U << inuse_bit) ;
      static constexpr unsigned reclaim_bit = 12 ;
      static constexpr Link reclaim_mask = (1U << reclaim_bit) ;
      // flag bits in m_next
   } ;

/************************************************************************/
/*	Base class for the actual hash array template class		*/
/************************************************************************/

class HashBase
   {
   public:
      HashBase() ;
      HashBase(size_t cap) ;
      ~HashBase() ;

      HashBase *next() const { return m_next.load() ; }
      size_t capacity() const { return  m_fullsize ; }
      bool superseded() const { return m_next.load() != nullptr ; }
      bool resizingDone() const { return m_resizedone.load() ; }

      void setNext(HashBase* nxt) { m_next.store(nxt) ; }

      // ========== STL compatibility ==========
      size_t bucket_count() const { return m_size ; }
      size_t max_bucket_count() const { return m_fullsize ; }

   protected:
      Fr::Atomic<HashBase*> m_next ;		// the table which supersedes us
      ifnot_INTERLEAVED(HashPtr *m_ptrs ;)	// links chaining elements of a hash bucket [unchanging]
      size_t	           m_size ;		// capacity of hash array [constant for life of table]
      size_t	           m_fullsize ;		// capacity including padding [constant for life of table]
      Fr::Atomic<size_t>   m_first_incomplete { ~0U } ;
      Fr::Atomic<size_t>   m_last_incomplete { 0 } ;
      Fr::Atomic<uint32_t> m_segments_total { 0 } ;
      Fr::Atomic<uint32_t> m_segments_assigned { 0 } ;
      Fr::SynchEvent       m_resizestarted ;
      Fr::SynchEventCountdown m_resizepending ;
      Fr::Atomic<bool>     m_resizelock { false } ; // ensures that only one thread can initiate a resize
      Fr::Atomic<bool>     m_resizedone { false } ;
   } ;

//----------------------------------------------------------------------------

class TablePtr
   {
   public:
      TablePtr*	             m_next  { nullptr } ;
      Fr::Atomic<HashBase*>* m_table { nullptr } ;
      size_t   	             m_id    { 0 } ;	   // thread ID, for help in debugging
   public:
      TablePtr() : m_next(nullptr), m_table(nullptr), m_id(0) {}
      bool initialized() const { return m_table != nullptr ; }
      void clear()
	 { m_table = nullptr ; m_next = nullptr ; }
      void init(HashBase** tab, TablePtr* next)
	 {
	    m_table = reinterpret_cast<Fr::Atomic<HashBase*>*>(tab) ;
	    m_next = next ; m_id = FramepaC::my_job_id ;
	 }
      void init(Fr::Atomic<HashBase*>* tab, TablePtr* next)
	 { m_table = tab ; m_next = next ; m_id = FramepaC::my_job_id ; }
      const HashBase* table() const { return m_table->load() ; }
   } ;

//----------------------------------------------------------------------------

} // end namespace FramepaC

/************************************************************************/
/************************************************************************/

namespace Fr
{

// forward declarations
template <typename KeyT, typename ValT, typename RetT> class HashTableIter ;
template <typename KeyT, typename ValT, typename RetT> class HashTableLocalIter ;

/************************************************************************/
/*	Helper functions						*/
/************************************************************************/

#if FrHASHTABLE_VERBOSITY > 0
#include <cstdio>
#endif /* FrHASHTABLE_VERBOSITY > 0 */

/************************************************************************/
/*	Declarations for class HashTableBase				*/
/************************************************************************/

class HashTableBase : public Object
   {
   public: // types
      typedef Object super ;
   public:
      HashTableBase(bool (*assist)(HashTableBase*) = nullptr) : m_active_resizes(0), m_assist(assist) {}
      ~HashTableBase() ;

      void startResize() ;
      void finishResize() ;

      unsigned activeResizes() const { return m_active_resizes ; }
      bool assistResize() { return m_assist ? m_assist(this) : false ; }

   protected:
      atom_int	     m_active_resizes ;
      bool         (*m_assist)(HashTableBase*) { nullptr } ;
   } ;

/************************************************************************/
/*	Declarations for class HashTableHelper				*/
/************************************************************************/

class HashTableHelper
   {
   public:
      static bool startHelper(HashTableBase* ht) ;

   protected:  // internal methods
      HashTableHelper() {}
      ~HashTableHelper() {}

      static void initialize() ;
      [[gnu::noreturn]] static void helperFunction() ;

   protected:
      static Semaphore	                 s_semaphore ;
      static MPSC_Queue<HashTableBase*>  s_queue ;
      static atom_flag                   s_initialized ;
   } ;

inline HashTableBase::~HashTableBase()
{ while (m_active_resizes.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1)) ; }

/************************************************************************/
/*	Declarations for template class HashTable			*/
/************************************************************************/

template <typename KeyT, typename ValT>
class HashTable : public HashTableBase
   {
   public:
      typedef HashTableBase super ;
      // typedefs for compatibility with C++ STL
      typedef KeyT key_type ;
      typedef ValT mappped_type ;
      typedef std::pair<const KeyT,ValT> value_type ;
      typedef std::size_t size_type ;
      typedef std::ptrdiff_t difference_type ;
      //typedef X hasher ;	// specialize HashTable::hashVal() instead
      //typedef Y key_equal ;	// specialize HashTable::isEqual() instead
      typedef ValT& reference ;
      typedef const ValT& const_reference ;
      typedef HashTableIter<KeyT,ValT,ValT> iterator ;
      typedef HashTableIter<KeyT,ValT,const ValT> const_iterator ;  //FIXME??
      typedef HashTableLocalIter<KeyT,ValT,ValT> local_iterator ;
      typedef HashTableLocalIter<KeyT,ValT,const ValT> const_local_iterator ;
      // unsupported features of C++ unordered_map/unordered_set:
      //  1. nodes are not individually allocated, so there is no way to have a custom allocator
      //   typedef UNAVAILABLE allocator_type ;
      //   typedef UNAVAILABLE pointer ;     // std::allocator_traits<Allocator>::pointer
      //   typedef UNAVAILABLE const_pointer ;
      //  2. because nodes are not individually allocated, we also can't have a C++17 node_handle
      //     which causes destruction of node
      //   typedef UNAVAILABLE node_handle ;
      
      // incorporate the auxiliary classes
      typedef FramepaC::Link Link ;
      typedef FramepaC::HashPtr HashPtr ;
      typedef FramepaC::TablePtr TablePtr ;
      typedef FramepaC::HashTable_Stats HashTable_Stats ;

      // the types of the various callback functions
      typedef void HashTableCleanupFunc(HashTable* ht, KeyT key, ValT value) ;
      typedef bool HashKVFunc(KeyT key, ValT value) ;
      typedef bool HashKeyValueFunc(KeyT key, ValT value, std::va_list) ;
      typedef bool HashKeyPtrFunc(KeyT key, ValT *, std::va_list) ;

      // avoid 0-as-null-pointer warnings from compiler
      template <typename RetT = KeyT>
      constexpr static typename std::enable_if<std::is_pointer<KeyT>::value, RetT>::type
      nullKey() { return nullptr ; }
      template <typename RetT = KeyT>
      constexpr static typename std::enable_if<!std::is_pointer<KeyT>::value, RetT>::type
      nullKey() { return (RetT)0 ; }
      template <typename RetT = ValT>
      constexpr static typename std::enable_if<std::is_pointer<ValT>::value, RetT>::type
      nullVal() { return nullptr ; }
      template <typename RetT = ValT>
      constexpr static typename std::enable_if<!std::is_pointer<ValT>::value, RetT>::type
      nullVal() { return (RetT)0 ; }

      // encapsulate a hash table entry
      class Entry
      {
      public: // methods
	 void *operator new(size_t,void *where) { return where ; }
	 template <typename K_T = KeyT>
	 static typename std::enable_if<std::is_pointer<KeyT>::value,K_T>::type
	 copy(KeyT obj) { return obj ? static_cast<KeyT>(obj->clone().move()) : nullKey() ; }
	 template <typename K_T = KeyT>
	 static typename std::enable_if<!std::is_pointer<KeyT>::value,K_T>::type
	 copy(KeyT obj) { return KeyT(obj) ; }
	 void init()
	    {
	       m_key = DELETED() ;
	       setValue(nullVal()) ; 
	    }
	 void init(const Entry &entry)
	    {
	       if_INTERLEAVED(m_info = entry.m_info) ;
	       m_key = entry.m_key ;
	       setValue(entry.getValue()) ;
	    }

	 // constructors/destructors
	 Entry() if_INTERLEAVED(: m_info()) { init() ; }
	 ~Entry() {}

	 // variable-setting functions
	 template <typename V_T = ValT>
	 void setValue(typename std::enable_if<std::is_empty<ValT>::value,V_T>::type /*value*/) {}
	 template <typename V_T = ValT>
	 void setValue(typename std::enable_if<!std::is_empty<ValT>::value,V_T>::type value)
	    { m_value[0] = value ; }

	 template <typename RetT = ValT>
	 typename std::enable_if<std::is_empty<ValT>::value,RetT>::type
	 incrCount(size_t incr) { return nullVal() ; }
	 template <typename RetT = ValT>
	 typename std::enable_if<!std::is_empty<ValT>::value,RetT>::type
	 incrCount(size_t incr) { return m_value[0] += incr ; }

	 template <typename RetT = ValT>
	 typename std::enable_if<std::is_empty<ValT>::value,RetT>::type
	 atomicIncrCount(size_t) { return nullVal() ; }
	 template <typename RetT = ValT>
	 typename std::enable_if<!std::is_empty<ValT>::value,RetT>::type
	 atomicIncrCount(size_t incr)
	    { return Fr::Atomic<ValT>::ref(m_value[0]) += incr ; }

	 // access to internal state
	 KeyT getKey() const { return m_key.load() ; }
	 void setKey(KeyT newkey) { m_key.store(newkey) ; }
	 bool updateKey(KeyT expected, KeyT desired)
	    { return m_key.compare_exchange_strong(expected,desired) ; }
	 KeyT copyName() const { return copy(m_key) ; }
	 bool isActive() const { return m_key.load() != DELETED() ; }

	 template <typename RetT = ValT>
	 typename std::enable_if<std::is_empty<ValT>::value,RetT>::type
	 getValue() const { return nullVal() ; }
	 template <typename RetT = ValT>
	 typename std::enable_if<!std::is_empty<ValT>::value,RetT>::type
	 getValue() const { return m_value[0]; }

	 template <typename RetT = ValT>
	 typename std::enable_if<std::is_empty<ValT>::value,RetT>::type*
	 getValuePtr() { return (ValT*)nullptr ; }
	 template <typename RetT = ValT>
	 typename std::enable_if<!std::is_empty<ValT>::value,RetT>::type*
	 getValuePtr() { return &m_value[0] ; }

	 template <typename RetT = ValT>
	 typename std::enable_if<!std::is_empty<ValT>::value,RetT>::type
	 swapValue(ValT new_value)
	    { return Atomic<ValT>::ref(*getValuePtr()).exchange(new_value) ; }
	 template <typename RetT = ValT>
	 typename std::enable_if<std::is_empty<ValT>::value,RetT>::type
	 swapValue(ValT) { return nullVal() ; }
	 ALWAYS_INLINE static constexpr KeyT DELETED() {return (KeyT)~0UL ; } 

      protected: // members
	 Atomic<KeyT> m_key ;
	 ValT         m_value[std::is_empty<ValT>::value ? 0 : 1] ;
      public: // data members
	 if_INTERLEAVED(HashPtr  m_info ;)
      } ;
      //------------------------
      // encapsulate all of the fields which must be atomically swapped at the end of a resize()
      class Table : public FramepaC::HashBase
      {
	 typedef class Fr::HashTable<KeyT,ValT> HT ;
      public:
	 void* operator new(size_t, Table* ptr) { return ptr ; }
	 Table() : HashBase(), m_entries(nullptr), m_container(nullptr), remove_fn(nullptr)
	    {
	       return;
	    }
	 Table(size_t size) : HashBase(size)
	    {
	       init() ;
	       return ;
	    } ;
	 ~Table() { cleanup() ; }
	 void init() ;
	 void cleanup() ;
	 bool good() const { return m_entries != nullptr ifnot_INTERLEAVED(&& m_ptrs != nullptr) && m_size > 0 ; }
	 Table* next() const { return static_cast<Table*>(this->HashBase::next()) ; }
	 // maintaining the count of elements in the table is too much of a bottleneck under high load,
	 //   so we'll punt and simply scan the hash array if someone needs that count
	 size_t currentSize() const { return countItems() ; }
	 KeyT getKey(size_t N) const { return m_entries[N].getKey() ; }
	 void setKey(size_t N, KeyT newkey) { m_entries[N].setKey(newkey) ; }
	 ValT getValue(size_t N) const { return m_entries[N].getValue() ; }
	 ValT *getValuePtr(size_t N) const { return m_entries[N].getValuePtr() ; }
	 void setValue(size_t N, ValT value) { m_entries[N].setValue(value) ; }
	 bool updateKey(size_t N, KeyT expected, KeyT desired)
	    { return m_entries[N].updateKey(expected,desired) ; }
	 bool activeEntry(size_t N) const { return m_entries[N].isActive() ; }
#ifndef FrHASHTABLE_INTERLEAVED_ENTRIES
	 HashPtr *bucketPtr(size_t N) const { return &m_ptrs[N] ; }
#else
	 HashPtr *bucketPtr(size_t N) const { return &m_entries[N].m_info ; }
#endif /* FrHASHTABLE_INTERLEAVED_ENTRIES */
	 bool chainCopied(size_t N) const { return bucketPtr(N)->copyDone() ; }
	 void copyEntry(size_t N, const Table *othertab)
	    {
	       m_entries[N].init(othertab->m_entries[N]) ;
	       ifnot_INTERLEAVED(m_ptrs[N] = othertab->m_ptrs[N]) ;
	       return ;
	    }
	 static size_t normalizeSize(size_t sz) ;
      protected:
	 // the following function is defined *after* the declaration of HashTable
	 //   due to the circular dependency of declarations....
	 void announceTable()
	    {
#ifndef FrSINGLE_THREADED
	       Atomic<Table*>& tbl = Atomic<Table*>::ref(Fr::HashTable<KeyT,ValT>::s_table) ;
	       tbl.store(this) ;
#endif /* !FrSINGLE_THREADED */
	       return ;
	    }

	 Link chainHead(size_t N) const { return bucketPtr(N)->first() ; }
	 Link chainHead(size_t N, Link& status) const { return bucketPtr(N)->firstAndStatus(status) ; }
	 Link chainNext(size_t N) const { return bucketPtr(N)->next() ; }
	 void setChainNext(size_t N, Link nxt) { bucketPtr(N)->next(nxt) ; }
	 void markCopyDone(size_t N) { bucketPtr(N)->markCopyDone() ; }
	 void autoResize() ;
      protected:
	 void removeValue(Entry &element) const
	    {
	       ValT value = element.swapValue(nullVal()) ;
	       if (remove_fn && value)
		  {
		  remove_fn(element.getKey(),value) ;
		  }
	       return ;
	    }
      public:
	 [[gnu::hot]] bool resizeCopySegments(size_t max_segs = ~0UL) ;
      protected:
	 [[gnu::hot]] bool copyChain(size_t bucketnum) ;
	 void waitUntilCopied(size_t bucketnum) ;
	 // copy the chains for buckets 'bucketnum' to 'endpos',
	 //   inclusive, and wait to ensure that all copying is
	 //   complete even if there are interfering operations in
	 //   progress
	 [[gnu::hot]] void copyChains(size_t bucketnum, size_t endpos) ;
	 // a separate version of add() for resizing, because Fr::SymbolTable needs to
	 //   look at the full key->hashValue(), not just 'key' itself
         bool reAdd(size_t hashval, KeyT key, ValT value = (ValT)0) ;

         [[gnu::hot]] Link locateEmptySlot(size_t bucketnum, Link hint = 0);

	 [[gnu::hot]] bool insertKey(size_t bucketnum, Link firstptr, KeyT key, ValT value) ;
	 [[gnu::hot]] void resizeCopySegment(size_t segnum) ;
	 void clearDuplicates(size_t bucketnum) ;

	 bool reclaimChain(size_t bucketnum) ;
	 void resizeCleanup() ;

	 template <typename RetT = Object*>
	 static typename std::enable_if<std::is_integral<KeyT>::value,RetT>::type makeObject(KeyT key)
	    { return Integer::create(key) ; }
	 template <typename RetT = Object*>
	 static typename std::enable_if<!std::is_integral<KeyT>::value,RetT>::type makeObject(KeyT key)
	    { return (Object*)key ; }

      public:
	 void onRemove(HashKVFunc *func) { remove_fn = func ; }
	 bool resize(size_t newsize) ;

	 [[gnu::hot]] bool add(size_t hashval, KeyT key, ValT value) ;
	 [[gnu::hot]] bool add(size_t hashval, KeyT key) { return add(hashval,key,nullVal()) ; }
	 [[gnu::hot]] ValT addCount(size_t hashval, KeyT key, size_t incr) ;

	 [[gnu::hot]] bool contains(size_t hashval, const KeyT key) const ;
	 [[gnu::hot]] ValT lookup(size_t hashval, KeyT key) const ;
	 [[gnu::hot]] bool lookup(size_t hashval, KeyT key, ValT *value) const ;
	 // NOTE: this lookup() is not entirely thread-safe if clear==true
	 [[gnu::hot]] bool lookup(size_t hashval, KeyT key, ValT *value, bool clear_entry) ;
	 // note: lookupValuePtr is not safe in the presence of parallel
	 //   add() and remove() calls!  Use global synchronization if
	 //   you will be using both this function and add()/remove()
	 //   concurrently on the same hash table.
	 ValT *lookupValuePtr(size_t hashval, KeyT key) const ;

	 bool remove(size_t hashval, KeyT key) ;
	 bool reclaimDeletions(size_t totalfrags, size_t fragnum) ;

	 // special support for Fr::SymHashSet
	 KeyT addKey(size_t hashval, const char* name, size_t namelen, bool* already_existed = nullptr) ;
	 // special support for Fr::SymbolTable
	 [[gnu::hot]] bool contains(size_t hashval, const char *name, size_t namelen) const ;
	 // special support for Fr::SymHashSet
	 [[gnu::hot]] KeyT lookupKey(size_t hashval, const char *name, size_t namelen) const ;

	 void replaceValue(size_t pos, ValT new_value) ;

	 // ========== STL compatibility ==========
	 void clear() ;
	 // insert()
	 // insert_or_assign()
	 // emplace()
	 // emplace_hint()
	 // try_emplace()
	 // ValT& at(const KeyT& key) throws(std::out_of_range) ;
	 // const ValT& at(const KeyT& key) const throws(std::out_of_range) ;
	 // ValT& operator[]( const Key& key) ;
	 // find()
	 // equal_range()

	 [[gnu::cold]] size_t bucket_size(size_t bucketnum) const ;
	 size_t bucket(KeyT key) const { return m_container->hashVal(key) % m_size ; }
	 float load_factor() const { return countItems() / m_size ; }

	 // rehash()
	 // reserve()
	 // hash_function
	 // key_eq

	 //============== Iterators ================
	 bool iterateVA(HashKeyValueFunc *func, std::va_list args) const ;
	 List *allKeys() const ;

	 //================= Content Statistics ===============
	 [[gnu::cold]] size_t countItems() const ;
	 [[gnu::cold]] size_t countItems(bool remove_dups) ;
	 [[gnu::cold]] size_t countDeletedItems() const ;
	 [[gnu::cold]] size_t* chainLengths(size_t &max_length) const ;
	 [[gnu::cold]] size_t* neighborhoodDensities(size_t &num_densities) const ;

	 //============== Debugging Support ================
	 [[gnu::cold]] bool verify() const ;

	 // ============= Object support =============

	 template <typename RetT = size_t>
	 typename std::enable_if<std::is_integral<KeyT>::value,RetT>::type
	 keyDisplayLength(KeyT key) const
	    { return snprintf(nullptr,0,"%ld%c",(ssize_t)key,(char)'\0') ; }
	 template <typename RetT = size_t>
	 typename std::enable_if<!std::is_integral<KeyT>::value,RetT>::type
	 keyDisplayLength(KeyT key) const
	    { return key ? key->cStringLength() + 1 : 3 ; }

	 template <typename RetT = char*>
	 typename std::enable_if<std::is_integral<KeyT>::value,RetT>::type
	 displayKeyValue(char *buffer, KeyT key) const
	    { return buffer + snprintf(buffer,50,"%ld%c",(ssize_t)key,(char)'\0') ; }
	 template <typename RetT = char*>
	 typename std::enable_if<!std::is_integral<KeyT>::value,RetT>::type
	 displayKeyValue(char *buffer, KeyT key) const
	    {
	       if (key)
		  {
		  size_t len = key->cStringLength() ;
		  if (key->toCstring(buffer,len))
		     buffer += len ;
		  *buffer = '\0' ;
		  return buffer ;
		  }
	       else
		  {
		  memcpy(buffer,"NIL",4) ;
		  return buffer + 4 ;
		  }
	    }

	 char *displayValue(char *buffer) const ;
	 // get size of buffer needed to display the string representation of the hash table
	 // NOTE: will not be valid if there are any add/remove/resize calls between calling
	 //   this function and using displayValue(); user must ensure locking if multithreaded
	 size_t cStringLength(size_t wrap_at, size_t indent) const ;

      public:
	 Entry*            m_entries ;			// hash array [unchanging]
	 HT*               m_container ;		// hash table for which this is the content [unchanging]
	 HashKVFunc*       remove_fn { nullptr } ; 	// invoke on removal of entry/value
	 static constexpr Link searchrange = FrHASHTABLE_SEARCHRANGE ; // full search window, starting at bucket head
	 static constexpr size_t NULLPOS = ~0UL ;
         } ;
      //------------------------
      class HazardLock
         {
	 private:
	 public:
#ifdef FrSINGLE_THREADED
	    ALWAYS_INLINE HazardLock(Table *) {}
	    ALWAYS_INLINE HazardLock(Atomic<Table*>) {}
	    ALWAYS_INLINE ~HazardLock() {}
#else
	    HazardLock(Atomic<Table*>tab)
	       {
	       Atomic<Table*>& tbl = Atomic<Table*>::ref(HashTable::s_table) ;
	       tbl.store(tab.load()) ;
	       }
	    HazardLock(Table *tab)
	       {
	       Atomic<Table*>& tbl = Atomic<Table*>::ref(HashTable::s_table) ;
	       tbl.store(tab) ;
	       }
	    ~HazardLock()
	       {
	       Atomic<Table*>& tbl = Atomic<Table*>::ref(HashTable::s_table) ;
	       tbl.store_relax((Table*)nullptr) ;
	       }
#endif /* FrSINGLE_THREADED */
         } ;
      //------------------------
   protected: // debug methods
#if FrHASHTABLE_VERBOSITY > 1
      [[gnu::format(printf,1,2)]] 
      static void debug_msg(const char *fmt, ...)
	 {
	    std::va_list args ;
	    va_start(args,fmt) ;
	    vfprintf(stderr,fmt,args) ;
	    va_end(args) ;
	    return ;
	 }
#else
      [[gnu::format(printf,1,2)]] 
      ALWAYS_INLINE static void debug_msg(const char *fmt, ...)
	 { (void)fmt ; }
#endif /* FrHASHTABLE_VERBOSITY > 1 */
   protected: // methods
      static void thread_backoff(size_t &loops) ;
      void init(size_t initial_size) ;
      static Table *allocTable() ;
      static void releaseTable(Table *t) ;
      void updateTable() ;

      // set up the per-thread info needed for safe reclamation of
      //   entries and hash arrays, as well as an on-exit callback
      //   to clear that info
   public: // should only be called from Initializer / ThreadInitializer
      static void threadInit() ;
      static void threadCleanup() ;
      static void StaticInitialization() ;
      static void StaticCleanup() ;
   protected:
      static bool stillLive(const Table *version) ;

      size_t maxSize() const { return m_table.load()->bucket_count() ; }
      template <typename RetT = size_t>
      inline typename std::enable_if<!std::is_integral<KeyT>::value,RetT>::type hashVal(KeyT key) const
	 {
	    return key ? key->hashValue() : 0 ;
	 }
      template <typename RetT = size_t>
      inline typename std::enable_if<std::is_integral<KeyT>::value,RetT>::type hashVal(KeyT key) const
	 {
	    return FrHASHTABLE_INT_HASH(key) ;
	 }
      inline size_t hashValFull(KeyT key) const
	 {
	    // separate version for use by resizeTo(), which needs to use the full
	    //   hash value and not just the key on Fr::SymHashSet
	    // this is the version for everyone *but* Fr::SymHashSet
	    return hashVal(key) ;
	 }
      // special support for Fr::SymHashSet
      size_t hashVal(const char *keyname, size_t *namelen) const
	 { return Symbol::hashValue(keyname,namelen) ; }
      template <typename RetT = bool>
      inline typename std::enable_if<std::is_integral<KeyT>::value,RetT>::type isEqual(KeyT key1, KeyT key2) const
	 {
 	    return key2 != Entry::DELETED() && key1 == key2 ;
	 }
      template <typename RetT = bool>
      inline typename std::enable_if<!std::is_integral<KeyT>::value,RetT>::type isEqual(KeyT key1, KeyT key2) const
	 {
	    return key2 != Entry::DELETED() && Fr::equal(key1,key2) ;
	 }
      inline bool isEqualFull(KeyT key1, KeyT key2) const
	 {
	    // separate version for use by resizeTo(), which needs to use the full
	    //   key name and not just the key's pointer on Fr::SymHashSet
	    // this is the version for everyone *but* Fr::SymHashSet
	    return isEqual(key1,key2) ;
	 }
      // special support for Fr::SymHashSet; must be overridden.  This is the default version for
      //   all key types except Symbol
      static bool isEqual(const char * /*keyname*/, size_t /*namelen*/, KeyT /*key*/) { return false ; }

      // ============== Definitions to reduce duplication ================
      // much of the HashTable API just calls through to Table after
      //   setting a hazard pointer
#ifdef FrSINGLE_THREADED
#define DELEGATE(delegate) \
            return m_table.load()->delegate ;
#define DELEGATE_HASH(delegate) 			\
	    size_t hashval = hashVal(key) ; 		\
            return m_table.load()->delegate ;
#else
#define DELEGATE(delegate) 						\
            HazardLock hl(m_table) ;					\
	    return m_table.load()->delegate ;
#define DELEGATE_HASH(delegate) 					\
	    size_t hashval = hashVal(key) ; 				\
	    HazardLock hl(m_table) ;					\
	    return m_table.load()->delegate ;
#endif /* FrSINGLE_THREADED */

   protected:
      HashTable(size_t initial_size = FrHASHTABLE_MIN_SIZE)
	 : HashTableBase(doAssistResize), m_table(nullptr)
	 {
	    init(initial_size) ;
	    return ;
	 }
      HashTable(const HashTable &ht) ;
      ~HashTable() ;

      // ============== The public API for HashTable ================
   public:
      bool load(CFile&, const char* filename) ;
      bool load(const char* mmap_base, size_t mmap_len) ;
      bool save(CFile&) const ;

      // get N instances of Table from the OS and place on our freelist
      static void preallocateTables(size_t N) ; 
      // release any unused Table instances
      static void freeTables() ;

      // *** object factories ***
      static HashTable* create(size_t initial_size = FrHASHTABLE_MIN_SIZE) { return new HashTable(initial_size) ; }
      static HashTable* create(const HashTable& ht) { return new HashTable(ht) ; }

      bool resizeTo(size_t newsize) { DELEGATE(resize(newsize)) ; }
      bool reserve_(size_t newsize) { DELEGATE(resize(newsize)) ; }

      bool reclaimDeletions(size_t totalfrags = 1, size_t fragnum = 0)
         { DELEGATE(reclaimDeletions(totalfrags,fragnum)) }

      [[gnu::hot]] bool add(KeyT key) { INCR_COUNT(insert) ; DELEGATE_HASH(add(hashval,key)) }
      [[gnu::hot]] bool add(KeyT key, ValT value) { INCR_COUNT(insert) ; DELEGATE_HASH(add(hashval,key,value)) }
      [[gnu::hot]] ValT addCount(KeyT key, size_t incr) { INCR_COUNT(insert) ; DELEGATE_HASH(addCount(hashval,key,incr)) }
      bool remove(KeyT key) { DELEGATE_HASH(remove(hashval,key)) }
      [[gnu::hot]] bool contains(const KeyT key) const { DELEGATE_HASH(contains(hashval,key)) }
      [[gnu::hot]] ValT lookup(KeyT key) const { DELEGATE_HASH(lookup(hashval,key)) }
      [[gnu::hot]] bool lookup(KeyT key, ValT *value) const { DELEGATE_HASH(lookup(hashval,key,value)) }
      // NOTE: this lookup() is not entirely thread-safe if clear==true
      [[gnu::hot]] bool lookup(KeyT key, ValT *value, bool clr) { DELEGATE_HASH(lookup(hashval,key,value,clr)) }
      // NOTE: lookupValuePtr is not safe in the presence of parallel
      //   add() and remove() calls!  Use global synchronization if
      //   you will be using both this function and add()/remove()
      //   concurrently on the same hash table.
      ValT *lookupValuePtr(KeyT key) const { DELEGATE_HASH(lookupValuePtr(hashval,key)) }
      [[gnu::hot]] bool add(KeyT key, ValT value, bool replace)
	 {
	    size_t hashval = hashVal(key) ;
	    HazardLock hl(m_table) ;
	    Table *tab = m_table.load() ;
	    if (replace)
	       tab->remove(hashval,key) ;
	    return tab->add(hashval,key,value) ;
	 }
      // special support for Fr::SymHashSet
      [[gnu::hot]] KeyT addKey(const char *name, bool *already_existed = nullptr)
	 {
	    INCR_COUNT(insert) ;
	    size_t namelen ;
	    size_t hashval = hashVal(name,&namelen) ;
	    DELEGATE(addKey(hashval, name, namelen, already_existed))
	 }
      // special support for Fr::SymbolTable
      [[gnu::hot]] bool contains(const char *name) const
         {
	    size_t namelen ;
	    size_t hashval = hashVal(name,&namelen) ;
	    DELEGATE(contains(hashval,name,namelen))
	 }
      // special support for Fr::SymHashSet
      [[gnu::hot]] KeyT lookupKey(const char *name, size_t namelen, size_t hashval) const
         { DELEGATE(lookupKey(hashval,name,namelen)) }
      // special support for Fr::SymHashSet
      [[gnu::hot]] KeyT lookupKey(const char *name) const
         {
	    if (!name)
	       return nullKey() ;
	    size_t namelen ;
	    size_t hashval = hashVal(name,&namelen) ;
	    DELEGATE(lookupKey(hashval,name,namelen))
	 }
      // special support for Fr::SymHashSet; this is the generic version for everyone except SymHashSet
      KeyT createSymbol(const char* /*name*/) const { abort() ; return nullKey() ; }
      // special support for Fr::SymHashSet; this is the generic version for everyone except SymHashSet
      void deleteSymbol(KeyT /*sym*/) const { abort() ; }

      [[gnu::cold]] size_t countItems(bool remove_dups = false) const
	 { DELEGATE(countItems(remove_dups)) }
      [[gnu::cold]] size_t countDeletedItems() const
	 { DELEGATE(countDeletedItems()) }
      [[gnu::cold]] size_t* chainLengths(size_t &max_length) const
         { DELEGATE(chainLengths(max_length)) }
      [[gnu::cold]] size_t* neighborhoodDensities(size_t &num_densities) const
	 { DELEGATE(neighborhoodDensities(num_densities)) }

      void* userData() const { return m_userdata ; }
      void userData(void* ud) { m_userdata = ud ; }

      // ========== STL compatibility ==========
      void clear()
	 { DELEGATE(clear()) }
      // std::pair<iterator,bool> insert( const value_type& value) ;
      // std::pair<iterator,bool> insert_or_assign(const key_type& key, value_type& val) ;
      // emplace()
      // emplace_hint()
      // try_emplace()
      size_t erase(const KeyT& key) { return remove(key) ? 1 : 0 ; }
      // ValT& at(const KeyT& key) ;
      // const ValT& at(const KeyT& key) const ;
      // operator[]
      size_t count(const KeyT& key) const { return contains(key) ? 1 : 0 ; }
      // iterator find(const KeyT& key) ;
      // const_iterator find(const KeyT& key) const ;
      // std::pair<iterator,iterator> equal_range(const KeyT& key) ;
      // std::pair<const_iterator,const_iterator> equal_range(const KeyT& key) const ;

      size_t bucket_count() const
	 { DELEGATE(bucket_count()) }
      size_t max_bucket_count() const
	 { DELEGATE(max_bucket_count()) }
      size_t bucket_size(size_t N) const
	 { DELEGATE(bucket_size(N)) }
      size_t bucket(KeyT key) const
	 { DELEGATE(bucket(key)) }

      void rehash(size_t cnt) { if (cnt) resizeTo(cnt) ; else reserve(countItems()) ; }
      void reserve(size_t cnt) { rehash((size_t)(1 + cnt / 0.9f)) ; }
      float load_factor() const
	 { DELEGATE(load_factor()) }

      // since automatic resizing happens due to filled neighborhoods, we just provide dummy max_load_factor
      //   functions
      float max_load_factor() const { return 1.0f ; }
      void max_load_factor(float) {}

      // hash_function
      // key_eq

      // ========== Iterators ===========
      inline iterator begin() const ;
      inline const_iterator cbegin() const ;
      inline iterator end() const ;
      inline const_iterator cend() const ;

      inline local_iterator begin(int bucket) const ;
      inline const_local_iterator cbegin(int bucket) const ;
      inline local_iterator end(int bucket) const ;
      inline const_local_iterator cend(int bucket) const ;

      bool iterateVA(HashKeyValueFunc *func, std::va_list args) const
         { DELEGATE(iterateVA(func,args)) }
      bool iterate(HashKeyValueFunc *func,...) const
	 {
	    std::va_list args ;
	    va_start(args,func) ;
	    bool success = iterateVA(func,args) ;
	    va_end(args) ;
	    return success ;
	 }
      List *allKeys() const { DELEGATE(allKeys()) }

      // set callback function to be invoked when the hash table is deleted
      void onDelete(HashTableCleanupFunc *func) { cleanup_fn = func ; }
      // set callback function to be invoked when an entry is deleted; this
      //   permits the associated value for the entry to be freed
      void onRemove(HashKVFunc *func)
         {
	 remove_fn = func ;
	 Table* tbl = m_table.load() ;
	 if (tbl) tbl->onRemove(func) ; 
	 }

      // access to internal state
      size_t currentSize() const { DELEGATE(countItems()) }
      size_t maxCapacity() const { DELEGATE(capacity()) }
      bool isPacked() const { return false ; }  // backwards compatibility
      HashTableCleanupFunc *cleanupFunc() const { return cleanup_fn ; }
      HashKVFunc *onRemoveFunc() const { return remove_fn ; }

      // =============== Background Processing =================
      static bool doAssistResize(HashTableBase*) ;

      // =============== Operational Statistics ================
      [[gnu::cold]] void clearGlobalStats()
	 {
	 if_HASHSTATS(m_stats.clear()) ;
	 return ;
	 }
      [[gnu::cold]] static void clearPerThreadStats()
	 {
	 if_HASHSTATS(if (s_stats) s_stats->clear()) ;
	 return ;
	 }
      [[gnu::cold]] void updateGlobalStats()
	 {
	 if_HASHSTATS(m_stats.add(s_stats)) ;
	 return ;
	 }
#ifdef FrHASHTABLE_STATS
      size_t numberOfInsertions() const { return m_stats.insert ; }
      size_t numberOfDupInsertions() const { return m_stats.insert_dup ; }
      size_t numberOfForwardedInsertions() const { return m_stats.insert_forwarded ; }
      size_t numberOfResizeInsertions() const { return m_stats.insert_resize ; }
      size_t numberOfInsertionAttempts() const { return  m_stats.insert_attempt ; }
      size_t numberOfRemovals() const { return m_stats.remove ; }
      size_t numberOfForwardedRemovals() const { return m_stats.remove_forwarded ; }
      size_t numberOfItemsRemoved() const { return m_stats.remove_found ; }
      size_t numberOfContainsCalls() const { return m_stats.contains ; }
      size_t numberOfSuccessfulContains() const { return m_stats.contains_found ; }
      size_t numberOfForwardedContains() const { return m_stats.contains_forwarded ; }
      size_t numberOfLookups() const { return  m_stats.lookup ; }
      size_t numberOfSuccessfulLookups() const { return  m_stats.lookup_found ; }
      size_t numberOfForwardedLookups() const { return m_stats.lookup_forwarded ; }
      size_t numberOfResizes() const { return  m_stats.resize ; }
      size_t numberOfResizeAssists() const { return  m_stats.resize_assist ; }
      size_t numberOfResizeCleanups() const { return  m_stats.resize_cleanup ; }
      size_t numberOfResizeWaits() const { return m_stats.resize_wait ; }
      size_t numberOfReclamations() const { return m_stats.reclaim ; }
      size_t numberOfFullNeighborhoods() const { return  m_stats.neighborhood_full ; }
      size_t numberOfCASCollisions() const { return m_stats.CAS_coll ; }
      size_t numberOfSpins() const { return  m_stats.spin ; }
      size_t numberOfYields() const { return m_stats.yield ; }
      size_t numberOfSleeps() const { return m_stats.sleep ; }
#else
      static size_t numberOfInsertions() { return 0 ; }
      static size_t numberOfDupInsertions() { return 0 ; }
      static size_t numberOfForwardedInsertions() { return 0 ; }
      static size_t numberOfInsertionAttempts() { return 0 ; }
      static size_t numberOfRemovals() { return 0 ; }
      static size_t numberOfForwardedRemovals() { return 0 ; }
      static size_t numberOfItemsRemoved() { return 0 ; }
      static size_t numberOfContainsCalls() { return 0 ; }
      static size_t numberOfSuccessfulContains() { return 0 ; }
      static size_t numberOfForwardedContains() { return 0 ; }
      static size_t numberOfLookups() { return 0 ; }
      static size_t numberOfSuccessfulLookups() { return 0 ; }
      static size_t numberOfForwardedLookups() { return 0 ; }
      static size_t numberOfResizes() { return 0 ; }
      static size_t numberOfResizeAssists() { return 0 ; }
      static size_t numberOfResizeCleanups() { return 0 ; }
      static size_t numberOfResizeWaits() { return 0 ; }
      static size_t numberOfReclamations() { return 0 ; }
      static size_t numberOfFullNeighborhoods() { return 0 ; }
      static size_t numberOfCASCollisions() { return 0 ; }
      static size_t numberOfSpins() { return 0 ; }
      static size_t numberOfYields() { return 0 ; }
      static size_t numberOfSleeps() { return 0 ; }
#endif

      // ============= Fr::Object support =============
      static ObjectPtr clone_(const Object* o) { return new HashTable(*static_cast<const HashTable*>(o)) ; }
      static Object* shallowCopy_(const Object* o) { return clone_(o) ; }
      static void free_(Object* o) { delete static_cast<HashTable*>(o) ; }
      static void shallowFree_(Object* o) { free_(o) ; }
      static size_t size_(const Object* o) { return static_cast<const HashTable*>(o)->currentSize() ; }
      static size_t hashValue_(const Object*) { return 0 ; } //FIXME

      // get size of buffer needed to display the string representation of the hash table
      // NOTE: will not be valid if there are any add/remove/resize calls between calling
      //   this function and using displayValue(); user must ensure locking if multithreaded
      size_t cStringLength_(size_t wrap_at, size_t indent) const { DELEGATE(cStringLength(wrap_at,indent)) ; }
      static size_t cStringLength_(const Object* o, size_t wrap_at, size_t indent, size_t /*wrapped_indent*/)
	 { return o ? static_cast<const HashTable*>(o)->cStringLength_(wrap_at,indent) : 0 ; }
      //TODO: convert displayValue to toCString_()
      char* displayValue(char *buffer) const { DELEGATE(displayValue(buffer)) ; }

      //  =========== Debugging Support ============
      [[gnu::cold]] bool verify() const { DELEGATE(verify()) }

   protected: // members of HashTable
      Atomic<Table*>   	    m_table ;			// pointer to currently-active m_tables[] entry
      Atomic<Table*>	    m_oldtables { nullptr } ;	// start of list of currently-live hash arrays
      HashTableCleanupFunc* cleanup_fn ;		// invoke on destruction of obj
      HashKVFunc*           remove_fn ; 		// invoke on removal of entry/value
      void*                 m_userdata ;		// available for use by isEqual, hashValue, hashValueFull
      static Fr::Initializer<HashTable> global_initializer ;
      static Atomic<FramepaC::HashBase*> s_freetables ;
#ifndef FrSINGLE_THREADED
      static Fr::ThreadInitializer<HashTable> initializer ;
      static TablePtr*    s_thread_entries ;
      static thread_local TablePtr* s_thread_record ;
      static thread_local Table*    s_table ; // thread's announcement which hash table it's using
#endif /* FrSINGLE_THREADED */
#ifdef FrHASHTABLE_STATS
      mutable HashTable_Stats	  m_stats ;
      static thread_local HashTable_Stats* s_stats ;
#endif /* FrHASHTABLE_STATS */

      // magic values for serializing
      static constexpr auto signature = "\x7FHshTable" ;
      static constexpr unsigned file_format = 1 ;
      static constexpr unsigned min_file_format = 1 ;
   private:
      friend class FramepaC::Object_VMT<HashTable> ;
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk) { s_allocator.release(blk) ; }
      static Allocator s_allocator ;
      static const char s_typename[] ;
   } ;

template <typename KeyT, typename ValT>
const char HashTable<KeyT,ValT>::s_typename[] = "HashTable" ;

template <typename KeyT, typename ValT>
//Atomic<HashTable<KeyT,ValT>::Table> HashTable<KeyT,ValT>::s_freetables = nullptr ;
Atomic<FramepaC::HashBase*> HashTable<KeyT,ValT>::s_freetables = nullptr ;

template <typename KeyT, typename ValT>
Allocator HashTable<KeyT,ValT>::s_allocator(FramepaC::Object_VMT<HashTable<KeyT,ValT>>::instance(),sizeof(HashTable<KeyT,ValT>)) ;

#ifndef FrSINGLE_THREADED
template <typename KeyT, typename ValT>
Fr::ThreadInitializer<HashTable<KeyT,ValT> > HashTable<KeyT,ValT>::initializer ;
#endif /* FrSINGLE_THREADED */

template <typename KeyT, typename ValT>
Fr::Initializer<HashTable<KeyT,ValT> > HashTable<KeyT,ValT>::global_initializer ;

//----------------------------------------------------------------------
// specializations: Fr::Symbol* keys

#define FrMAKE_SYMBOL_HASHTABLE_CLASS(NAME,V) \
\
template <> \
template <> \
inline bool HashTable<const Symbol*,V>::isEqual(const Symbol* key1, const Symbol* key2) const \
{ return (size_t)key1 == (size_t)key2 ; }			  \
\
template <> template <>	\
inline const Symbol* HashTable<const Symbol* ,V>::Entry::copy<const Symbol*>(const Symbol* obj) { return obj ; } \
\
extern template class HashTable<const Symbol*,V> ; \
typedef HashTable<const Symbol*,V> NAME ;

//----------------------------------------------------------------------
// specializations for Fr::SymHashSet not included in the FrMAKE_SYMBOL_HASHTABLE_CLASS macro

template <>
inline size_t HashTable<const Symbol*,NullObject>::hashValFull(const Symbol* key) const
{ 
   return Symbol::hashValue(key) ;
}

template <>
inline bool HashTable<const Symbol*,NullObject>::isEqualFull(const Symbol* key1, const Symbol* key2) const
{ 
   if (key2 == Entry::DELETED())
      return false ;
   if (key1 == key2)
      return true ;
   return (key1 && key2 && strcmp(key1->name(),key2->name()) == 0) ;
}

template <>
inline bool HashTable<const Symbol*,NullObject>::isEqual(const char *keyname, size_t /*namelen*/, const Symbol* key)
{
   if (!key) return keyname == nullptr ;
   if (!keyname) return false ;
   return strcmp(key->name(),keyname) == 0 ;
}

template <>
inline const Symbol* HashTable<const Symbol*,NullObject>::createSymbol(const char* name) const
{
   if (!name) return nullKey() ;
   Symbol* sym = Symbol::create(name) ;
   // mark symbol as belonging to the current symbol table
   //FIXME: set proper table ID
   sym->symtabID(1) ;
   return sym ;
}

template <>
inline void HashTable<const Symbol*,NullObject>::deleteSymbol(const Symbol* sym) const
{
   const_cast<Symbol*>(sym)->free() ;
   return ;
}

/************************************************************************/
/*	Declarations for class HashTableIter				*/
/************************************************************************/

template <typename KeyT, typename ValT>
class HashTableIterBase
   {
   public:
      HashTableIterBase(typename HashTable<KeyT,ValT>::Table* table, size_t index = 0)
	 {
	    m_table = table  ;
	    size_t cap = table->capacity() ;
	    while (index < cap && !table->activeEntry(index))
	       ++index ;
	    m_index = index ;
	    return ;
	 }
      HashTableIterBase(const HashTableIterBase& o)
	 {
	    m_table = o.m_table ;
	    m_index = o.m_index ;
	    return;
	 }
      ~HashTableIterBase() {}

      HashTableIterBase& operator++ ()
	 {
	 size_t cap = m_table->capacity() ;
	 while (m_index < cap)
	    {
	    ++m_index ;
	    if (m_index == cap || m_table->activeEntry(m_index))
	       break ;
	    }
	 return *this ;
	 }
      bool operator== (const HashTableIterBase& o) const { return m_table == o.m_table && m_index == o.m_index ; }
      bool operator!= (const HashTableIterBase& o) const { return m_table != o.m_table || m_index != o.m_index ; }
   protected:
      typename HashTable<KeyT,ValT>::Table* m_table ;
      size_t                       m_index ;
   } ;

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT, typename RetT>
class HashTableIter : public HashTableIterBase<KeyT,ValT>
   {
   public:
      typedef HashTableIterBase<KeyT,ValT> super ;
   public:
      HashTableIter(typename HashTable<KeyT,ValT>::Table* table, size_t index = 0) : super(table,index)
	 {}
      
      std::pair<KeyT,RetT&> operator* () const
	 {
	 size_t idx = this->m_index ;
	 KeyT key = this->m_table->getKey(idx) ;
	 ValT* valptr = this->m_table->getValuePtr(idx) ;
	 return std::pair<KeyT,RetT&>(key,*valptr) ;
	 }
   } ;

/************************************************************************/
/*	Declarations for class HashTableLocalIter			*/
/************************************************************************/

template <typename KeyT, typename ValT>
class HashTableLocalIterBase
   {
   public:
      HashTableLocalIterBase(typename HashTable<KeyT,ValT>::Table* table, size_t bucket, FramepaC::Link index)
	 {
	    m_table = table  ;
	    m_bucket = bucket ;
	    m_index = index ;
	    return ;
	 }
      HashTableLocalIterBase(typename HashTable<KeyT,ValT>::Table* table, size_t bucket)
	 {
	    m_table = table  ;
	    m_bucket = bucket ;
	    m_index = table->chainHead(bucket) ;
	    return ;
	 }
      HashTableLocalIterBase(const HashTableLocalIterBase& o)
	 {
	    m_table = o.m_table ;
	    m_bucket = o.m_bucket ;
	    m_index = o.m_index ;
	    return;
	 }
      ~HashTableLocalIterBase() {}

      HashTableLocalIterBase& operator++ ()
	 {
	 index = m_table->chainNext(m_bucket) ;
	 return *this ;
	 }
      bool operator== (const HashTableLocalIterBase& o) const
	 { return m_table == o.m_table && m_bucket + m_index == o.m_bucket + o.m_index ; }
      bool operator!= (const HashTableLocalIterBase& o) const
	 { return m_table != o.m_table || m_bucket + m_index != o.m_bucket + o.m_index ; }
   protected:
      typename HashTable<KeyT,ValT>::Table* m_table ;
      size_t                       m_bucket ;
      FramepaC::Link		   m_index ;
   } ;

template <typename KeyT, typename ValT, typename RetT>
class HashTableLocalIter : public HashTableLocalIterBase<KeyT,ValT>
   {
   public:
      typedef HashTableLocalIterBase<KeyT,ValT> super ;
   public:
      HashTableLocalIter(typename HashTable<KeyT,ValT>::Table* table, size_t bucket, FramepaC::Link index)
	 : super(table,bucket,index)
	 {}
      HashTableLocalIter(typename HashTable<KeyT,ValT>::Table* table, size_t bucket)
	 : super(table,bucket)
	 {}

      std::pair<KeyT,RetT&> operator* () const
	 {
	 size_t idx = (this->m_bucket + this->m_index) % this->m_table->bucket_count() ;
	 KeyT key = this->m_table->getKey(idx) ;
	 ValT* valptr = this->m_table->getValuePtr(idx) ;
	 return std::pair<KeyT,RetT&>(key,*valptr) ;
	 }
   } ;
   
/************************************************************************/
/*	Member functions for template class HashTable			*/
/************************************************************************/

// these need to be defined *after* the members of HashTableIter due to the
//  circular dependency between the types

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::iterator HashTable<KeyT,ValT>::begin() const
{
   return iterator(m_table.load(),0) ;
}

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::const_iterator HashTable<KeyT,ValT>::cbegin() const
{
   return const_iterator(m_table.load(),0) ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::iterator HashTable<KeyT,ValT>::end() const
{
   Table* table = m_table.load() ;
   return iterator(table,table->capacity()) ; 
}

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::const_iterator HashTable<KeyT,ValT>::cend() const
{
   Table* table = m_table.load() ;
   return const_iterator(table,table->capacity()) ; 
}

// these need to be defined *after* the members of HashTableLocalIter due to the
//  circular dependency between the types

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::local_iterator HashTable<KeyT,ValT>::begin(int bcket) const
{
   return local_iterator(m_table.load(),bcket,0) ;
}

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::const_local_iterator HashTable<KeyT,ValT>::cbegin(int bcket) const
{
   return const_local_iterator(m_table.load(),bcket,0) ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::local_iterator HashTable<KeyT,ValT>::end(int bcket) const
{
   Table* table = m_table.load() ;
   return local_iterator(table,bcket,FramepaC::NULLPTR) ;
}

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::const_local_iterator HashTable<KeyT,ValT>::cend(int bcket) const
{
   Table* table = m_table.load() ;
   return const_local_iterator(table,bcket,FramepaC::NULLPTR) ;
}

/************************************************************************/

extern template class HashTable<Object*,Object*> ;
typedef HashTable<Object*,Object*> ObjHashTable ;

extern template class HashTable<Object*,size_t> ;
typedef HashTable<Object*,size_t> ObjCountHashTable ;

extern template class HashTable<uint32_t,uint32_t> ;
typedef HashTable<uint32_t,uint32_t> HashTable_U32_U32 ;

extern template class HashTable<uint32_t,Object*> ;
typedef HashTable<uint32_t,Object*> IntObjHashTable ;

FrMAKE_SYMBOL_HASHTABLE_CLASS(SymHashTable,Object*) ;
FrMAKE_SYMBOL_HASHTABLE_CLASS(SymCountHashTable,size_t) ;

// Hash Sets: hash tables with keys but no associated values
extern template class HashTable<Object*,NullObject> ;
typedef HashTable<Object*,NullObject> ObjHashSet ;

extern template class HashTable<uint32_t,NullObject> ;
typedef HashTable<uint32_t,NullObject> HashSet_U32 ;

FrMAKE_SYMBOL_HASHTABLE_CLASS(SymHashSet,NullObject) ;

//----------------------------------------------------------------------------

} // end namespace Fr

#undef DELEGATE
#undef DELEGATE_HASH
#undef DELEGATE_HASH_RECLAIM

#endif /* !__Fr_HASHTABLE_H_INCLUDED */

// end of file hashtable.h //
