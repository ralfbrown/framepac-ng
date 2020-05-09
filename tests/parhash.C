/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-15					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2015,2017,2018,2019 Carnegie Mellon University		*/
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

#include <iomanip>
#include <pthread.h>
#include <signal.h>
#include <sstream>
#include <unistd.h>
#include <unordered_set>
#include "framepac/argparser.h"
#include "framepac/fasthash64.h"
#include "framepac/hashtable.h"
#include "framepac/message.h"
#include "framepac/random.h"
#include "framepac/symboltable.h"
#include "framepac/texttransforms.h"
#include "framepac/threadpool.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

// uncomment the following line to enable testing the Herlihy et al (2008) Hopscotch hash table
#define TEST_HOPSCOTCH

// uncomment the following line to show statistcs on the lengths
//   of the chains in the hash array
#define SHOW_CHAINS
#define SHOW_LOST_CHAINS

#define INTEGER_TYPE uint32_t

// uncomment the next line to add synchronization to STLset accesses
#define STL_MUTEX

// how should synchronization handle contention?  Uncomment to spin,
//   comment out to use blocking condition variable
#define STL_MUTEX_SPIN

// without synchronization in class STLset, we can only write from one thread
//   at a time; if synchronization is implemented, we can use as many threads as we want
#ifdef STL_MUTEX
# define STL_WRITE_THREADS threads
#else
# define STL_WRITE_THREADS 1
#endif

/************************************************************************/
/************************************************************************/

#ifdef TEST_HOPSCOTCH

// typedefs used by HopscotchHashMap
typedef uint32_t _u32 ;
#define inline_ inline
// functions used by HopscotchHashMap (forward declarations)
static int first_msb_bit_indx(INTEGER_TYPE) ;
#include "hopscotch/HopscotchHashMap.h"

#endif /* TEST_HOPSCOTCH */

/************************************************************************/
/*	Type declarations						*/
/************************************************************************/

enum Operation
{
   Op_NONE,
   Op_GENSYM,
   Op_ADD,
   Op_CHECK,
   Op_CHECKMISS,
   Op_CHECKSYMS,
   Op_REMOVE,
   Op_RANDOM,
   Op_RANDOM_HIGHREMOVE,
   Op_RANDOM_LOWREMOVE,
   Op_RANDOM_NOREMOVE,
   Op_RANDOM_ADDONLY,
   Op_RECLAIM,
   Op_THROUGHPUT			// timed throughput test a la Herlihy et al
} ;

typedef void HashRequestFunc(class HashRequestOrder*) ;

class HashRequestOrder
   {
   public:
      Operation	  op ;
      size_t	  size ;
      size_t	  cycles ;
      size_t	  id ;
      size_t	  slices ;
      size_t	  slice_start ;
      size_t	  slice_size ;
      size_t	  current_cycle ;
      size_t	  total_ops ;
      size_t	  lookup_frac ;
      bool	  m_verbose ;
      atom_bool   m_terse ;
      bool	  strict ;		// check results?
      unsigned	  extra_arg ;
      uint32_t*   randnums ;
      HashRequestFunc* func ;
      void*       ht ;
      Symbol**    syms ;
      ThreadPool* pool ;
   public:
      HashRequestOrder() { current_cycle = 0 ; }
      ~HashRequestOrder() {}
   } ;

// STL_MUTEX_SPIN variant is a non-distributed version of Dmitry
// Vyukov's distributed reader-writer lock from www.1024cores.net
class RWLock
   {
   public:
      RWLock()
#ifdef STL_MUTEX_SPIN
	 : m_critsect(), m_write(false), m_readers(0)
	 {
#else
	 {
	 pthread_rwlock_init(&m_lock,nullptr) ;
#endif /* STL_MUTEX_SPIN */
	 }
      ~RWLock()
	 {
#ifdef STL_MUTEX_SPIN
#else
	 pthread_rwlock_destroy(&m_lock) ;
#endif /* STL_MUTEX_SPIN */
	 }
      void readStart()
	 {
#ifdef STL_MUTEX_SPIN
	 m_readers++ ;
	 if (m_write)
	    {
	    // cancel read request until writer completes
	    m_readers-- ;
	    m_critsect.lock() ;
	    m_readers++ ;
	    m_critsect.unlock() ;
	    }
	 memoryBarrier() ;
#else
	 (void)pthread_rwlock_rdlock(&m_lock) ;
#endif /* STL_MUTEX_SPIN */
	 }
      void readDone()
	 {
#ifdef STL_MUTEX_SPIN
	 memoryBarrier() ;
	 m_readers-- ;
#else
	 (void)pthread_rwlock_unlock(&m_lock) ;
#endif /* STL_MUTEX_SPIN */
	 }
      void writeStart()
	 {
#ifdef STL_MUTEX_SPIN
	 m_critsect.lock() ;
	 m_write = true ;
	 memoryBarrier() ;
#else
	 (void)pthread_rwlock_wrlock(&m_lock) ;
#endif /* STL_MUTEX_SPIN */
	 }
      void writeDone()
	 {
#ifdef STL_MUTEX_SPIN
	 m_write = false ;
	 m_critsect.unlock() ;
#else
	 (void)pthread_rwlock_unlock(&m_lock) ;
#endif /* STL_MUTEX_SPIN */
	 }
   protected:
#ifdef STL_MUTEX_SPIN
      CriticalSection  m_critsect ;
      atom_bool        m_write ;
      atom_int	       m_readers ;
#else
      pthread_rwlock_t m_lock ;
#endif /* STL_MUTEX_SPIN */
   } ;

class ScopedReadLock
   {
   public:
      ScopedReadLock(RWLock& lock)
#ifdef STL_MUTEX
	 : m_lock(lock)
	 {
	 lock.readStart() ;
#else
	 {
	 (void)lock ;
#endif /* STL_MUTEX */
	 }
      ~ScopedReadLock()
	 {
#ifdef STL_MUTEX
	 m_lock.readDone() ;
#endif /* STL_MUTEX */
	 }
   protected:
#ifdef STL_MUTEX
      RWLock& m_lock ;
#endif /* STL_MUTEX */
   } ;

class ScopedWriteLock
   {
   public:
      ScopedWriteLock(RWLock& lock)
#ifdef STL_MUTEX
	 : m_lock(lock)
	 {
	 lock.writeStart() ;
#else
	 {
	 (void)lock ;
#endif /* STL_MUTEX */
	 }
      ~ScopedWriteLock()
	 {
#ifdef STL_MUTEX
	 m_lock.writeDone() ;
#endif /* STL_MUTEX */
	 }
   protected:
#ifdef STL_MUTEX
      RWLock& m_lock ;
#endif /* STL_MUTEX */
   } ;
   
//----------------------------------------------------------------------

class STLset : public unordered_set<INTEGER_TYPE>
   {
   public:
      STLset(size_t init_size) : unordered_set<INTEGER_TYPE>(init_size)
	 {
	 }
      ~STLset() = default ;
      static STLset* create(size_t init_size) { return new STLset(init_size) ; }
      void free() { delete this ; }

      bool add(INTEGER_TYPE key)
	 {
	    ScopedWriteLock _(m_lock) ;
	    return !insert(key).second ;
	 }
      bool contains(INTEGER_TYPE key)
	 {
	    ScopedReadLock _(m_lock) ;
	    return count(key) != 0 ;
	 }
      bool remove(INTEGER_TYPE key)
	 {
	    ScopedWriteLock _(m_lock) ;
	    return erase(key) != 0 ;
	 }
      size_t* chainLengths(size_t& max_length) const { max_length = 0 ; return nullptr ; }
      size_t* neighborhoodDensities(size_t& num_densities) const { num_densities = 0 ; return nullptr ; }
      using unordered_set<INTEGER_TYPE>::begin ;
      using unordered_set<INTEGER_TYPE>::cbegin ;
      using unordered_set<INTEGER_TYPE>::end ;
      using unordered_set<INTEGER_TYPE>::cend ;
      static void threadInit() {}
      void clearGlobalStats() {}
      static void clearPerThreadStats() {}
      void updateGlobalStats() {}
      void reclaimDeletions(size_t, size_t) {}
      size_t currentSize() const { return size() ; }
      size_t countItems() const { return size() ; }
      size_t countDeletedItems() const { return 0 ; }
      size_t numberOfInsertions() const { return 0 ; }
      size_t numberOfDupInsertions() const { return 0 ; }
      size_t numberOfInsertionAttempts() const { return 0 ; }
      size_t numberOfForwardedInsertions() const { return 0 ; }
      size_t numberOfResizeInsertions() const { return 0 ; }
      size_t numberOfContainsCalls() const { return 0 ; }
      size_t numberOfSuccessfulContains() const { return 0 ; }
      size_t numberOfForwardedContains() const { return 0 ; }
      size_t numberOfLookups() const { return 0 ; }
      size_t numberOfSuccessfulLookups() const { return 0 ; }
      size_t numberOfForwardedLookups() const { return 0 ; }
      size_t numberOfRemovals() const { return 0 ; }
      size_t numberOfItemsRemoved() const { return 0 ; }
      size_t numberOfForwardedRemovals() const { return 0 ; }
      size_t numberOfResizes() const { return 0 ; }
      size_t numberOfResizeAssists() const { return 0 ; }
      size_t numberOfResizeWaits() const { return 0 ; }
      size_t numberOfReclamations() const { return 0 ; }
      size_t numberOfFullNeighborhoods() const { return 0 ; }
      size_t numberOfSpins() const { return 0 ; }
      size_t numberOfYields() const { return 0 ; }
      size_t numberOfSleeps() const { return 0 ; }
      size_t numberOfCASCollisions() const { return 0 ; }
      size_t numberOfResizeCleanups() const { return 0 ; }
   private:
      RWLock m_lock ;
   } ;

//----------------------------------------------------------------------

#ifdef TEST_HOPSCOTCH

// adapter to API HopscotchHashMap expects its hashers to use
class HopscotchHASH
   {
   public:
      static const unsigned int _EMPTY_HASH ;
      static const unsigned int _BUSY_HASH ;
      static const INTEGER_TYPE _EMPTY_KEY ;
      static const short _EMPTY_DATA ;

      static unsigned int Calc(INTEGER_TYPE key)
	 {
	    unsigned int hash = (unsigned int)(key) ;
	    return hash == _EMPTY_HASH ? 0 : hash ;
	 }
      static bool IsEqual(INTEGER_TYPE key1, INTEGER_TYPE key2) { return key1 == key2 ; }
      static void relocate_key_reference(INTEGER_TYPE volatile& key1, const INTEGER_TYPE volatile& key2)
	 { key1 = key2 ; }
      static void relocate_data_reference(short volatile &data1, const short volatile& data2)
	 { data1 = data2 ; }
      static void relocate_data_reference(short& data1, const short& data2)
	 { data1 = data2 ; }
      
   } ;

const INTEGER_TYPE HopscotchHASH::_EMPTY_KEY = (INTEGER_TYPE)(~0ULL) ;
const short HopscotchHASH::_EMPTY_DATA = 0 ;
const unsigned int HopscotchHASH::_EMPTY_HASH = ~0U ;
const unsigned int HopscotchHASH::_BUSY_HASH = ~1U ;

// adapter to API HopscotchHashMap expects its locks to use
//class HopscotchLock : public Fr::CriticalSection
class HopscotchLock : public std::mutex
   {
   public:
      HopscotchLock() {}
      ~HopscotchLock() {}
      void init() {}
      //inherited: void lock();
      //inherited: void unlock();
      bool tryLock() { return try_lock() ; }
//      bool isLocked() const { return locked() ; }
   } ;

// adapter to API HopscotchMap expects its memory allocator to use
class HopscotchAllocator
   {
   public:
      static void* byte_aligned_malloc(size_t n, size_t align = 64)
	 {
	    void* alloc;
	    return posix_memalign(&alloc, align, n) ? nullptr : alloc ;
	 }
      static void byte_aligned_free(void* ptr) { free(ptr) ; }
   } ;

static int first_msb_bit_indx(INTEGER_TYPE val)
{
   return val ? __builtin_clz(val)-1 : -1 ;
}

static int g_Hopscotch_Concurrency = 32 ;

class HopscotchMap
   {
   public:
      HopscotchHashMap<INTEGER_TYPE,short,HopscotchHASH,HopscotchLock,HopscotchAllocator> ht ;
   public:
      HopscotchMap(size_t init_size, size_t concur = g_Hopscotch_Concurrency)
	 : ht(init_size,concur,64,false) //capacity,concurrency,cacheline,opt_cl
	 {
	 }
      ~HopscotchMap() = default ;
      static HopscotchMap* create(size_t init_size,size_t concur = g_Hopscotch_Concurrency)
	 { return new HopscotchMap(init_size,concur) ; }
      void free() { delete this ; }

      bool add(INTEGER_TYPE key) { (void)ht.putIfAbsent(key,0) ; return false; }
      bool contains(INTEGER_TYPE key) { return ht.containsKey(key) ; }
      bool remove(INTEGER_TYPE key) { (void)ht.remove(key) ; return false; }
      size_t size() const { return ht.size() ; }
      size_t* chainLengths(size_t& max_length) const { max_length = 0 ; return nullptr ; }
      size_t* neighborhoodDensities(size_t& num_densities) const { num_densities = 0 ; return nullptr ; }
      static void threadInit() {}
      void clearGlobalStats() {}
      static void clearPerThreadStats() {}
      void updateGlobalStats() {}
      void reclaimDeletions(size_t, size_t) {}
      size_t currentSize() const { return size() ; }
      size_t countItems() const { return size() ; }
      size_t countDeletedItems() const { return 0 ; }
      size_t numberOfInsertions() const { return 0 ; }
      size_t numberOfDupInsertions() const { return 0 ; }
      size_t numberOfInsertionAttempts() const { return 0 ; }
      size_t numberOfForwardedInsertions() const { return 0 ; }
      size_t numberOfResizeInsertions() const { return 0 ; }
      size_t numberOfContainsCalls() const { return 0 ; }
      size_t numberOfSuccessfulContains() const { return 0 ; }
      size_t numberOfForwardedContains() const { return 0 ; }
      size_t numberOfLookups() const { return 0 ; }
      size_t numberOfSuccessfulLookups() const { return 0 ; }
      size_t numberOfForwardedLookups() const { return 0 ; }
      size_t numberOfRemovals() const { return 0 ; }
      size_t numberOfItemsRemoved() const { return 0 ; }
      size_t numberOfForwardedRemovals() const { return 0 ; }
      size_t numberOfResizes() const { return 0 ; }
      size_t numberOfResizeAssists() const { return 0 ; }
      size_t numberOfResizeWaits() const { return 0 ; }
      size_t numberOfReclamations() const { return 0 ; }
      size_t numberOfFullNeighborhoods() const { return 0 ; }
      size_t numberOfSpins() const { return 0 ; }
      size_t numberOfYields() const { return 0 ; }
      size_t numberOfSleeps() const { return 0 ; }
      size_t numberOfCASCollisions() const { return 0 ; }
      size_t numberOfResizeCleanups() const { return 0 ; }
   } ;

#endif /* TEST_HOPSCOTCH */

/************************************************************************/
/*	Global variables for this module				*/
/************************************************************************/

static atom_bool stop_run { false } ;
static bool show_neighbors { false } ;
static int time_limit { 4 } ;
      
/************************************************************************/
/*	Syntactic sugar for conditional compilation			*/
/************************************************************************/

#ifdef SHOW_CHAINS
#  define if_SHOW_CHAINS(x) x
#else
#  define if_SHOW_CHAINS(x)
#endif

#ifdef SHOW_NEIGHBORHOODS
#  define if_SHOW_NEIGHBORS(x) x
#else
#  define if_SHOW_NEIGHBORS(x)
#endif

/************************************************************************/
/*	Members for class HashRequestOrder				*/
/************************************************************************/

using FramepaC::my_job_id ;

/************************************************************************/
/*	Helper functions						*/
/************************************************************************/

static void pretty_print(size_t val, ostream &out)
{
   if (val < 10000)
      out << val ;
   else
      {
      char buf[200] ;
      int digits = 0 ;
      while (val)
	 {
	 buf[digits++] = '0' + (val % 10) ;
	 val /= 10 ;
	 if (val && (digits % 4) == 3)
	    buf[digits++] = ',' ;
	 }
      while (digits)
	 {
	 out << buf[--digits] ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

[[gnu::format(gnu_printf,2,3)]]
static void print_msg(ostream& out, const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   CharPtr msg { Fr::vaprintf(fmt,args) } ;
   va_end(args) ;
   if (msg)
      {
      out << *msg << flush  ;
      }
   return  ;
}

/************************************************************************/
/************************************************************************/

static void hash_nop(HashRequestOrder*)
{
   return ;
}

//----------------------------------------------------------------------------

static void hash_gensym(HashRequestOrder* order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   Symbol** syms = order->syms ;
   // generate the symbols in multiple interleaved passes so that we
   //   don't end up with strictly increasing (and thus well-cached)
   //   hash keys during insertion
   SymbolTable* symtab = SymbolTable::current() ;
   size_t passes = 53 ;
   for (size_t pass = 0 ; pass < passes ; ++pass)
      {
      for (size_t i = order->slice_start + pass ; i < slice_end ; i += passes)
	 {
	 // generate our own unique symbol to avoid contention in gensym()
	 char name[100] ;
	 snprintf(name,sizeof(name),"%c%lu%c",(char)('A' + (order->id % 26)),i,'\0') ;
	 syms[i] = symtab->add(name) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

template <class KeyT>
static void err_msg(const char* what, size_t id, KeyT obj)
{
   // put everything into a single string to minimize the chances of the output getting interleaved
   //   with that from other threads 
   auto str = aprintf(";  Job %lu encountered %s %s\n",id,what,*as_string(obj)) ;
   cerr << str << flush ;
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_add(HashRequestOrder* order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   HashT* ht = (HashT*)order->ht ;
   KeyT* syms  = (KeyT*)order->syms ;
   for (size_t i = order->slice_start ; i < slice_end ; ++i)
      {
      KeyT sym = syms[i] ;
      if (ht->add(sym))
	 {
	 err_msg("symbol already in the table:",order->id,sym) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_check(HashRequestOrder* order)
{
   my_job_id = order->id ;
   bool missing = (bool)order->extra_arg ;
   size_t slice_end = order->slice_start + order->slice_size ;
   HashT* ht = (HashT*)order->ht ;
   KeyT* syms  = (KeyT*)order->syms ;
   if (missing)
      {
      for (size_t i = order->slice_start ; i < slice_end ; ++i)
	 {
	 KeyT sym = syms[i] ;
	 if (ht->contains(sym))
	    {
	    err_msg("spurious symbol",order->id,sym) ;
	    }
	 }
      }
   else
      {
      for (size_t i = order->slice_start ; i < slice_end ; ++i)
	 {
	 KeyT sym = syms[i] ;
	 if (!ht->contains(sym) && order->strict)
	    {
	    err_msg("missing symbol",order->id,sym) ;
	    }
	 }
      }
   if (order->m_verbose)
      {
      CharPtr msg { aprintf(";  Job %lu cycle %lu complete.\n",order->id,order->current_cycle) } ;
      cout << *msg << flush ;
      }
   return ;
}

//----------------------------------------------------------------------

static bool find_Symbol(const SymbolTable* symtab, const Symbol* sym)
{
   return symtab->find(sym) != nullptr ;
}

static bool find_Symbol(const SymbolTable*, INTEGER_TYPE) { return false ; }

//----------------------------------------------------------------------

static const char* sym_name(const Symbol* sym) { return sym->name() ; }

static const char* sym_name(INTEGER_TYPE) { return "" ; }

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_checksyms(HashRequestOrder* order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   KeyT* syms  = (KeyT*)order->syms ;
   SymbolTable* symtab = SymbolTable::current() ;
   for (size_t i = order->slice_start ; i < slice_end ; ++i)
      {
      if (!find_Symbol(symtab,syms[i]) && order->strict)
	 {
	 print_msg(cerr,";  Job %lu - missing symbol %s\n",
		   order->id,sym_name(syms[i])) ;
	 }
      }
   if (order->m_verbose)
      {
      print_msg(cout,";  Job %lu cycle %ld complete.\n",order->id,order->current_cycle) ;
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_remove(HashRequestOrder* order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   HashT* ht = (HashT*)order->ht ;
   KeyT* syms  = (KeyT*)order->syms ;
   for (size_t i = order->slice_start ; i < slice_end ; ++i)
      {
      if (!ht->remove(syms[i]) && order->strict)
	 {
	 print_msg(cerr,";  Job %ld encountered missing symbol @ %ld.\n",order->id,i) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_random(HashRequestOrder* order)
{
   my_job_id = order->id ;
   unsigned remove_frac = order->extra_arg ;
   HashT* ht = (HashT*)order->ht ;
   KeyT* syms = (KeyT*)order->syms ;
   uint32_t* randnums = order->randnums + order->slice_start ;
   for (size_t i = 0 ; i < order->slice_size ; ++i)
      {
      size_t which = randnums[i] ;
      ++order->total_ops ;
      if (!ht->contains(syms[which]))
	 {
	 (void)ht->add(syms[which]) ;
	 ++order->total_ops ;
	 }
      else if (remove_frac == 0)
	 {
	 (void)ht->add(syms[which]) ;
	 ++order->total_ops ;
	 }
      else if (((which ^ i) % 10) < remove_frac)
	 {
	 (void)ht->remove(syms[which]) ;
	 ++order->total_ops ;
	 }
      }
   if (order->m_verbose)
      {
      print_msg(cout,";  Job %ld cycle %ld complete.\n",order->id,order->current_cycle) ;
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_random_add(HashRequestOrder* order)
{
   my_job_id = order->id ;
   HashT* ht = (HashT*)order->ht ;
   KeyT* syms = (KeyT*)order->syms ;
   uint32_t* randnums = order->randnums + order->slice_start ;
   for (size_t i = 0 ; i < order->slice_size ; ++i)
      {
      size_t which = randnums[i] ;
      (void)ht->add(syms[which]) ;
      }
   if (order->m_verbose)
      {
      print_msg(cout,";  Job %ld cycle %ld complete.\n",order->id,order->current_cycle) ;
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_throughput(HashRequestOrder* order)
{
   my_job_id = order->id ;
   HashT* ht = (HashT*)order->ht ;
   KeyT* keys1 = (KeyT*)order->syms + order->slice_start ;
   KeyT* keys2 = (KeyT*)order->syms + order->size + order->slice_start ;
   KeyT* lookup_keys1 = keys1 ;
   KeyT* lookup_keys2 = keys2 ;
   size_t lookup_frac = order->lookup_frac ;
   size_t modify_frac = (100 - lookup_frac) / 2 ;
   // test like in Herlihy et al:
   //   insert/remove (100-N) elements
   //   lookup N elements which are in the table and N which are not
   //   swap in-table and not-in-table arrays once the slice is exhausted
   //   loop until 'stop' variable is set by main thread
   size_t modify_offset = 0 ;
   size_t lookup_offset = order->slice_size-1 ;
   while (!stop_run)
      {
      for (size_t i = 0 ; i < modify_frac ; ++i)
	 {
	 (void)ht->remove(keys1[modify_offset]) ;
	 (void)ht->add(keys2[modify_offset]) ;
	 if (++modify_offset >= order->slice_size)
	    {
	    std::swap(keys1,keys2) ;
	    modify_offset = 0 ;
	    }
	 }
      for (size_t i = 0 ; i < lookup_frac ; ++i)
	 {
	 if (lookup_offset-- == 0)
	    {
	    std::swap(lookup_keys1,lookup_keys2) ;
	    lookup_offset = order->slice_size-1 ;
	    }
	 (void)ht->contains(lookup_keys1[lookup_offset]) ;
	 (void)ht->contains(lookup_keys2[lookup_offset]) ;
	 }
      order->total_ops += 2 * (modify_frac + lookup_frac) ;
      }
   if (order->m_verbose)
      {
      print_msg(cout,";  Job %ld complete.\n",order->id) ;
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT>
static void hash_dispatch(const void* input, void* /*output*/ )
{
   HashT::threadInit() ;  // should not be needed once HashTable is completely fixed
   HashRequestOrder* order = (HashRequestOrder*)input ;
   my_job_id = order->id ;
   order->current_cycle = 1 ;
   while (order->current_cycle <= order->cycles)
      {
      order->func(order) ;
      order->current_cycle++ ;
      }
   // keep ThreadSanitizer happy by using an atomic read, even though
   //   it isn't actually necessary
   if (!order->m_terse.load() && order->current_cycle > order->cycles)
      {
      // try to ensure that the message doesn't get interleaved with
      //   other completion messages by generating a single string
      //   written with a single call
      print_msg(cout,"  Job %ld done.\n",order->id) ;
      }
   if (order->ht)
      {
      ((HashT*)order->ht)->updateGlobalStats() ;
      HashT::clearPerThreadStats() ;
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT>
static void reclaim_deletion(const void* input, void*)
{
   HashT::threadInit() ;  // should not be needed once HashTable is completely fixed
   const HashRequestOrder* order = reinterpret_cast<const HashRequestOrder*>(input) ;
   HashT* ht = (HashT*)order->ht ;
   if (ht)
      {
      ht->reclaimDeletions(order->slices,order->cycles) ;
      ht->updateGlobalStats() ;
      HashT::clearPerThreadStats() ;
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT>
static void reclaim_deletions(HashT* ht, ThreadPool* tpool, HashRequestOrder* hashorders, size_t slices)
{
   if (ht)
      {
      for (size_t i = 0 ; i < slices ; ++i)
	 {
	 hashorders[i].ht = (void*)ht ;
	 hashorders[i].cycles = i ;
	 hashorders[i].slices = slices ;
	 tpool->dispatch(&reclaim_deletion<HashT>,&hashorders[i],nullptr) ;
	 }
      tpool->waitUntilIdle() ;
      }
   return ;
}

//----------------------------------------------------------------------

static void announce(ostream& out, bool terse, const char *msg, const char* type,
		     size_t threads)
{
   if (terse)
      {
      out << type << ',' << threads << ',' << msg << ',' << flush ;
      }
   else
      out << msg << ' ' << endl ;
   return ;
}

//----------------------------------------------------------------------

void announce(ostream& out, bool terse, const char *msg, size_t threads, HashSet_U32*)
{
   announce(out,terse,msg,"int",threads) ;
   return ;
}

//----------------------------------------------------------------------

void announce(ostream& out, bool terse, const char *msg, size_t threads, ObjHashTable*)
{
   announce(out,terse,msg,"obj",threads) ;
   return ;
}

//----------------------------------------------------------------------

void announce(ostream& out, bool terse, const char *msg, size_t threads, STLset*)
{
   announce(out,terse,msg,"STL",threads) ;
   return ;
}

//----------------------------------------------------------------------

#ifdef TEST_HOPSCOTCH
void announce(ostream& out, bool terse, const char *msg, size_t threads, HopscotchMap*)
{
   CharPtr type { Fr::aprintf("Hopsc%03d",g_Hopscotch_Concurrency) } ;
   announce(out,terse,msg,*type,threads) ;
   return ;
}
#endif /* TEST_HOPSCOTCH */

//----------------------------------------------------------------------

static void timeout_handler(int)
{
   stop_run = true ;
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_test(ThreadPool* user_pool, ostream& out, const char* heading, size_t threads,
   		      size_t cycles, HashT* ht, size_t maxsize, KeyT* syms, enum Operation op,
		      bool terse, double overhead = 0.0, bool strict = true, uint32_t* randnums = nullptr)
{
   if (heading && *heading)
      announce(out,terse,heading,threads,ht) ;
   // if we are not protecting STL unordered_set from concurrent writes, we may
   //   get a value for 'threads' that is less than the number of threads in the
   //   given thread pool; in that case, instantiate a private pool with just the
   //   requested number of threads
   if (threads <= 1) user_pool = nullptr ;
   ThreadPool* tpool = user_pool ? user_pool : new ThreadPool(threads) ;
   size_t slices = threads ;
   if (threads == 0)
      slices = 1 ;			// only a single slice when running single-threaded
   else
      {
      // use somewhat finer slices if each segment would be really large,
      //   to avoid the case where one slice takes much longer than others
      for (size_t loop = 1 ; loop <= 2 ; ++loop)
	 {
	 if (maxsize > 2000000 * slices && slices < 512) slices *= 2 ;
	 }
      }
   LocalAlloc<HashRequestOrder> hashorders(slices) ;
   size_t slice_size = (maxsize + slices/2) / slices ;
   if (ht)
      {
      ht->clearGlobalStats() ;
      ht->clearPerThreadStats() ;
      }
   stop_run = false ;
   Timer timer ;
   struct sigaction oldsig ;
   if (op == Op_THROUGHPUT && threads == 0)
      {
      // when running single-threaded, set up an alarm for N seconds from now; the handler will set stop_run
      struct sigaction action ;
      action.sa_handler = timeout_handler ;
      sigemptyset(&action.sa_mask) ;
      action.sa_flags = 0 ;
      sigaction(SIGALRM,&action,&oldsig) ;
      alarm(cycles) ;
      }
   for (size_t i = 0 ; i < slices ; ++i)
      {
      hashorders[i].op = op ;
      hashorders[i].size = maxsize ;
      hashorders[i].ht = (void*)ht ;
      hashorders[i].syms = (Symbol**)syms ;
      hashorders[i].randnums = randnums ;
      hashorders[i].strict = strict ;
      hashorders[i].m_verbose = false ;
      hashorders[i].m_terse = terse ;
      hashorders[i].cycles = cycles ;
      hashorders[i].id = i+1 ;
      hashorders[i].slices = slices ;
      hashorders[i].pool = tpool ;
      hashorders[i].slice_start = i * slice_size ;
      hashorders[i].slice_size = (i+1 < slices) ? slice_size : (maxsize - hashorders[i].slice_start) ;
      hashorders[i].extra_arg = 0 ;
      hashorders[i].total_ops = 0 ;
      hashorders[i].lookup_frac = 0 ;
      switch (op)
	 {
	 case Op_NONE:
	    hashorders[i].func = hash_nop ;
	    break ;
	 case Op_GENSYM:
	    hashorders[i].func = hash_gensym ;
	    break ;
	 case Op_ADD:
	    hashorders[i].func = hash_add<HashT,KeyT> ;
	    break ;
	 case Op_CHECK:
	    hashorders[i].func = hash_check<HashT,KeyT> ;
	    break ;
	 case Op_CHECKMISS:
	    hashorders[i].func = hash_check<HashT,KeyT> ;
	    hashorders[i].extra_arg = 1 ;
	    break ;
	 case Op_CHECKSYMS:
	    hashorders[i].func = hash_checksyms<HashT,KeyT> ;
	    break ;
	 case Op_REMOVE:
	    hashorders[i].func = hash_remove<HashT,KeyT> ;
	    break ;
	 case Op_RANDOM:
	    hashorders[i].func = hash_random<HashT,KeyT> ;
	    hashorders[i].extra_arg = 3 ;
	    break ;
	 case Op_RANDOM_LOWREMOVE:
	    hashorders[i].func = hash_random<HashT,KeyT> ;
	    hashorders[i].extra_arg = 1 ;
	    break ;
	 case Op_RANDOM_HIGHREMOVE:
	    hashorders[i].func = hash_random<HashT,KeyT> ;
	    hashorders[i].extra_arg = 7 ;
	    break ;
	 case Op_RANDOM_NOREMOVE:
	    hashorders[i].func = hash_random<HashT,KeyT> ;
	    break ;
	 case Op_RANDOM_ADDONLY:
	    hashorders[i].func = hash_random_add<HashT,KeyT> ;
	    break ;
	 case Op_THROUGHPUT:
	    hashorders[i].cycles = 1 ;
	    hashorders[i].lookup_frac = (size_t)overhead ; // re-using parm to set fraction of lookups
	    hashorders[i].func = hash_throughput<HashT,KeyT> ;
	    break;
	 default:
	    SystemMessage::missed_case("hash_test") ;
	 }
      tpool->dispatch(&hash_dispatch<HashT>,&hashorders[i],nullptr) ;
      }
   if (!terse)
      out << "  Waiting for thread completion" << endl ;
#ifndef FrSINGLE_THREADED
   if (op == Op_THROUGHPUT && threads > 0)
      {
      overhead = 0 ;			// we re-used this parm to set the fraction of lookups
      std::this_thread::sleep_for(std::chrono::seconds(cycles)) ;
      stop_run = true ;
      }
#endif /* !FrSINGLE_THREADED */
   tpool->waitUntilIdle() ;
   if (op == Op_THROUGHPUT && threads == 0)
      {
      sigaction(SIGALRM,&oldsig,nullptr) ;
      }
   if (op == Op_NONE)
      {
      if (!user_pool)
	 delete tpool ;
      return ;
      }
   double walltime_noreclaim = timer.elapsedSeconds() ;
   if (walltime_noreclaim > overhead) walltime_noreclaim -= overhead ;
   if (ht && op == Op_REMOVE)
      {
      reclaim_deletions(ht,tpool,hashorders,slices) ;
      }
   double time = timer.cpuSeconds() ;
   double walltime = timer.elapsedSeconds() ;
   if (walltime > overhead) walltime -= overhead ;
   size_t ops = cycles * maxsize ;
   if (op == Op_RANDOM || op == Op_RANDOM_LOWREMOVE || op == Op_RANDOM_HIGHREMOVE ||
       op == Op_RANDOM_NOREMOVE || op == Op_THROUGHPUT)
      {
      // sum up the per-thread counts of operations performed
      ops = 0 ;
      for (size_t i = 0 ; i < slices ; ++i)
	 {
	 ops += hashorders[i].total_ops ;
	 }
      }
   if (time <= 0.0) time = 0.00001 ;
   if (walltime <= 0.0) walltime = 0.00001 ;
   if (terse)
      out << (size_t)(ops / walltime) << endl ;
   else
      {
      out << "  Time: " << timer << ", " ;
      pretty_print((size_t)(ops / walltime),out) ;
      out << " ops/sec" << endl ;
      }
   if (!terse && op == Op_REMOVE)
      {
      out << "  RwTm: " << walltime_noreclaim << "s without reclamation (" ;
      pretty_print((size_t)(ops / walltime_noreclaim),out) ;
      out << " ops/sec)" << endl ;
      }
   // verify success
   size_t size = ht ? ht->currentSize() : 0 ;
   size_t count = ht ? ht->countItems() : 0 ;
   size_t deleted = ht ? ht->countDeletedItems() : 0 ;
   if (size != count)
      {
      out << "'size' and 'count' disagree!  " << size << " vs " << count << endl ;
      }
   if (op == Op_ADD)
      {
      if (size > maxsize)
	 out << "   " << (size-maxsize) <<  " spurious additions to hash table!" << endl ;
      else if (size < maxsize)
	 out << "   Failed to add " << (maxsize-size) << " items to hash table!" << endl ;
      }
   if (op == Op_RANDOM)
      {
      if (deleted > 0)
	 {
	 reclaim_deletions(ht,tpool,hashorders,slices) ;
	 out << "   Pending deletions: " << deleted << " marked for deletion, "
	     << (ht ? ht->countDeletedItems() : 0) << " after reclamation"
	     << endl ;
	 }
      }
   if (op == Op_REMOVE)
      {
      if (size != 0)
	 out << "   Hash table was not emptied!  " << size << " items remain (activeitems="
	     << count << ")." << endl ;
      }
   if (!user_pool)
      delete tpool ;
   if (!ht)
      return ;
#ifdef FrHASHTABLE_STATS
   size_t stat_ins = ht->numberOfInsertions() ;
   size_t stat_ins_dup = ht->numberOfDupInsertions() ;
   size_t stat_ins_att = ht->numberOfInsertionAttempts() ;
   size_t stat_ins_forw = ht->numberOfForwardedInsertions() ;
   size_t stat_ins_resize = ht->numberOfResizeInsertions() ;
   size_t stat_cont = ht->numberOfContainsCalls() ;
   size_t stat_cont_succ = ht->numberOfSuccessfulContains() ;
   size_t stat_cont_forw = ht->numberOfForwardedContains() ;
   size_t stat_lookup = ht->numberOfLookups() ;
   size_t stat_lookup_succ = ht->numberOfSuccessfulLookups() ;
   size_t stat_lookup_forw = ht->numberOfForwardedLookups() ;
   size_t stat_rem = ht->numberOfRemovals() ;
   size_t stat_rem_count = ht->numberOfItemsRemoved() ;
   size_t stat_rem_forw = ht->numberOfForwardedRemovals() ;
   size_t stat_resize = ht->numberOfResizes() ;
   size_t stat_resize_assist = ht->numberOfResizeAssists() ;
   size_t stat_wait = ht->numberOfResizeWaits() ;
   size_t stat_reclam = ht->numberOfReclamations() ;
   size_t stat_full = ht->numberOfFullNeighborhoods() ;
   size_t retries = (stat_ins_att >= stat_ins - stat_ins_dup) ? stat_ins_att - (stat_ins - stat_ins_dup) : 0 ;
   if (!terse && (stat_ins || stat_cont || stat_lookup || stat_rem))
      {
      out << "  Stat: " << stat_ins << "+" << stat_ins_forw << " ins (" 
	  << stat_ins_dup << " dup, " << retries << " retry, " << stat_ins_resize << " resz), "
	  << stat_cont_succ << '/' << stat_cont << '+' << stat_cont_forw << " cont, "
	  << stat_lookup_succ << '/' << stat_lookup << '+' << stat_lookup_forw << " look, "
	  << stat_rem_count << '/' << stat_rem << '+' << stat_rem_forw << " rem"
	  << endl ;
      }
   if (!terse && (stat_resize || stat_resize_assist || stat_wait || stat_full || stat_reclam))
      {
      out << "  Admn: " << stat_resize << " resizes (" << stat_resize_assist << " assists, "
	  << stat_wait << " waits), " << stat_full << " congest, "
	  << stat_reclam << " reclam" << endl ;
      }
#ifndef FrSINGLE_THREADED
   size_t stat_spin = ht->numberOfSpins() ;
   size_t stat_yield = ht->numberOfYields() ;
   size_t stat_sleep = ht->numberOfSleeps() ;
   size_t stat_CAS = ht->numberOfCASCollisions() ;
   size_t stat_resize_cleanup = ht->numberOfResizeCleanups() ;
   if (!terse && (stat_spin || stat_yield || stat_sleep || stat_CAS || stat_resize_cleanup))
      {
      out << "  Thrd: " << stat_spin << " spins, " << stat_yield << " yields, " << stat_sleep << " sleeps, "
	  << stat_CAS << " CAS, " << stat_resize_cleanup << " resize cleanups" << endl ;
      }
#endif /* !FrSINGLE_THREADED */
#endif /* FrHASHTABLE_STATS */
   return  ;
}

//----------------------------------------------------------------------

static void print_stats(ostream &out, size_t* values, size_t max_value, bool nonzero_only)
{
   size_t first = 0 ;
   for (size_t i = 0 ; i <= max_value && values[i] == 0 ; i++)
      {
      first = i ;
      }
   if (first > 0)
      {
      ++first ;
      out << ' ' << first << "*0" ;
      }
   for (size_t i = first ; i <= max_value ; ++i)
      {
      out << ' ' << setw(9) << values[i] ;
      }
   size_t total = 0 ;
   uint64_t weighted = 0 ;
   first = nonzero_only ? 1 : 0 ;
   for (size_t i = first ; i <= max_value ; ++i)
      {
      total += values[i] ;
      weighted += (i * values[i]) ;
      }
   out << "\t(avg " << setprecision(4) << (weighted / (double)total) << ")" << endl ;
   return ;
}

//----------------------------------------------------------------------

static void print_chain_lengths(ostream &out, size_t which, size_t* chains, size_t max_chain)
{
   if (max_chain > 0 && chains)
      {
      out << "Chain lengths " << which << ": " ;
      print_stats(out,chains,max_chain,true) ;
      }
   return ;
}

//----------------------------------------------------------------------

static void print_neighborhoods(ostream &out, size_t which, size_t* neighborhoods, size_t max_neighbors)
{
   if (max_neighbors > 0 && neighborhoods)
      {
      out << "Densities " << which << ": " ;
      print_stats(out,neighborhoods,max_neighbors,false) ;
      }
   return ;
}

//----------------------------------------------------------------------

template <typename KeyT>
static void swap_segments(KeyT* keys, size_t numkeys, size_t threads)
{
   // because we set up the 'keys' array with the first half containing
   //   keys that are in the hash table and the second half with keys
   //   NOT in the table, if we didn't swap around entries, half of the
   //   threads would run noticeably faster than the other half, yielding
   //   suboptimal parallelism
   // fix that by swapping half of the keys for each of the first half
   //   of the threads with the corresponding keys from the second
   //   half; undo by repeating the process after the test run
   if (threads == 0)
      return ;
   size_t seg_size = numkeys / threads ;
   for (size_t segment = 0 ; segment < threads/2 ; ++segment)
      {
      size_t seg1_start = segment * seg_size ;
      size_t seg2_start = (threads - 1 - segment) * seg_size ;
      for (size_t i = 0 ; i < seg_size ; i += 2)
	 {
	 if (seg2_start + i >= numkeys)
	    continue ;
	 KeyT tmp = keys[seg1_start + i] ;
	 keys[seg1_start + i] = keys[seg2_start + i] ;
	 keys[seg2_start + i] = tmp ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT>
static void lost_chains(ostream& out, HashT* ht, size_t* chains, size_t max_chain)
{
   (void)out; (void)ht; (void)chains; (void)max_chain;
#if defined(SHOW_LOST_CHAINS)
   if (chains && max_chain > 0)
      {
      // try to display the lost deletions
      out << "lost keys:" << endl ;
      size_t count = 0 ;
      for (const auto entry : *ht)
	 {
	 ++count ;
	 out << " #" << count << ": " << entry.first << endl ;
	 }
      out << "(end of list)" << endl ;
      }
#endif /* SHOW_LOST_CHAINS */
   return ;
}

#ifdef TEST_HOPSCOTCH
template <>
void lost_chains(ostream& out, HopscotchMap* ht, size_t* chains, size_t max_chain)
{
   (void)out; (void)ht; (void)chains; (void)max_chain;
   return ;
}
#endif /* TEST_HOPSCOTCH */

//----------------------------------------------------------------------
      
template<>
void lost_chains(ostream& out, STLset* ht, size_t* chains, size_t max_chain)
{
   (void)out; (void)ht; (void)chains; (void)max_chain;
#if defined(SHOW_LOST_CHAINS)
   if (chains && max_chain > 0)
      {
      // try to display the lost deletions
      out << "lost keys:" << endl ;
      size_t count = 0 ;
      for (const auto entry : *ht)
	 {
	 ++count ;
	 out << " #" << count << ": " << entry << endl ;
	 }
      out << "(end of list)" << endl ;
      }
#endif /* SHOW_LOST_CHAINS */
   return ;
}

//----------------------------------------------------------------------
      
template <class HashT, typename KeyT>
static void run_tests(size_t threads, size_t writethreads, size_t startsize, size_t maxsize,
		      size_t cycles, KeyT* keys, uint32_t* randnums, ostream &out,
   		      bool terse, int throughput = -1, int timelimit = 4)
{
   if_SHOW_CHAINS(size_t* chains[7]);
   if_SHOW_CHAINS(size_t max_chain[7]) ;
   size_t* neighborhoods[5] ;
   size_t max_neighbors[5] ;

   Ptr<HashT> ht { HashT::create(startsize) } ;
   ThreadPool tpool(threads) ;
   if (!terse)
      out << "Checking overhead (NOP) " << endl ;
   Timer timer  ;
   hash_test(&tpool,out,"",threads,1,&ht,maxsize,keys,Op_NONE,true) ;
   double overhead = timer.elapsedSeconds() ;
   if (!terse)
      out << "   overhead = " << 1000.0*overhead << "ms" << endl ;
   hash_test(&tpool,out,"Filling hash table",writethreads,1,&ht,maxsize,keys,Op_ADD,terse,overhead) ;
   if_SHOW_CHAINS(chains[0] = ht->chainLengths(max_chain[0]));
   neighborhoods[0] = show_neighbors ? ht->neighborhoodDensities(max_neighbors[0]) : nullptr ;
   neighborhoods[1] = nullptr ;  // keep compiler happy
   hash_test(&tpool,out,"Lookups (100% present)",threads,cycles,&ht,maxsize,keys,Op_CHECK,terse,overhead) ;
   size_t half_cycles = (cycles + 1) / 2 ;
   swap_segments(keys,2*maxsize,threads) ;
   hash_test(&tpool,out,"Lookups (50% present)",threads,half_cycles,&ht,2*maxsize,keys,Op_CHECK,terse,overhead,false) ;
   swap_segments(keys,2*maxsize,threads) ;
   hash_test(&tpool,out,"Lookups (0% present)",threads,cycles,&ht,maxsize,keys+maxsize,Op_CHECKMISS,terse,overhead) ;
   if (throughput >= 0)
      {
      hash_test(&tpool,out,"Timed throughput test (10%)",threads,timelimit,&ht,maxsize,keys,Op_THROUGHPUT,terse,10,false,randnums) ;
      hash_test(&tpool,out,"Timed throughput test (30%)",threads,timelimit,&ht,maxsize,keys,Op_THROUGHPUT,terse,30,false,randnums) ;
      hash_test(&tpool,out,"Timed throughput test (50%)",threads,timelimit,&ht,maxsize,keys,Op_THROUGHPUT,terse,50,false,randnums) ;
      hash_test(&tpool,out,"Timed throughput test (70%)",threads,timelimit,&ht,maxsize,keys,Op_THROUGHPUT,terse,70,false,randnums) ;
      hash_test(&tpool,out,"Timed throughput test (90%)",threads,timelimit,&ht,maxsize,keys,Op_THROUGHPUT,terse,90,false,randnums) ;
      if (throughput != 10 && throughput != 30 && throughput != 50 && throughput != 70 && throughput != 90)
	 {
	 CharPtr heading { Fr::aprintf("Timed throughput test (%d%%)",throughput) } ;
	 hash_test(&tpool,out,*heading,threads,timelimit,&ht,maxsize,keys,Op_THROUGHPUT,terse,throughput,false,randnums) ;
	 }
      for (size_t i = 0 ; i < maxsize ; i++)
	 {
	 (void)ht->remove(keys[maxsize+i]) ;
	 }
      }
   hash_test(&tpool,out,"Emptying hash table",writethreads,1,&ht,maxsize,keys,Op_REMOVE,terse,overhead,throughput < 0) ;
   if (throughput < 0)
      {
      hash_test(&tpool,out,"Lookups in empty table",threads,cycles,&ht,maxsize,keys,Op_CHECKMISS,terse,overhead) ;
      ht = HashT::create(startsize) ;
      hash_test(&tpool,out,"Random additions",writethreads,half_cycles,&ht,maxsize,keys,Op_RANDOM_ADDONLY,terse,overhead,true,
	 randnums) ;
      if_SHOW_CHAINS(chains[1] = ht->chainLengths(max_chain[1])) ;
      hash_test(&tpool,out,"Emptying hash table",writethreads,1,&ht,maxsize,keys,Op_REMOVE,terse,overhead,false) ;
      ht = HashT::create(startsize) ;
      hash_test(&tpool,out,"Random ops (del=1)",writethreads,half_cycles,&ht,maxsize,keys,Op_RANDOM_LOWREMOVE,terse,overhead,true,
	 randnums + maxsize/2 - 1) ;
      if_SHOW_CHAINS(chains[2] = ht->chainLengths(max_chain[2])) ;
      hash_test(&tpool,out,"Random ops (del=1,full)",writethreads,half_cycles,&ht,maxsize,keys,Op_RANDOM_LOWREMOVE,terse,overhead,true,
	 	randnums + maxsize/2 - 1) ;
      if_SHOW_CHAINS(chains[3] = ht->chainLengths(max_chain[3])) ;
      hash_test(&tpool,out,"Emptying hash table",writethreads,1,&ht,maxsize,keys,Op_REMOVE,terse,overhead,false) ;
      ht = HashT::create(startsize) ;
      hash_test(&tpool,out,"Random ops (del=3)",writethreads,cycles,&ht,maxsize,keys,Op_RANDOM,terse,overhead,true,
	 randnums + maxsize - 1) ;
      if_SHOW_CHAINS(chains[4] = ht->chainLengths(max_chain[4])) ;
      neighborhoods[1] = show_neighbors ? ht->neighborhoodDensities(max_neighbors[1]) : nullptr ;
      hash_test(&tpool,out,"Emptying hash table",writethreads,1,&ht,maxsize,keys,Op_REMOVE,terse,overhead,false) ;
      ht = HashT::create(startsize) ;
      hash_test(&tpool,out,"Random ops (del=7)",writethreads,cycles,&ht,maxsize,keys,Op_RANDOM,terse,overhead,true,
	 randnums + maxsize - 1) ;
      if_SHOW_CHAINS(chains[5] = ht->chainLengths(max_chain[5])) ;
      neighborhoods[1] = show_neighbors ? ht->neighborhoodDensities(max_neighbors[1]) : nullptr ;
      hash_test(&tpool,out,"Emptying hash table",writethreads,1,&ht,maxsize,keys,Op_REMOVE,terse,overhead,false) ;
      if_SHOW_CHAINS(chains[6] = ht->chainLengths(max_chain[6])) ;
      }
#ifdef SHOW_CHAINS
   if (!terse && throughput < 0)
      {
      for (size_t i = 0 ; i < 7 ; i++)
	 {
	 print_chain_lengths(out,i,chains[i],max_chain[i]) ;
	 delete[] chains[i] ;
	 }
      lost_chains<HashT>(out,&ht,chains[5],max_chain[5]) ;
      }
#endif /* SHOW_CHAINS */
   if (!terse && show_neighbors && throughput < 0)
      {
      for (size_t i = 0 ; i < 2 ; i++)
	 {
	 print_neighborhoods(out,i,neighborhoods[i],max_neighbors[i]) ;
	 delete[] neighborhoods[i] ;
	 }
      }
   return  ;
}

//----------------------------------------------------------------------

void hash_command(ostream &out, int threads, bool terse, uint32_t* randnums,
		  size_t startsize, size_t maxsize, size_t cycles, int throughput)
{
   if (!terse)
      out << "Parallel (threaded) Object Hash Table operations\n" << endl ;
   LocalAlloc<Symbol*> keys(2*maxsize) ;
   // speed up symbol creation and avoid memory fragmentation by
   //  expanding the symbol table to hold all the symbols we will
   //  create
   size_t needed = (size_t)(2.5*maxsize) ;
   ScopedObject<SymbolTable> symtab(needed) ;
   symtab->select() ;
   hash_test(nullptr,out,"Preparing symbols",threads,1,(ObjHashTable*)nullptr,2*maxsize,&keys,Op_GENSYM,terse) ;
   hash_test(nullptr,out,"Checking symbols",threads,1,(ObjHashTable*)nullptr,2*maxsize,&keys,Op_CHECKSYMS,terse) ;
   run_tests<ObjHashTable>(threads,threads,startsize,maxsize,cycles,&keys,randnums,out,terse,throughput,time_limit) ;
   return ;
}

//----------------------------------------------------------------------

static INTEGER_TYPE* gen_keys(ostream& out, size_t maxsize, size_t order, size_t stride, bool terse)
{
   auto keys = new INTEGER_TYPE[2*maxsize] ;
   if (order == 0)
      {
      if (!terse)
	 out << "Generating sequential keys" << endl ;
      for (size_t i = 0 ; i < 2*maxsize ; i++)
	 keys[i] = (uint32_t)i ;
      }
   else if (order == 1 || order == 2)
      {
      // generate a randomly-distributed set of 32-bit integers, less the reserved values
      if (!terse)
	 out << "Generating random keys" << endl ;
#if 0
      RandomInteger rand(0xFFFFFFFE) ;
      if (order == 2)
	 rand.seed(31415926) ;
      for (size_t i = 0 ; i < 2*maxsize ; ++i)
	 {
	 keys[i] = rand() ;
	 }
#else
      // generate a randomly-distributed set of 31-bit integers
      //   (Park-Miller Lehmer RNG, aka MINSTD)
      uint32_t seed = 1 ;
      if (order == 1)
	 {
	 std::chrono::system_clock::time_point now = std::chrono::system_clock::now() ;
	 seed = (uint32_t)(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count()) ;
	 }
      for (size_t i = 0 ; i < 2*maxsize ; i++)
	 {
	 keys[i] = seed ;
	 do {
	 seed = ((uint64_t)seed * 48271UL) % INT32_MAX ;
	 // skip the special reserved values, just in case we hit them
	 } while (seed >= (uint32_t)~1) ;
	 }
#endif
      }
   else if (order == 3)
      {
      if (!terse)
	 out << "Generating keys spaced by " << stride << endl ;
      size_t val = 0 ;
      size_t adj = 1 ;
      for (size_t i = 0 ; i < 2*maxsize ; i++)
	 {
	 keys[i] = val ;
	 val += stride ;
	 if (val > UINT32_MAX)
	    {
	    // each time we wrap around 2**32, shift by one
	    //   additional position so that we eventually fill the
	    //   entire space
	    val = adj++ ;
	    }
	 }
      }
   else if (order == 4)
      {
      if (!terse)
	 out << "Generating keys by applying FastHash64 to sequence" << endl ;
      for (size_t i = 0 ; i < 2*maxsize ; i++)
	 {
	 keys[i] = FramepaC::fasthash64_int(i) ;
	 }
      }
   return keys ;
}

//----------------------------------------------------------------------

void stlset_command(ostream &out, int threads, bool terse, uint32_t* randnums, size_t startsize,
		    size_t maxsize, size_t cycles, size_t order, size_t stride, int throughput)
{
   if (!terse)
      out << "Parallel (threaded) STL integer unordered_set operations\n" << endl ;
   LocalAlloc<INTEGER_TYPE,1> keys(gen_keys(out,maxsize,order,stride,terse)) ;
   run_tests<STLset>(threads,STL_WRITE_THREADS,startsize,maxsize,cycles,&keys,randnums,out,terse,throughput,time_limit) ;
   return ;
}

//----------------------------------------------------------------------

#ifdef TEST_HOPSCOTCH
void hopscotch_command(ostream &out, int threads, bool terse, uint32_t* randnums, size_t startsize,
		       size_t maxsize, size_t cycles, size_t order, size_t stride, int throughput)
{
   if (!terse)
      out << "Parallel Hopscotch integer map operations (concurrency=" << g_Hopscotch_Concurrency << ")\n"
	  << endl ;
   LocalAlloc<INTEGER_TYPE,1> keys(gen_keys(out,maxsize,order,stride,terse)) ;
   run_tests<HopscotchMap>(threads,threads,startsize,maxsize,cycles,&keys,randnums,out,terse,throughput,time_limit) ;
   return ;
}
#endif /* TEST_HOPSCOTCH */

//----------------------------------------------------------------------

void ihash_command(ostream &out, int threads, bool terse, uint32_t* randnums, size_t startsize,
   		   size_t maxsize, size_t cycles, size_t order, size_t stride, int throughput)
{
   if (!terse)
      out << "Parallel (threaded) Integer Hash Table operations\n" << endl ;
   LocalAlloc<INTEGER_TYPE,1> keys(gen_keys(out,maxsize,order,stride,terse)) ;
   run_tests<HashSet_U32>(threads,threads,startsize,maxsize,cycles,&keys,randnums,out,terse,throughput,time_limit) ;
   return ;
}

/************************************************************************/
/************************************************************************/

int main(int argc, char** argv)
{
//   const char* operation ;
   bool use_int_hashtable { false } ;
   bool use_STL_unorderedset { false } ;
   bool use_hopscotch { false } ;
   int throughput { -1 } ;
   int threads { 0 } ;
   size_t start_size { 8000000 } ;   // as in Herlihy et al
   size_t grow_size { 6000000 } ;    // 75% load factor
   size_t repetitions { 1 } ;
   size_t stride { 2 } ;
   int key_order { 1 } ;
   bool terse = false ;

   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(use_int_hashtable,"i","int","use integer-keyed hash table instead of Object-keyed")
      .add(use_STL_unorderedset,"I","stl","use STL unordered_set with integer keys, not FramepaC hashtable")
#ifdef TEST_HOPSCOTCH
      .add(use_hopscotch,"H","hopscotch","use Herlihy et al (2008) Hopscotch hash map with integer keys")
      .add(g_Hopscotch_Concurrency,"C","concur","set concurrency of Hopscotch hash map",8,16384)
#endif /* TEST_HOPSCOTCH */
//      .add(operation,"f","function","")
      .add(grow_size,"g","","")
      .add(threads,"j","threads","")
      .add(key_order,"k","keys","key order: 0=seq, 1=random, 2=fixrandom, 3=stride, 4=fasthash64",0,4)
      .add(show_neighbors,"N","densities","show densities within neighborhoods of hash buckets")
      .add(repetitions,"r","reps","number of repetitions to run")
      .add(start_size,"s","initsize","initial size of hash table")
      .add(stride,"S","stride","")
      .add(throughput,"T","throughput","do a timed throughput test with N% loookups",0,100)
      .add(time_limit,"","time","set time limit for time throughput tests",1,60)
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
#ifdef FrSINGLE_THREADED
   threads = 0 ;
#endif
   if (threads <= 0)
      {
      terse = true ;
      threads = -threads ;
      }
   if (grow_size == 0)
      {
      cout << "Entered test size of zero, skipping test" << endl ;
      }
   else
      {
      if (!terse)
	 cout << "Generating random numbers for randomized tests" << endl ;
      LocalAlloc<uint32_t> randnums(2*grow_size) ;
      RandomInteger rand(grow_size) ;
      for (size_t i = 0 ; i < 2*grow_size ; i++)
	 {
	 randnums[i] = rand() ;
	 }
      if (use_STL_unorderedset)
	 {
	 stlset_command(cout,threads,terse,&randnums,start_size,grow_size,repetitions,key_order,stride,throughput) ;
	 }
      else if (use_hopscotch)
	 {
#ifdef TEST_HOPSCOTCH
	 hopscotch_command(cout,threads,terse,&randnums,start_size,grow_size,repetitions,key_order,stride,throughput) ;
#endif /* TEST_HOPSCOTCH */
	 }
      else if (use_int_hashtable)
	 ihash_command(cout,threads,terse,&randnums,start_size,grow_size,repetitions,key_order,stride,throughput) ;
      else
	 hash_command(cout,threads,terse,&randnums,start_size,grow_size,repetitions,throughput) ;
      }
   return 0 ;
}

// end of file parhash.C //
