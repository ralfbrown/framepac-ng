/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-07					*/
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

#ifndef __Fr_HASHTABLE_H_INCLUDED
#define __Fr_HASHTABLE_H_INCLUDED

#include <cstdarg>
#include <cstdint>
#include <climits>
#include <iostream>
#include <type_traits>
#include "framepac/counter.h"
#include "framepac/init.h"
#include "framepac/list.h"
#include "framepac/number.h"
#include "framepac/symbol.h"
#include "framepac/synchevent.h"
#include <cassert>

//#undef FrHASHTABLE_VERBOSITY
//#define FrHASHTABLE_VERBOSITY 2

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#define FrHASHTABLE_MIN_INCREMENT 256

#define FrHASHTABLE_MIN_SIZE      128	// minimum user-specifiable size (number of hash buckets)

#ifdef FrSINGLE_THREADED
#  define FrHT_NUM_TABLES	    2	// need separate for before/after resize
#else
// number of distinct indirection records to maintain in the
//   HashTable proper (if we need more, we'll allocate them, but we
//   mostly want to be able to satisfy needs without that).  We want
//   not just before/after resize, but also a couple extra so that we
//   can allow additional writes even before all old readers complete;
//   if we were to wait for all readers, performance wouldn't scale as
//   well with processor over-subscription.
#  define FrHT_NUM_TABLES	    14
#endif

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

#define FrHASHTABLE_SEARCHRANGE 0x7FF

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
      size_t insert_dup ;
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
      size_t resize ;
      size_t resize_assist ;
      size_t resize_cleanup ;
      size_t resize_wait ;
      size_t reclaim ;
      size_t neighborhood_full ;
      size_t CAS_coll ;
      size_t chain_lock_coll ;
      size_t spin ;
      size_t yield ;
      size_t sleep ;
      size_t none ;			// dummy for use in macros that require a counter name
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
      Link next() const { return m_next.load() ; }
      Link status() const { return m_first.load() & ~link_mask ; }
      bool stale() const { return (m_first.load() & stale_mask) != 0 ; }
      static bool stale(Link stat) { return (stat & stale_mask) != 0 ; }
      bool inUse() const { return (m_first.load() & inuse_mask) != 0 ; }
      static bool inUse(Link stat) { return (stat & inuse_mask) != 0 ; }
      bool reclaiming() const { return (m_first.load() & reclaim_mask) != 0 ; }
      bool copyDone() const { return (m_first.load() & copied_mask) != 0 ; }
      static bool copyDone(Link stat) { return (stat & copied_mask) != 0 ; }

      // modifiers
      void first(Link ofs) { m_first.store(status() | ofs) ; }
      bool first(Link new_ofs, Link expected, Link stat)
	 {
	    new_ofs = (new_ofs /*& link_mask*/) | stat ;
	    expected = (expected /*& link_mask*/) | stat ;
	    return m_first.compare_exchange_weak(expected,new_ofs) ;
	 }
      void next(Link ofs) { m_next.store(ofs) ; }
      bool next(Link new_ofs, Link expected, Link /*stat*/)
	 {
	    // (not using extra flags in m_next yet)
	    new_ofs = (new_ofs /*& link_mask*/) /*| stat*/ ;
	    expected = (expected /*& link_mask*/) /*| stat*/ ;
	    return m_next.compare_exchange_strong(expected,new_ofs) ;
	 }
      bool markStale() { return m_first.test_and_set_bit(stale_bit) ; }
      bool markUsed() { return m_first.test_and_set_bit(inuse_bit) ; }
      bool markFree() { return m_first.test_and_clear_bit(inuse_bit) ; }
      bool markReclaiming() { return m_first.test_and_set_bit(reclaim_bit) ; }
      void markReclaimed() { m_first.fetch_and_relax(~reclaim_mask) ; }
      Link markStaleGetStatus() { return m_first.fetch_or(stale_mask) & ~link_mask ; }
      bool markCopyDone() { return m_first.test_and_set_bit(copied_bit) ; }
   protected:
      Fr::Atomic<Link> m_first ;	// offset of first entry in hash bucket
      Fr::Atomic<Link> m_next ;		// pointer to next entry in current chain

      static constexpr Link link_mask = 0x0FFF ;
      // flag bits in m_first
      static constexpr unsigned stale_bit = 15 ;
      static constexpr Link stale_mask = (1 << stale_bit) ;
      static constexpr unsigned copied_bit = 14 ;
      static constexpr Link copied_mask = (1 << copied_bit) ;
      static constexpr unsigned inuse_bit = 13 ;
      static constexpr Link inuse_mask = (1 << inuse_bit) ;
      static constexpr unsigned reclaim_bit = 12 ;
      static constexpr Link reclaim_mask = (1 << reclaim_bit) ;
   } ;

/************************************************************************/
/************************************************************************/


//----------------------------------------------------------------------------

} // end namespace FramepaC

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/*	Forward declaration for Fr::SymbolTable				*/
/************************************************************************/

const class Fr::Symbol *Fr_allocate_symbol(class Fr::SymbolTable *symtab, const char *name,
					   size_t namelen) ;

/************************************************************************/
/*	Helper functions						*/
/************************************************************************/

#if FrHASHTABLE_VERBOSITY > 0
#include <cstdio>
#endif /* FrHASHTABLE_VERBOSITY > 0 */

/************************************************************************/
/*  Helper specializations of global functions for use by HashTable	*/
/************************************************************************/

inline void free_object(const Object*) { return ; }
inline void free_object(const class Fr::Symbol*) { return ; }
inline void free_object(size_t) { return ; }

/************************************************************************/
/*	Declarations for class HashTableBase				*/
/************************************************************************/

class HashTableBase : public Object
   {
   public:
      HashTableBase() : m_active_resizes(0) {}
      ~HashTableBase() {}

      void startResize() ;
      void finishResize() ;

      virtual bool assistResize() = 0 ;
   protected:
      HashTableBase* m_next_base ;
      unsigned	     m_active_resizes ;
   } ;

/************************************************************************/
/*	Declarations for class HashTableHelper				*/
/************************************************************************/

class HashTableHelper
   {
   public:
      static bool queueResize(HashTableBase* ht) ;

   protected:  // internal methods
      HashTableHelper() {}
      ~HashTableHelper() {}
      static bool initialize() ;

   protected:
      static std::thread*     s_thread ;
      static bool             s_initialized ;
   } ;

/************************************************************************/
/*	Declarations for template class HashTable			*/
/************************************************************************/

template <typename KeyT, typename ValT>
class HashTable : public HashTableBase
   {
   public:
      typedef bool HashKeyValueFunc(KeyT key, ValT value,std::va_list) ;
      typedef bool HashKVFunc(KeyT key, ValT value) ;
      typedef bool HashKeyPtrFunc(KeyT key, ValT *,std::va_list) ;

      // incorporate the auxiliary classes
      typedef FramepaC::Link Link ;
      typedef FramepaC::HashPtr HashPtr ;
      typedef FramepaC::HashTable_Stats HashTable_Stats ;

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
	 static KeyT copy(KeyT obj)
	    {
	       return obj ? static_cast<KeyT>((Object*)obj->clone()) : nullKey() ;
	    }
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
	 bool isActive() const { return m_key != DELETED() ; }

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

#if defined(FrSINGLE_THREADED)
	 ValT swapValue(ValT new_value)
	    {
	       ValT old_value = getValue() ;
	       setValue(new_value) ;
	       return old_value ;
	    }
#else
	 template <typename RetT = ValT>
	 typename std::enable_if<!std::is_empty<ValT>::value,RetT>::type
	 swapValue(ValT new_value)
	    { return Atomic<ValT>::ref(*getValuePtr()).exchange(new_value) ; }
	 template <typename RetT = ValT>
	 typename std::enable_if<std::is_empty<ValT>::value,RetT>::type
	 swapValue(ValT) { return nullVal() ; }
#endif /* FrSINGLE_THREADED */
	 ALWAYS_INLINE static constexpr KeyT DELETED() {return (KeyT)~0UL ; } 

	 // I/O
#if FIXME
	 ostream &printValue(ostream &output) const
	    {
	       output << m_key ;
	       return output ;
	    }
	 char *displayValue(char *buffer) const
	    {
	       buffer = m_key.load().print(buffer) ;
	       *buffer = '\0' ;
	       return buffer ;
	    }
	 size_t displayLength() const
	    {
	       return m_key ? m_key->displayLength() : 3 ;
	    }
#endif /* FIXME */
      public: // data members
	 if_INTERLEAVED(HashPtr  m_info ;)
      protected: // members
	 Fr::Atomic<KeyT>  m_key ;
	 ValT              m_value[std::is_empty<ValT>::value ? 0 : 1] ;
      } ;
      //------------------------
      // encapsulate all of the fields which must be atomically swapped at the end of a resize()
      class Table
      {
	 typedef class Fr::HashTable<KeyT,ValT> HT ;
      public:
	 Table(size_t size = 0)
	    {
	       init(size) ;
	       return ;
	    } ;
	 ~Table() { clear() ; }
	 void init(size_t size) ;
	 void clear() ;
	 bool good() const { return m_entries != nullptr ifnot_INTERLEAVED(&& m_ptrs != nullptr) && m_size > 0 ; }
	 bool superseded() const { return m_next_table.load() != nullptr ; }
	 bool resizingDone() const { return m_resizedone.load() ; }
	 // maintaining the count of elements in the table is too much of a bottleneck under high load,
	 //   so we'll punt and simply scan the hash array if someone needs that count
	 size_t currentSize() const { return countItems() ; }
	 Table *next() const { return m_next_table.load() ; }
	 Table *nextFree() const { return m_next_free.load() ; }
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
	 bool chainIsStale(size_t N) const { return bucketPtr(N)->stale() ; }
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
	 [[gnu::hot]] void resizeCopySegments(size_t max_segs = ~0UL) ;
	 void clearDuplicates(size_t bucketnum) ;

	 bool reclaimChain(size_t bucketnum) ;
	 bool assistWithResize() ;
	 void resizeCleanup() ;

	 static Object* makeObject(KeyT key) ;

      public:
	 void onRemove(HashKVFunc *func) { remove_fn = func ; }
	 bool resize(size_t newsize, bool enlarge_only = false) ;

	 [[gnu::hot]] bool add(size_t hashval, KeyT key, ValT value) ;
	 [[gnu::hot]] bool add(size_t hashval, KeyT key) { return add(hashval,key,nullVal()) ; }
	 [[gnu::hot]] ValT addCount(size_t hashval, KeyT key, size_t incr) ;

	 [[gnu::hot]] bool contains(size_t hashval, KeyT key) const ;
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

	 // special support for Fr::SymbolTableX
	 template <typename RetT = KeyT>
	 typename std::enable_if<std::is_base_of<Fr::Symbol,KeyT>::value,RetT>::type
	 addKey(size_t hashval, const char* name, size_t namelen, bool* already_existed = nullptr) ;
	 // (default no-op version for non-Symbol hash tables, selected by SFINAE)
	 template <typename RetT = KeyT>
	 typename std::enable_if<!std::is_base_of<Fr::Symbol,KeyT>::value,RetT>::type
	 addKey(size_t /*hashval*/, const char* /*name*/, size_t /*namelen*/, bool* /*already_existed*/ = nullptr) ;

	 // special support for Fr::SymbolTable
	 [[gnu::hot]] bool contains(size_t hashval, const char *name, size_t namelen) const ;
	 // special support for Fr::SymbolTableX
	 [[gnu::hot]] KeyT lookupKey(size_t hashval, const char *name, size_t namelen) const ;

	 void replaceValue(size_t pos, ValT new_value) ;

	 //================= Content Statistics ===============
	 [[gnu::cold]] size_t countItems() const ;
	 [[gnu::cold]] size_t countItems(bool remove_dups) ;
	 [[gnu::cold]] size_t countDeletedItems() const ;
	 [[gnu::cold]] size_t chainLength(size_t bucketnum) const ;
	 [[gnu::cold]] size_t *chainLengths(size_t &max_length) const ;
	 [[gnu::cold]] size_t *neighborhoodDensities(size_t &num_densities) const ;

	 //============== Iterators ================
	 bool iterateVA(HashKeyValueFunc *func, std::va_list args) const ;
	 bool iterateAndClearVA(HashKeyValueFunc *func, std::va_list args) const ;
	 bool iterateAndModifyVA(HashKeyPtrFunc *func, std::va_list args) const ;
	 List *allKeys() const ;

	 //============== Debugging Support ================
	 [[gnu::cold]] bool verify() const ;

	 // ============= Object support =============
	 ostream &printKeyValue(ostream &output, KeyT key) const ;
	 size_t keyDisplayLength(KeyT key) const ;
	 char *displayKeyValue(char *buffer, KeyT key) const ;
	 ostream &printValue(ostream &output) const ;
	 char *displayValue(char *buffer) const ;
	 // get size of buffer needed to display the string representation of the hash table
	 // NOTE: will not be valid if there are any add/remove/resize calls between calling
	 //   this function and using displayValue(); user must ensure locking if multithreaded
	 size_t cStringLength(size_t wrap_at, size_t indent) const ;

      public:
	 Entry*            m_entries ;			// hash array [unchanging]
	 ifnot_INTERLEAVED(HashPtr *m_ptrs ;)		// links chaining elements of a hash bucket [unchanging]
	 HT*               m_container ;		// hash table for which this is the content [unchanging]
	 Fr::Atomic<Table*> m_next_table { nullptr } ;	// the table which supersedes us
	 Fr::Atomic<Table*> m_next_free { nullptr } ;
	 HashKVFunc*       remove_fn { nullptr } ; 	// invoke on removal of entry/value
	 static const Link searchrange = FrHASHTABLE_SEARCHRANGE ; // full search window, starting at bucket head
	 static const size_t NULLPOS = ~0UL ;
	 size_t	           m_size ;		// capacity of hash array [constant for life of table]
	 size_t	           m_fullsize ;		// capacity including padding [constant for life of table]
      protected:
	 Fr::Atomic<size_t> m_segments_total { 0 } ;
	 Fr::Atomic<size_t> m_segments_assigned { 0 } ;
	 Fr::Atomic<size_t> m_first_incomplete { ~0U } ;
	 Fr::Atomic<size_t> m_last_incomplete { 0 } ;
	 Fr::SynchEvent     m_resizestarted ;
	 Fr::SynchEventCountdown m_resizepending ;
	 Fr::Atomic<bool>   m_resizelock { false } ; // ensures that only one thread can initiate a resize
	 Fr::Atomic<bool>   m_resizedone { false } ;

         } ;
      //------------------------
      class TablePtr
         {
	 public:
	    TablePtr 	   *m_next { nullptr } ;
	    Atomic<Table*> *m_table { nullptr } ;
	    size_t   	    m_id { 0 } ;	   // thread ID, for help in debugging
	 public:
	    TablePtr() : m_next(nullptr), m_table(nullptr), m_id(0) {}
	    bool initialized() const { return m_table != nullptr ; }
	    void clear()
	       { m_table = nullptr ; m_next = nullptr ; }
	    void init(Table** tab, TablePtr* next)
	       { m_table = reinterpret_cast<Atomic<Table*>*>(tab) ;
		 m_next = next ; m_id = FramepaC::my_job_id ; }
	    void init(Atomic<Table*>* tab, TablePtr* next)
	       { m_table = tab ; m_next = next ; m_id = FramepaC::my_job_id ; }
	    const Table *table() const { return m_table->load() ; }
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
   protected: // members
      Atomic<Table*>   	  m_table ;	// pointer to currently-active m_tables[] entry
      Atomic<Table*>	  m_oldtables { nullptr } ;  // start of list of currently-live hash arrays
      Atomic<Table*>	  m_freetables { nullptr } ;
      Table		  m_tables[FrHT_NUM_TABLES] ;// hash array, chains, and associated data
      HashKeyValueFunc   *cleanup_fn ;	// invoke on destruction of obj
      HashKVFunc         *remove_fn ; 	// invoke on removal of entry/value
#ifndef FrSINGLE_THREADED
      static FramepaC::ThreadInitializer<HashTable> initializer ;
      static TablePtr*    s_thread_entries ;
      static size_t       s_registered_threads ;
      static thread_local TablePtr* s_thread_record ;
      static thread_local Table*    s_table ; // thread's announcement which hash table it's using
#endif /* FrSINGLE_THREADED */
#ifdef FrHASHTABLE_STATS
      mutable HashTable_Stats	  m_stats ;
      static thread_local HashTable_Stats* s_stats ;
#endif /* FrHASHTABLE_STATS */

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
      void init(size_t initial_size, Table *table = nullptr) ;
      Table *allocTable() ;
      void releaseTable(Table *t) ;
      void freeTables() ;
      void updateTable() ;

      // set up the per-thread info needed for safe reclamation of
      //   entries and hash arrays, as well as an on-exit callback
      //   to clear that info
   public://FIXME, should only be called from ThreadInitializer
      static void threadInit() ;
   protected:
      static void threadCleanup() ;
      static bool stillLive(const Table *version) ;

      size_t maxSize() const { return m_table.load()->m_size ; }
      static inline size_t hashVal(KeyT key)
	 {
	    return key ? key->hashValue() : 0 ;
	 }
      static inline size_t hashValFull(KeyT key)
	 {
	    // separate version for use by resizeTo(), which needs to use the full
	    //   hash value and not just the key on Fr::SymbolTableX
	    // this is the version for everyone *but* Fr::SymbolTableX
	    return hashVal(key) ;
	 }
      // special support for Fr::SymbolTableX
      static size_t hashVal(const char *keyname, size_t *namelen) ;
      static inline bool isEqual(KeyT key1, KeyT key2)
	 {
	    return key2 != Entry::DELETED() && Fr::equal(key1,key2) ;
	 }
      static inline bool isEqualFull(KeyT key1, KeyT key2)
	 {
	    // separate version for use by resizeTo(), which needs to use the full
	    //   key name and not just the key's pointer on Fr::SymbolTableX
	    // this is the version for everyone *but* Fr::SymbolTableX
	    return isEqual(key1,key2) ;
	 }
      // special support for Fr::SymbolTableX; must be overridden
      static bool isEqual(const char *keyname, size_t namelen, KeyT key) ;

      // ============== Definitions to reduce duplication ================
      // much of the HashTable API just calls through to Table after
      //   setting a hazard pointer
#ifdef FrSINGLE_THREADED
#define DELEGATE(delegate) \
            return m_table.load()->delegate ;
#define DELEGATE_HASH(delegate) 			\
	    size_t hashval = hashVal(key) ; 		\
            return m_table.load()->delegate ;
#define DELEGATE_HASH_RECLAIM(type,delegate)		\
	    if (m_oldtables.load() != m_table.load())	\
	       assistReclaim() ;			\
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
#define DELEGATE_HASH_RECLAIM(type,delegate)				\
            if (m_oldtables.load() != m_table.load())			\
	       assistReclaim() ;					\
	    size_t hashval = hashVal(key) ; 				\
	    HazardLock hl(m_table) ;					\
	    return m_table.load()->delegate ;
#endif /* FrSINGLE_THREADED */

      // ============== The public API for HashTable ================
   public:
      HashTable(size_t initial_size = 1031)
	 : HashTableBase(), m_table(nullptr)
	 {
	    init(initial_size) ;
	    return ;
	 }
      HashTable(const HashTable &ht) ;
      virtual ~HashTable() ;

      bool resizeTo(size_t newsize, bool enlarge_only = false)
	 { DELEGATE(resize(newsize,enlarge_only)) }
      bool reserve_(size_t newsize) { DELEGATE(resize(newsize,true) ; }
      void reserve(size_t newsize) { reserve_(newsize) ; }

      bool reclaimDeletions(size_t totalfrags = 1, size_t fragnum = 0)
         { DELEGATE(reclaimDeletions(totalfrags,fragnum)) }

      [[gnu::hot]] bool add(KeyT key) { DELEGATE_HASH_RECLAIM(bool,add(hashval,key)) }
      [[gnu::hot]] bool add(KeyT key, ValT value) { DELEGATE_HASH_RECLAIM(bool,add(hashval,key,value)) }
      [[gnu::hot]] ValT addCount(KeyT key, size_t incr) { DELEGATE_HASH_RECLAIM(size_t,addCount(hashval,key,incr)) }
      bool remove(KeyT key) { DELEGATE_HASH(remove(hashval,key)) }
      [[gnu::hot]] bool contains(KeyT key) const { DELEGATE_HASH(contains(hashval,key)) }
      [[gnu::hot]] ValT lookup(KeyT key) const { DELEGATE_HASH(lookup(hashval,key)) }
      [[gnu::hot]] bool lookup(KeyT key, ValT *value) const { DELEGATE_HASH(lookup(hashval,key,value)) }
      // NOTE: this lookup() is not entirely thread-safe if clear==true
      [[gnu::hot]] bool lookup(KeyT key, ValT *value, bool clear) { DELEGATE_HASH(lookup(hashval,key,value,clear)) }
      // NOTE: lookupValuePtr is not safe in the presence of parallel
      //   add() and remove() calls!  Use global synchronization if
      //   you will be using both this function and add()/remove()
      //   concurrently on the same hash table.
      ValT *lookupValuePtr(KeyT key) const { DELEGATE_HASH(lookupValuePtr(hashval,key)) }
      [[gnu::hot]] bool add(KeyT key, ValT value, bool replace)
	 {
	 if (m_oldtables.load() != m_table.load())
	       assistReclaim() ;
	    size_t hashval = hashVal(key) ;
	    HazardLock hl(m_table) ;
	    Table *tab = m_table.load() ;
	    if (replace)
	       tab->remove(hashval,key) ;
	    return tab->add(hashval,key,value) ;
	 }
      // special support for Fr::SymbolTableX
      [[gnu::hot]] KeyT addKey(const char *name, bool *already_existed = nullptr)
	 {
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
      // special support for Fr::SymbolTableX
      [[gnu::hot]] KeyT lookupKey(const char *name, size_t namelen, size_t hashval) const
         { DELEGATE(lookupKey(hashval,name,namelen)) }
      // special support for Fr::SymbolTableX
      [[gnu::hot]] KeyT lookupKey(const char *name) const
         {
	    if (!name)
	       return nullKey() ;
	    size_t namelen ;
	    size_t hashval = hashVal(name,&namelen) ;
	    DELEGATE(lookupKey(hashval,name,namelen))
	 }

      [[gnu::cold]] size_t countItems(bool remove_dups = false) const
	 { DELEGATE(countItems(remove_dups)) }
      [[gnu::cold]] size_t countDeletedItems() const
	 { DELEGATE(countDeletedItems()) }
      [[gnu::cold]] size_t *chainLengths(size_t &max_length) const
         { DELEGATE(chainLengths(max_length)) }
      [[gnu::cold]] size_t *neighborhoodDensities(size_t &num_densities) const
	 { DELEGATE(neighborhoodDensities(num_densities)) }

      // ========== Iterators ===========
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
      bool iterateAndClearVA(HashKeyValueFunc *func, std::va_list args)
	 { DELEGATE(iterateAndClearVA(func,args)) }
      bool iterateAndClear(HashKeyValueFunc *func,...)
	 {
	    std::va_list args ;
	    va_start(args,func) ;
	    bool success = iterateAndClearVA(func,args) ;
	    va_end(args) ;
	    return success ;
	 }
      //  iterateAndModify is not safe in the presence of parallel remove() calls!
      bool iterateAndModifyVA(HashKeyPtrFunc *func, std::va_list args)
	 { DELEGATE(iterateAndModifyVA(func,args)) }
      //  iterateAndModify is not safe in the presence of parallel remove() calls!
      bool iterateAndModify(HashKeyPtrFunc *func,...)
	 {
	    std::va_list args ;
	    va_start(args,func) ;
	    bool success = iterateAndModifyVA(func,args) ;
	    va_end(args) ;
	    return success ;
	 }
      List *allKeys() const { DELEGATE(allKeys()) }

      // set callback function to be invoked when the hash table is deleted
      void onDelete(HashKeyValueFunc *func) { cleanup_fn = func ; }
      // set callback function to be invoked when an entry is deleted; this
      //   permits the associated value for the entry to be freed
      void onRemove(HashKVFunc *func)
         {
	 remove_fn = func ;
	 Table* tbl = m_table.load() ;
	 if (tbl) tbl->onRemove(func) ; 
	 }

      // access to internal state
      size_t currentSize() const { DELEGATE(currentSize()) }
      size_t maxCapacity() const { DELEGATE(m_fullsize) }
      bool isPacked() const { return false ; }  // backwards compatibility
      HashKeyValueFunc *cleanupFunc() const { return cleanup_fn ; }
      HashKVFunc *onRemoveFunc() const { return remove_fn ; }

      // =============== Background Processing =================
      virtual bool assistResize() ;
      virtual bool assistReclaim() ;

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
      size_t numberOfChainLockCollisions() const { return m_stats.chain_lock_coll ; }
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
      static size_t numberOfChainLockCollisions() { return 0 ; }
      static size_t numberOfSpins() { return 0 ; }
      static size_t numberOfYields() { return 0 ; }
      static size_t numberOfSleeps() { return 0 ; }
#endif

      // ============= Fr::Object support =============
      const char* typeName_() const { return "HashTable" ; }
      HashTable* shallowCopy_() const { return new HashTable(*this) ; }
      HashTable* clone_() const { return new HashTable(*this) ; }
      void free_() { delete this ; }
      void shallowFree_() { free_() ; }
      size_t size_() const { return currentSize() ; }
      size_t hashValue_() const { return 0 ; } //FIXME

      // get size of buffer needed to display the string representation of the hash table
      // NOTE: will not be valid if there are any add/remove/resize calls between calling
      //   this function and using displayValue(); user must ensure locking if multithreaded
      size_t cStringLength_(size_t wrap_at, size_t indent) const { DELEGATE(cStringLength(wrap_at,indent))) }

      virtual ostream& printValue(ostream &output) const { DELEGATE(printValue(output)) }
      virtual char* displayValue(char *buffer) const { DELEGATE(displayValue(buffer)) }
      virtual bool expand(size_t incr)
	 {
	    if (incr < FrHASHTABLE_MIN_INCREMENT)
	       incr = FrHASHTABLE_MIN_INCREMENT ;
	    resizeTo(maxSize() + incr) ;
	    return true ;
	 }
      virtual bool expandTo(size_t newsize)
	 {
	    if (newsize > maxSize())
	       resizeTo(newsize) ;
	    return true ;
	 }
      virtual bool hashp() const { return true ; }

      //  =========== Debugging Support ============
      [[gnu::cold]] bool verify() const { DELEGATE(verify()) }

   } ;

//----------------------------------------------------------------------
// specializations: integer keys

#define FrMAKE_INTEGER_HASHTABLE_CLASS(NAME,K,V) \
\
template <> \
inline size_t HashTable<K,V>::hashVal(const K key) { return (size_t)key ; } \
\
template <> \
inline bool HashTable<K,V>::isEqual(const K key1, const K key2) \
{ return key1 == key2 ; } \
\
template <> \
inline K HashTable<K,V>::Entry::copy(const K obj) { return obj ; } \
\
template <> \
inline Object* HashTable<K,V>::Table::makeObject(K key) \
{ return Integer::create(key) ; }	 \
\
template <> \
inline size_t HashTable<K,V>::Table::keyDisplayLength(const K key) const	\
{ return snprintf(nullptr,0,"%ld%c",(size_t)key,(char)'\0') ; }		\
\
template <> \
inline char* HashTable<K,V>::Table::displayKeyValue(char* buffer,const K key) const \
{ return buffer + snprintf(buffer,50,"%ld%c",(size_t)key,(char)'\0') ; }	\
\
extern template class HashTable<K,V> ; \
typedef HashTable<K,V> NAME ;

//----------------------------------------------------------------------
// specializations: Fr::Symbol* keys

#define FrMAKE_SYMBOL_HASHTABLE_CLASS(NAME,V) \
\
template <> \
inline size_t HashTable<const Symbol*,V>::hashVal(const Symbol* key) { return (size_t)key ; } \
\
template <> \
inline bool HashTable<const Symbol*,V>::isEqual(const Symbol* key1, const Symbol* key2) \
{ return (size_t)key1 == (size_t)key2 ; }			  \
\
template <> \
inline const Symbol* HashTable<const Symbol* ,V>::Entry::copy(const Symbol* obj) { return obj ; } \
\
extern template class HashTable<const Symbol*,V> ; \
typedef HashTable<const Symbol*,V> NAME ;

//----------------------------------------------------------------------
// specializations for Fr::SymbolTableX not included in the FrMAKE_SYMBOL_HASHTABLE_CLASS macro

size_t Fr_symboltable_hashvalue(const char* symname) ;
template <>
inline size_t HashTable<const Symbol*,NullObject>::hashValFull(const Symbol* key)
{ 
   return key ? Fr_symboltable_hashvalue(key->name()) : 0 ;
}

template <>
inline bool HashTable<const Symbol*,NullObject>::isEqualFull(const Symbol* key1, const Symbol* key2)
{ 
   if (key2 != Entry::DELETED())
      return false ;
   if (key1 == key2)
      return true ;
   return (key1 && key2 && strcmp(key1->name(),key2->name()) == 0) ;
}

/************************************************************************/

extern template class HashTable<Object*,Object*> ;
typedef HashTable<Object*,Object*> ObjHashTable ;

extern template class HashTable<Object*,size_t> ;
typedef HashTable<Object*,size_t> ObjCountHashTable ;

FrMAKE_SYMBOL_HASHTABLE_CLASS(SymHashTable,Object*) ;
FrMAKE_SYMBOL_HASHTABLE_CLASS(SymCountHashTable,size_t) ;

// Hash Sets: hash tables with keys but no associated values
extern template class HashTable<Object*,NullObject> ;
typedef HashTable<Object*,NullObject> ObjHashSet ;

FrMAKE_SYMBOL_HASHTABLE_CLASS(SymbolTableX,NullObject) ;
FrMAKE_INTEGER_HASHTABLE_CLASS(HashSet_U32,uint32_t,NullObject) ;

//----------------------------------------------------------------------------

} // end namespace Fr

#undef DELEGATE
#undef DELEGATE_HASH
#undef DELEGATE_HASH_RECLAIM

#endif /* !__Fr_HASHTABLE_H_INCLUDED */

// end of file hashtable.h //
