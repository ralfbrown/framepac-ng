/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-02					*/
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
#include "framepac/list.h"
#include "framepac/synchevent.h"
#include <cassert>

/************************************************************************/
/*	Compile-Time Configuration					*/
/************************************************************************/

// specify how much checking and output you want:
//   0 - completely silent except for critical errors
//   1 - important messsages only
//   2 - status messages
//   3 - extra sanity checking
//   4 - expensive sanity checking
#define FrHASHTABLE_VERBOSITY 0

// uncomment the following line to collect operational statistics (slightly slower)
#define FrHASHTABLE_STATS

// comment out the following to maintain global stats (much slower, since it requires
//    atomic increments of contended locations)
#define FrHASHTABLE_STATS_PERTHREAD

// uncomment the following line to interleave bucket headers and
//   bucket contents rather than using separate arrays (fewer cache
//   misses, but wastes some space on 64-bit machines if the key or
//   value are 64 bits)
#define FrHASHTABLE_INTERLEAVED_ENTRIES

// uncomment the following line to enable 16-bit link pointers instead
//   of 8-bit.  This will increase memory use unless you use
//   interleaved entries on a 64-bit machine.
//#define FrHASHTABLE_BIGLINK

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#define FrHASHTABLE_MIN_INCREMENT 256

#ifdef FrSINGLE_THREADED
#  define FrHASHTABLE_MIN_SIZE 128
#  define FrHT_NUM_TABLES	2	// need separate for before/after resize
#else
#  define FrHASHTABLE_MIN_SIZE 256
// number of distinct indirection records to maintain in the
//   HashTable proper (if we need more, we'll allocate them, but we
//   mostly want to be able to satisfy needs without that).  We want
//   not just before/after resize, but also a couple extra so that we
//   can allow additional writes even before all old readers complete;
//   if we were to wait for all readers, performance wouldn't scale as
//   well with processor over-subscription.
#  define FrHT_NUM_TABLES	8
#endif

#define FrHashTable_DefaultMaxFill 0.97
#define FrHashTable_MinFill        0.25	// lowest allowed value for MaxFill

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

#ifdef FrHASHTABLE_BIGLINK
#  define FrHASHTABLE_SEARCHRANGE INT16_MAX
#else
#  define FrHASHTABLE_SEARCHRANGE INT8_MAX
#endif

#ifdef FrHASHTABLE_STATS
# define INCR_COUNTstat(x) (++ s_stats.x)
#  define INCR_COUNT(x) (++ s_stats.x)
#  define DECR_COUNT(x) (-- s_stats.x)
#else
#  define INCR_COUNTstat(x)
#  define INCR_COUNT(x)
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

namespace Fr
{
   // forward declarations
   template <typename KeyT, typename ValT> class HashTable ;

   class List ;
   class Symbol ;
   class SymbolTable ;//FIXME

} // end namespace Fr

//----------------------------------------------------------------------------

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
      size_t reclaim ;
      size_t move ; // how many entries were moved to make room?
      size_t neighborhood_full ;
      size_t CAS_coll ;
      Fr::Atomic<size_t> chain_lock_count ;
      Fr::Atomic<size_t> chain_lock_coll ;
      size_t spin ;
      size_t yield ;
      size_t sleep ;
      size_t none ;			// dummy for use in macros that require a counter name
   public:
      void clear() ;
      void add(const HashTable_Stats *other_stats) ;
   } ;

// encapsulate the chaining links
#ifdef FrHASHTABLE_BIGLINK
typedef int16_t Link ;
#else
typedef int8_t Link ;
#endif /* FrHASHTABLE_BIGLINK */

/************************************************************************/
/*      Auxiliary class for hash pointer heads				*/
/************************************************************************/

struct HashPtrHead
   {
   public:
   // the following two fields need to be adjacent so that we
   //   can CAS them together in insertKey(), so we define
   //   this struct just for that purpose
      Fr::Atomic<Link>    first ; // offset of first entry in hash bucket
      Fr::Atomic<uint8_t> status ;// is someone messing with this bucket's chain?
   public:
      HashPtrHead() : first(0), status(0) { status.store(0) ; }
   } ;	    

/************************************************************************/
/************************************************************************/

union HashPtrInt
   {
   HashPtrHead head ;  // .first and .status fields
#ifdef FrHASHTABLE_BIGLINK
   uint32_t head_int ;
#else
   uint16_t head_int ;
#endif /* FrHASHTABLE_BIGLINK */
   } ;

/************************************************************************/
/************************************************************************/

#ifdef FrHASHTABLE_BIGLINK
   static const Link NULLPTR = INT16_MIN ;
#else
   static const Link NULLPTR = INT8_MIN ;
#endif /* FrHASHTABLE_BIGLINK */

class HashPtr
   {
   public:
      Fr::Atomic<Link> next ;	// pointer to next entry in current chain
      Fr::Atomic<Link> offset ; // this entry's offset in hash bucket (subtract to get bucketnum)
      HashPtrHead head ;  	// .first and .status fields
   public:
      static const unsigned lock_bit = 0 ;
      static const uint8_t lock_mask = (1 << lock_bit) ;
      static const unsigned reclaimed_bit = 3 ;
      static const uint8_t reclaimed_mask = (1 << reclaimed_bit) ;
      static const unsigned deleted_bit = 4 ;
      static const uint8_t deleted_mask = (1 << deleted_bit) ;
      static const unsigned inuse_bit = 5 ;
      static const uint8_t inuse_mask = (1 << inuse_bit) ;
      static const unsigned copied_bit = 6 ;
      static const uint8_t copied_mask = (1 << copied_bit) ;
      static const unsigned stale_bit = 7 ;
      static const uint8_t stale_mask = (1 << stale_bit) ;
      void init() { head.first = NULLPTR ; next.store(NULLPTR) ; offset.store(NULLPTR) ; head.status.store(0) ; }
      HashPtr() : next(), offset(), head() { init() ; }
      ~HashPtr() {}
      uint8_t status() const { return head.status.load() ; }
#if 0
//FIXME:
      const uint8_t *statusPtr() const { return &head.status ; }
      uint8_t *statusPtr() { return &head.status ; }
#else
      const uint8_t *statusPtr() const ;
      uint8_t *statusPtr() ;
#endif
      bool locked() const { return (status() & lock_mask) != 0 ; }
      bool stale() const { return (status() & stale_mask) != 0 ; }
      bool copyDone() const { return (status() & copied_mask) != 0 ; }
      bool markStale() { return head.status.test_and_set_bit(stale_bit) ; }
      uint8_t markStaleGetStatus() { return head.status.test_and_set_mask(stale_mask) ; }
      bool markCopyDone() { return head.status.test_and_set_bit(copied_bit) ; }
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
/*	Declarations for template class HashTable			*/
/************************************************************************/

template <typename KeyT, typename ValT>
class HashTable : public Object
   {
   public:
      typedef bool HashKeyValueFunc(KeyT key, ValT value,std::va_list) ;
      typedef bool HashKVFunc(KeyT key, ValT value) ;
      typedef bool HashKeyPtrFunc(KeyT key, ValT *,std::va_list) ;

      // incorporate the auxiliary classes
      typedef FramepaC::Link Link ;
      typedef FramepaC::HashPtr HashPtr ;
      typedef FramepaC::HashTable_Stats HashTable_Stats ;

      // encapsulate a hash table entry
      class Entry
      {
      private:
	 constexpr static size_t numValues() { return lengthof(Entry::m_value) ; }
      public: // members
	 if_INTERLEAVED(HashPtr  m_info ;)
	 Fr::Atomic<KeyT>  m_key ;
	 ValT              m_value[(sizeof(ValT) < 2) ? 0 : 1] ;
      public: // methods
	 void *operator new(size_t,void *where) { return where ; }
	 static KeyT copy(KeyT obj)
	    {
	       return obj ? static_cast<KeyT>(obj->deepcopy()) : 0 ;
	    }
	 void markUnused() { m_key = UNUSED() ; }
	 void init()
	    {
	       if_INTERLEAVED(m_info.init() ;)
		  markUnused() ;
	       setValue(0) ; 
	    }
	 void init(const Entry &entry)
	    {
	       if_INTERLEAVED(m_info = entry.m_info ;)
		  m_key = entry.m_key ;
	       setValue(entry.getValue()) ;
	    }

	 // constructors/destructors
	 Entry() { init() ; }
	 ~Entry() { if (isActive()) { markUnused() ; } }

	 // variable-setting functions
	 void setValue(ValT value)
	    { if (numValues() > 0) m_value[0] = value ; }
	 ValT incrCount(ValT incr)
	    { return (numValues() > 0) ? (m_value[0] += incr) : incr ; }
	 ValT atomicIncrCount(ValT incr)
	    { return (numValues() > 0) ? Fr::Atomic<ValT>::ref(m_value[0]) += incr : incr ; }

	 // access to internal state
	 KeyT getKey() const { return m_key ; }
	 KeyT copyName() const { return copy(m_key) ; }
	 ValT getValue() const { return (numValues() > 0) ? m_value[0] : 0 ; }
	 //const ValT *getValuePtr() const { return (numValues() > 0) ? &m_value[0] : (ValT*)nullptr ; }
	 ValT *getValuePtr() const { return (numValues() > 0) ? (ValT*)&m_value[0] : (ValT*)nullptr ; }
#if defined(FrSINGLE_THREADED)
	 ValT swapValue(ValT new_value)
	    {
	       ValT old_value = getValue() ;
	       setValue(new_value) ;
	       return old_value ;
	    }
#else
	 ValT swapValue(ValT new_value)
	    { return getValuePtr()->exchange(new_value) ; }
#endif /* FrSINGLE_THREADED */
	 ALWAYS_INLINE static KeyT UNUSED() {return (KeyT)~0UL ; } 
	 ALWAYS_INLINE static KeyT DELETED() {return (KeyT)~1UL ; } 
	 ALWAYS_INLINE static KeyT RECLAIMING() { return (KeyT)~2UL ; }
	 bool isUnused() const { return ANNOTATE_UNPROTECTED_READ(m_key) == UNUSED() ; }
	 bool isDeleted() const { return ANNOTATE_UNPROTECTED_READ(m_key) == DELETED() ; }
	 bool isBeingReclaimed() const { return  ANNOTATE_UNPROTECTED_READ(m_key) == RECLAIMING() ; }
	 bool isActive() const { return ANNOTATE_UNPROTECTED_READ(m_key) < RECLAIMING() ; }

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
         } ;
      //------------------------
      // encapsulate all of the fields which must be atomically swapped at the end of a resize()
      class Table
      {
	 typedef class Fr::HashTable<KeyT,ValT> HT ;
      public:
//!      void* operator new(size_t sz) { return FrMalloc(sz) ; }
//!      void* operator new(size_t, void* where) { return where ; }
//!      void operator delete(void* blk) { FrFree(blk) ; }
	 Table()
	    {
	       init(0) ;
	       return ;
	    }
	 Table(size_t size, double maxfill)
	    {
	       init(size,maxfill) ;
	       return ;
	    } ;
	 ~Table() { clear() ; }
	 void init(size_t size, double max_fill = FrHashTable_DefaultMaxFill) ;
	 void clear() ;
	 bool good() const { return m_entries != nullptr ifnot_INTERLEAVED(&& m_ptrs != nullptr) && m_size > 0 ; }
	 bool superseded() const { return m_next_table.load() != nullptr ; }
	 bool resizingDone() const { return m_resizedone.load() ; }
	 size_t currentSize() const { return m_currsize.load() ; }
	 void setSize(size_t newsz) { m_currsize.store(newsz) ; }
	 Table *next() const { return m_next_table.load() ; }
	 Table *nextFree() const { return m_next_free.load() ; }
	 KeyT getKey(size_t N) const { return m_entries[N].m_key.load() ; }
	 KeyT getKeyNonatomic(size_t N) const { return ANNOTATE_UNPROTECTED_READ(m_entries[N].m_key) ; }
	 void setKey(size_t N, KeyT newkey) { m_entries[N].m_key.store(newkey) ; }
	 ValT getValue(size_t N) const { return m_entries[N].getValue() ; }
	 ValT *getValuePtr(size_t N) const { return m_entries[N].getValuePtr() ; }
	 void setValue(size_t N, ValT value) { m_entries[N].setValue(value) ; }
	 bool updateKey(size_t N, KeyT expected, KeyT desired)
	    { return m_entries[N].m_key.compare_exchange_strong(expected,desired) ; }
	 bool activeEntry(size_t N) const { return m_entries[N].isActive() ; }
	 bool deletedEntry(size_t N) const { return m_entries[N].isDeleted() ; }
	 bool unusedEntry(size_t N) const { return m_entries[N].isUnused() ; }
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
	       ifnot_INTERLEAVED(m_ptrs[N] = othertab->m_ptrs[N] ;)
		  return ;
	    }
	 static size_t normalizeSize(size_t sz) ;
	 size_t resizeThreshold(size_t sz)
	    {
	       size_t thresh = (size_t)(sz * m_container->m_maxfill + 0.5) ;
	       return (thresh >= sz) ? sz-1 : thresh ;
	    }
      protected:
	 // the following three functions are defined *after* the declaration of HashTable
	 //   due to the circular dependency of declarations....
	 void announceTable()
	    {
#ifndef FrSINGLE_THREADED
	       Fr::HashTable<KeyT,ValT>::s_table.store(this) ;
#endif /* !FrSINGLE_THREADED */
	       return ;
	    }
	 static void announceBucketNumber(size_t bucket)
	    {
#ifndef FrSINGLE_THREADED
	       Fr::HashTable<KeyT,ValT>::s_bucket.store(bucket) ;
#endif /* !FrSINGLE_THREADED */
	       return ;
	    }
	 static void unannounceBucketNumber()
	    {
#ifndef FrSINGLE_THREADED
	       Fr::HashTable<KeyT,ValT>::s_bucket.store(~0UL) ;
#endif /* !FrSINGLE_THREADED */
	       return ;
	    }

	 Link chainHead(size_t N) const { return ANNOTATE_UNPROTECTED_READ(bucketPtr(N)->head.first) ; }
	 Atomic<Link>* chainHeadPtr(size_t N) const { return &bucketPtr(N)->head.first ; }
	 Link chainNext(size_t N) const { return ANNOTATE_UNPROTECTED_READ(bucketPtr(N)->next) ; }
	 Atomic<Link>* chainNextPtr(size_t N) const { return &bucketPtr(N)->next ; }
	 Link chainOwner(size_t N) const { return bucketPtr(N)->offset.load() ; }
	 void setChainNext(size_t N, Link nxt) { bucketPtr(N)->next.store(nxt) ; }
	 void setChainOwner(size_t N, Link ofs) { bucketPtr(N)->offset.store(ofs) ; }
	 void markCopyDone(size_t N) { bucketPtr(N)->markCopyDone() ; }
	 size_t bucketContaining(size_t N) const
	    { Link owner = chainOwner(N) ; return (owner == FramepaC::NULLPTR) ? NULLPOS : N - owner ; }
	 size_t sizeForCapacity(size_t capacity) const
	    {
	       return (size_t)(capacity / m_container->m_maxfill + 0.99) ;
	    }
	 void autoResize() ;
	 bool bucketsInUse(size_t startbucket, size_t endbucket) const ;
      public:
	 void awaitIdle(size_t bucketnum, size_t endpos) ;
      protected:
	 void removeValue(Entry &element) const
	    {
	       ValT value = element.swapValue((ValT)0) ;
	       if (remove_fn && value)
		  {
		  remove_fn(element.getKey(),value) ;
		  }
	       return ;
	    }
      public:
	 [[gnu::hot]] bool copyChainLocked(size_t bucketnum);
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
	 bool reAdd(size_t hashval, KeyT key, ValT value = 0) ;

	 [[gnu::hot]] bool claimEmptySlot(size_t pos, KeyT key) ;
	 [[gnu::hot]] Link locateEmptySlot(size_t bucketnum, KeyT key, bool &got_resized);

	 [[gnu::hot]] bool insertKey(size_t bucketnum, Link firstptr, KeyT key, ValT value) ;
	 [[gnu::hot]] void resizeCopySegment(size_t segnum) ;
	 [[gnu::hot]] void resizeCopySegments(size_t max_segs = ~0UL) ;
	 void clearDuplicates(size_t bucketnum) ;
	 bool relocateEntry(size_t from, size_t to, size_t bucketnum) ;
	 size_t hopscotchChain(size_t bucketnum, size_t entrynum) ;
	 size_t hopscotch(size_t bucketnum) ;

	 bool unlinkEntry(size_t entrynum) ;
	 size_t recycleDeletedEntry(size_t bucketnum, KeyT new_key) ;
	 bool reclaimChain(size_t bucketnum, size_t &min_reclaimed, size_t &max_reclaimed) ;
	 bool assistResize() ;

	 static Object* makeObject(KeyT key) ;

      public:
	 void onRemove(HashKVFunc *func) { remove_fn = func ; }
	 bool resize(size_t newsize, bool enlarge_only = false) ;

	 [[gnu::hot]] bool add(size_t hashval, KeyT key, ValT value = 0) ;
	 [[gnu::hot]] size_t addCount(size_t hashval, KeyT key, ValT incr) ;

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
	 bool reclaimDeletions() ;

	 // special support for Fr::SymbolTableX
	 template <typename RetT = KeyT>
	 typename std::enable_if<std::is_base_of<Fr::Symbol,KeyT>::value,RetT>::type
	 addKey(size_t hashval, const char* name, size_t namelen, bool* already_existed = 0) ;
	 // (default no-op version for non-Symbol hash tables, selected by SFINAE)
	 template <typename RetT = KeyT>
	 typename std::enable_if<!std::is_base_of<Fr::Symbol,KeyT>::value,RetT>::type
	 addKey(size_t /*hashval*/, const char* /*name*/, size_t /*namelen*/, bool* /*already_existed*/ = 0) ;

	 // special support for Fr::SymbolTable
	 [[gnu::hot]] bool contains(size_t hashval, const char *name, size_t namelen) const ;
	 // special support for Fr::SymbolTableX
	 [[gnu::hot]] KeyT lookupKey(size_t hashval, const char *name, size_t namelen) const ;

	 void replaceValue(size_t pos, ValT new_value) ;

	 //================= Content Statistics ===============
	 [[gnu::cold]] size_t countItems() const;
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
	 static const Link searchrange = FrHASHTABLE_SEARCHRANGE ; // full search window (+/-)
	 static const size_t NULLPOS = ~0UL ;
	 size_t	           m_size ;		// capacity of hash array [constant for life of table]
	 size_t	           m_resizethresh ;	// at what entry count should we grow? [constant]
      protected:
	 Fr::Atomic<size_t> m_currsize { 0 } ;	// number of active entries
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
	    TablePtr 	   *m_next ;
	    Atomic<size_t> *m_bucket ;
	    Atomic<Table*> *m_table { nullptr } ;
	    size_t   	    m_id ;	   // thread ID, for help in debugging
	 public:
	    bool initialized() const { return m_table != nullptr ; }
	    void clear()
	       { m_table = nullptr ; m_bucket = nullptr ; m_next = nullptr ; }
	    void init(Table** tab, size_t* bcket, TablePtr* next)
	       { m_table = reinterpret_cast<Atomic<Table*>*>(tab) ;
		 m_bucket = reinterpret_cast<Atomic<size_t>*>(bcket) ;
		 m_next = next ; m_id = FramepaC::my_job_id ; }
	    void init(Atomic<Table*>* tab, Atomic<size_t>* bcket, TablePtr* next)
	       { m_table = tab ; m_bucket = bcket ;
		 m_next = next ; m_id = FramepaC::my_job_id ; }
	    size_t bucket() const { return m_bucket->load() ; }
	    const Table *table() const { return m_table->load() ; }
         } ;
      //------------------------
      class HazardLock
         {
	 private:
	 public:
#ifdef FrSINGLE_THREADED
	    ALWAYS_INLINE HazardLock(Table *) {}
	    ALWAYS_INLINE ~HazardLock() {}
#else
	    HazardLock(Table *tab)  { HashTable::s_table.store(tab) ; }
	    ~HazardLock() { HashTable::s_table.store((Table*)nullptr) ; }
#endif /* FrSINGLE_THREADED */
         } ;
      //------------------------
      class ScopedChainLock
      {
      private:
#ifndef FrSINGLE_THREADED
	 Table*		 m_table ;
	 size_t		 m_pos ;
	 Atomic<uint8_t>* m_lock ;
#endif /* !FrSINGLE_THREADED */
	 bool		 m_locked ;
	 bool		 m_stale ;
      public:
#ifdef FrSINGLE_THREADED
	 ScopedChainLock(class HashTable::Table *, size_t, bool) { m_locked = m_stale = false ; }
	 ~ScopedChainLock() {}
#else
	 ScopedChainLock(Table *ht, size_t bucketnum)
	    {
	       m_table = ht ;
	       m_pos = bucketnum ;
	       m_lock = reinterpret_cast<Atomic<uint8_t>*>(ht->bucketPtr(bucketnum)->statusPtr()) ;
	       INCR_COUNTstat(chain_lock_count) ;
	       uint8_t oldval = m_lock->test_and_set_mask((uint8_t)HashPtr::lock_mask) ;
	       if ((oldval & HashPtr::lock_mask) == 0)
		  {
		  m_locked = true ;
		  m_stale = (oldval & HashPtr::stale_mask) != 0 ;
		  return ;
		  }
	       INCR_COUNTstat(chain_lock_coll) ;
	       m_locked = false ;
	       m_stale = (oldval & HashPtr::stale_mask) != 0 ;
	       return ;
	    }
	 ~ScopedChainLock()
	    {
	       uint8_t status ;
	       if (m_locked)
		  status = m_lock->test_and_clear_mask((uint8_t)HashPtr::lock_mask) ; 
	       else
		  status = m_lock->load() ;
	       if (!m_stale && (status & HashPtr::stale_mask) != 0)
		  {
		  // someone else tried to copy the chain while we held the lock,
		  //    so copy it for them now
		  m_table->copyChainLocked(m_pos) ;
		  }
	    }
#endif /* FrSINGLE_THREADED */
	 bool locked() const { return m_locked ; }
	 bool stale() const { return m_stale ; }
	 bool busy() const { return !locked() || stale() ; }
#ifdef FrHASHTABLE_STATS
	 static void clearPerThreadStats() { s_stats.chain_lock_count.store(0) ; s_stats.chain_lock_coll.store(0) ; }
	 static size_t numberOfLocks() { return s_stats.chain_lock_count.load() ; }
	 static size_t numberOfLockCollisions() { return s_stats.chain_lock_coll.load() ; }
#else
	 static void clearPerThreadStats() {}
	 static size_t numberOfLocks() { return 0 ; }
	 static size_t numberOfLockCollisions() { return 0 ; }
#endif /* FrHASHTABLE_STATS */
         } ;
      //------------------------
      class ThreadCleanup
         {
	 public:
	    ThreadCleanup() {}
	    ~ThreadCleanup() { HashTable::unregisterThread() ; }
         } ;
      //------------------------

   protected: // members
      Atomic<Table*>   	  m_table ;	// pointer to currently-active m_tables[] entry
      Atomic<Table*>	  m_oldtables { nullptr } ;  // start of list of currently-live hash arrays
      Atomic<Table*>	  m_freetables { nullptr } ;
      Table		  m_tables[FrHT_NUM_TABLES] ;// hash array, chains, and associated data
      HashKeyValueFunc   *cleanup_fn ;	// invoke on destruction of obj
      HashKVFunc         *remove_fn ; 	// invoke on removal of entry/value
      double	          m_maxfill ;	// maximum fill factor before resizing
#ifndef FrSINGLE_THREADED
      static Atomic<TablePtr*> s_thread_entries ;
      static size_t       s_registered_threads ;
      static thread_local TablePtr s_thread_record ;
      static thread_local Atomic<Table*> s_table ;// thread's announcement which hash table it's using
      static thread_local Atomic<size_t> s_bucket ;// thread's announcement which hash bucket it's using
      static thread_local ThreadCleanup cleaner ;
#endif /* FrSINGLE_THREADED */
#ifdef FrHASHTABLE_STATS
      mutable HashTable_Stats	  m_stats ;
      static thread_local HashTable_Stats   s_stats ;
#endif /* FrHASHTABLE_STATS */
      static const size_t m_bit_resize = ((sizeof(size_t)*CHAR_BIT)-1) ;
      static const size_t m_mask_resize = (1UL << m_bit_resize) ; // lock bit for resize

   protected: // debug methods
#if FrHASHTABLE_VERBOSITY > 0
      [[gnu::format(printf,1,2)]] static void warn_msg(const char *fmt, ...)
	 {
	    std::va_list args ;
	    va_start(args,fmt) ;
	    vfprintf(stderr,fmt,args) ;
	    va_end(args) ;
	    return ;
	 }
#else
      [[gnu::format(printf,1,2)]]
      ALWAYS_INLINE static void warn_msg(const char *fmt [[gnu::unused]], ...) { }
#endif /* FrHASHTABLE_VERBOSITY > 0 */
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
      static bool isActive(KeyT p) { return p < Entry::RECLAIMING() ; }
      void init(size_t initial_size, double max_fill, Table *table = nullptr) ;
      Table *allocTable() ;
      void releaseTable(Table *t) ;
      void freeTables() ;
      void updateTable() ;
      bool reclaimSuperseded() ;

      // set up the per-thread info needed for safe reclamation of
      //   entries and hash arrays, as well as an on-exit callback
      //   to clear that info
      static void registerThread() ;
      static void unregisterThread() ;

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
	    return HashTable::isActive(key2) && ::equal(key1,key2) ;
	 }
      static inline bool isEqualFull(KeyT key1, KeyT key2)
	 {
	    // separate version for use by resizeTo(), which needs to use the full
	    //   key name and not just the key's pointer on Fr::SymbolTableX
	    // this is the version for everyone *but* Fr::SymbolTableX
	    return isEqual(key1,key2) ;
	 }
      // special support for Fr::SymbolTableX
      // special support for Fr::SymbolTableX; must be overridden
      static bool isEqual(const char *keyname, size_t namelen, KeyT key) ;


   protected:
//FIXME
      // single-threaded only!!  But since it's only called from a ctor, that's fine
      void copyContents(const HashTable &ht)
         {
	    init(ht.maxSize(),ht.m_maxfill) ;
	    Table *table = m_table.load() ;
	    Table *othertab = ht.m_table.load() ;
	    table->setSize(ht.currentSize()) ;
	    for (size_t i = 0 ; i < table->m_size ; i++)
	       {
	       table->copyEntry(i,othertab) ;
	       }
	    return ;
	 }
      // single-threaded only!!  But since it's only called from a dtor, that's fine
      void clearContents()
         {
	    Table *table = m_table.load() ;
	    if (table && table->good())
	       {
	       if (cleanup_fn)
		  {
		  this->iterate(cleanup_fn) ;
		  cleanup_fn = nullptr ;
		  }
	       if (remove_fn)
		  {
		  for (size_t i = 0 ; i < table->m_size ; ++i)
		     {
		     if (table->activeEntry(i))
			{
			table->replaceValue(i,0) ;
			}
		     }
		  }
	       atomic_thread_fence(std::memory_order_seq_cst) ; 
	       debug_msg("HashTable dtor\n") ;
	       table->awaitIdle(0,table->m_size) ;
	       }
	    table->clear() ;
	    freeTables() ;
	    return ;
	 }

      static size_t stillLive(const Table *version)
	 {
#ifndef FrSINGLE_THREADED
	    // scan the list of per-thread s_table variables
	    for (const TablePtr *tables = s_thread_entries.load() ; tables ; tables = ANNOTATE_UNPROTECTED_READ(tables->m_next))
	       {
	       // check whether the hazard pointer is for the requested version
	       if (tables->table() == version)
		  {
		  // yep, the requested version is still live
		  return true ;
		  }
	       }
#endif /* !FrSINGLE_THREADED */
	    (void)version ;
	    // nobody else is using that version
	    return false ;
	 }

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
	       reclaimSuperseded() ;			\
	    size_t hashval = hashVal(key) ; 		\
	    return m_table.load()->delegate ; 
#else
#define DELEGATE(delegate) 				\
            HazardLock hl(m_table.load()) ; 		\
	    return m_table.load()->delegate ;
#define DELEGATE_HASH(delegate) 			\
	    size_t hashval = hashVal(key) ; 		\
	    HazardLock hl(m_table.load()) ;		\
	    return m_table.load()->delegate ;
#define DELEGATE_HASH_RECLAIM(type,delegate)		\
	    if (m_oldtables.load() != m_table.load())	\
	       reclaimSuperseded() ;			\
	    size_t hashval = hashVal(key) ; 		\
	    HazardLock hl(m_table.load()) ;		\
	    return m_table.load()->delegate ;
#endif /* FrSINGLE_THREADED */

      // ============== The public API for HashTable ================
   public:
      HashTable(size_t initial_size = 1031, double max_fill = 0.0)
	 : Object(), m_table(nullptr)
	 {
	    init(initial_size,max_fill) ;
	    return ;
	 }
      HashTable(const HashTable &ht)
	 : Object(), m_table(nullptr)
	 {
	    if (&ht == nullptr)
	       return ;
	    copyContents(ht) ;
	    return ;
	 }
      virtual ~HashTable()
	 {
	    clearContents() ;
	    remove_fn = nullptr ;
	    return ;
	 }

      size_t sizeForCapacity(size_t capacity) const
	 {
	    return (size_t)(capacity / m_maxfill + 0.99) ;
	 }

      bool resizeTo(size_t newsize, bool enlarge_only = false)
	 { DELEGATE(resize(newsize,enlarge_only)) }
      void resizeToFit(size_t new_capacity)
	 {
	    resizeTo(sizeForCapacity(new_capacity)) ;
	    return ;
	 } ;
      bool reserve_(size_t newsize) { DELEGATE(resize(newsize,true) ; }
      void reserve(size_t newsize) { reserve_(newsize) ; }
      void shrinK_to_fit() { resizeToFit(currentSize()) ; }

      bool reclaimDeletions() { DELEGATE(reclaimDeletions()) }

      [[gnu::hot]] bool add(KeyT key, ValT value = 0) { DELEGATE_HASH_RECLAIM(bool,add(hashval,key,value)) }
      [[gnu::hot]] size_t addCount(KeyT key, ValT incr) { DELEGATE_HASH_RECLAIM(size_t,addCount(hashval,key,incr)) }
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
	       reclaimSuperseded() ;
	    size_t hashval = hashVal(key) ;
	    HazardLock hl(m_table.load()) ;
	    if (replace)
	       m_table.load()->remove(hashval,key) ;
	    return m_table.load()->add(hashval,key,value) ;
	 }
      // special support for Fr::SymbolTableX
      [[gnu::hot]] KeyT addKey(const char *name, bool *already_existed = 0)
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
	       return 0 ;
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
      void onRemove(HashKVFunc *func) { remove_fn = func ; if (m_table.load()) m_table.load()->onRemove(func) ; }

      void setMaxFill(double fillfactor) ;

      // access to internal state
      size_t currentSize() const { DELEGATE(currentSize()) }
      size_t maxCapacity() const { DELEGATE(m_size) }
      bool isPacked() const { return false ; }  // backwards compatibility
      HashKeyValueFunc *cleanupFunc() const { return cleanup_fn ; }
      HashKVFunc *onRemoveFunc() const { return remove_fn ; }

      // =============== Operational Statistics ================
      [[gnu::cold]] void clearGlobalStats()
	 {
#ifdef FrHASHTABLE_STATS
	    m_stats.clear() ;
#endif /* FrHASHTABLE_STATS */
	    return ;
	 }
      [[gnu::cold]] static void clearPerThreadStats()
	 {
#if defined(FrHASHTABLE_STATS)
	    s_stats.clear();
	    ScopedChainLock::clearPerThreadStats() ;
#endif /* FrHASHTABLE_STATS */
	    return ;
	 }
      [[gnu::cold]] void updateGlobalStats()
	 {
#if defined(FrHASHTABLE_STATS)
	    m_stats.add(&s_stats) ;
	    m_stats.chain_lock_count += ScopedChainLock::numberOfLocks() ;
	    m_stats.chain_lock_coll += ScopedChainLock::numberOfLockCollisions() ;
#endif /* FrHASHTABLE_STATS */
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
      size_t numberOfReclamations() const { return m_stats.reclaim ; }
      size_t numberOfEntriesMoved() const { return m_stats.move ; }
      size_t numberOfFullNeighborhoods() const { return  m_stats.neighborhood_full ; }
      size_t numberOfCASCollisions() const { return m_stats.CAS_coll ; }
      size_t numberOfChainLocks() const { return m_stats.chain_lock_count.load() ; }
      size_t numberOfChainLockCollisions() const { return m_stats.chain_lock_coll.load() ; }
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
      static size_t numberOfReclamations() { return 0 ; }
      static size_t numberOfEntriesMoved() { return 0 ; }
      static size_t numberOfFullNeighborhoods() { return 0 ; }
      static size_t numberOfCASCollisions() { return 0 ; }
      static size_t numberOfChainLocks() { return 0 ; }
      static size_t numberOfChainLockCollisions() { return 0 ; }
      static size_t numberOfSpins() { return 0 ; }
      static size_t numberOfYields() { return 0 ; }
      static size_t numberOfSleeps() { return 0 ; }
#endif

      // ============= Fr::Object support =============
      const char* typeName_() const { return "HashTable" ; }
//!      virtual FrObjectType objSuperclass() const { return OT_FrObject ; }
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

} // end namespace Fr

#undef DELEGATE
#undef DELEGATE_HASH
#undef DELEGATE_HASH_RECLAIM

#endif /* !__Fr_HASHTABLE_H_INCLUDED */

// end of file hashtable.h //
