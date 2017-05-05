/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-05					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2015,2017 Carnegie Mellon University			*/
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
#include <sstream>
#include "framepac/argparser.h"
#include "framepac/hashtable.h"
#include "framepac/message.h"
#include "framepac/random.h"
#include "framepac/symbol.h"
#include "framepac/texttransforms.h"
#include "framepac/threadpool.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

// uncomment the following line to show statistcs on the lengths
//   of the chains in the hash array
#define SHOW_CHAINS
//#define SHOW_LOST_CHAINS

// uncomment the following line to show the population densities within
//   the search window around each location in the hash array
#define SHOW_NEIGHBORHOODS

#define INTEGER_TYPE uint32_t

/************************************************************************/
/*	Type declarations						*/
/************************************************************************/

static SymbolTable* current_symtab() { return nullptr ; /* SymbolTable::current() ;*/ } //FIXME
extern Symbol* findSymbol(const char* /*name*/) ; //FIXME
static Symbol* add_symbol(SymbolTable* /*symtab*/, const char* /*name*/)
{ /* return symtab->add(name) ; */ return nullptr ; } //FIXME

enum Operation
{
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
   Op_RANDOM_ADDONLY
} ;

typedef void HashRequestFunc(class HashRequestOrder *) ;

class HashRequestOrder
   {
   public:
      Operation	  op ;
      size_t	  size ;
      size_t	  cycles ;
      size_t	  id ;
      size_t	  threads ;
      size_t	  slice_start ;
      size_t	  slice_size ;
      size_t	  current_cycle ;
      size_t	  total_ops ;
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
   char* msg = Fr::vaprintf(fmt,args) ;
   if (msg)
      {
      out << msg << flush  ;
      delete[] msg ;
      }
   return  ;
}

/************************************************************************/
/************************************************************************/

static void hash_gensym(HashRequestOrder *order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   Symbol **syms = order->syms ;
   // generate the symbols in multiple interleaved passes so that we
   //   don't end up with strictly increasing (and thus well-cached)
   //   hash keys during insertion
   SymbolTable *symtab = current_symtab() ;
   size_t passes = 53 ;
   for (size_t pass = 0 ; pass < passes ; ++pass)
      {
      for (size_t i = order->slice_start + pass ; i < slice_end ; i += passes)
	 {
	 // generate our own unique symbol to avoid contention in gensym()
	 char name[100] ;
	 snprintf(name,sizeof(name),"%c%lu%c",(char)('A' + (order->id % 26)),i,'\0') ;
	 syms[i] = add_symbol(symtab,name) ;
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
   //FIXME: generating empty output!
   ostringstream ss ;
   ss << ";  Job " << id << " encountered " << what << " " << obj << endl << flush ;
   const char* str = ss.str().c_str() ;
   cerr << str << flush ;
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_add(HashRequestOrder *order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   HashT *ht = (HashT*)order->ht ;
   KeyT *syms  = (KeyT*)order->syms ;
   for (size_t i = order->slice_start ; i < slice_end ; ++i)
      {
      if (ht->add(syms[i]))
	 {
	 err_msg("a symbol already in the table:",order->id,syms[i]) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_check(HashRequestOrder *order)
{
   my_job_id = order->id ;
   bool missing = (bool)order->extra_arg ;
   size_t slice_end = order->slice_start + order->slice_size ;
   HashT *ht = (HashT*)order->ht ;
   KeyT *syms  = (KeyT*)order->syms ;
   if (missing)
      {
      for (size_t i = order->slice_start ; i < slice_end ; ++i)
	 {
	 if (ht->contains(syms[i]))
	    {
	    err_msg("spurious symbol",order->id,syms[i]) ;
	    }
	 }
      }
   else
      {
      for (size_t i = order->slice_start ; i < slice_end ; ++i)
	 {
	 if (!ht->contains(syms[i]) && order->strict)
	    {
	    err_msg("missing symbol",order->id,syms[i]) ;
	    }
	 }
      }
   if (order->m_verbose)
      {
      char* msg = aprintf(";  Job %lu cycle %lu complete.\n",order->id,order->current_cycle) ;
      cout << msg << flush ;
      delete[] msg ;
      }
   return ;
}

//----------------------------------------------------------------------

static bool find_Symbol(const Symbol *sym)
{
//FIXME   return sym ? findSymbol(sym->symbolName()) != nullptr : false;
   (void)sym;
   return false ;
}

static bool find_Symbol(INTEGER_TYPE) { return false ; }

//static const char *sym_name(const Symbol *sym) { return sym->symbolName() ; }
static const char *sym_name(const Symbol * /*sym*/) { return nullptr ; } //FIXME

static const char *sym_name(INTEGER_TYPE) { return "" ; }

template <class HashT, typename KeyT>
static void hash_checksyms(HashRequestOrder *order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   KeyT *syms  = (KeyT*)order->syms ;
   for (size_t i = order->slice_start ; i < slice_end ; ++i)
      {
      if (!find_Symbol(syms[i]) && order->strict)
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
static void hash_remove(HashRequestOrder *order)
{
   my_job_id = order->id ;
   size_t slice_end = order->slice_start + order->slice_size ;
   HashT *ht = (HashT*)order->ht ;
   KeyT *syms  = (KeyT*)order->syms ;
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
static void hash_random(HashRequestOrder *order)
{
   my_job_id = order->id ;
   unsigned remove_frac = order->extra_arg ;
   HashT *ht = (HashT*)order->ht ;
   KeyT *syms = (KeyT*)order->syms ;
   uint32_t *randnums = order->randnums + order->slice_start ;
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
static void hash_random_add(HashRequestOrder *order)
{
   my_job_id = order->id ;
   HashT *ht = (HashT*)order->ht ;
   KeyT *syms = (KeyT*)order->syms ;
   uint32_t *randnums = order->randnums + order->slice_start ;
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

template <class HashT>
static void hash_dispatch(const void *input, void * /*output*/ )
{
   HashRequestOrder *order = (HashRequestOrder*)input ;
   my_job_id = order->id ;
   order->current_cycle = 1 ;
   HashT::registerThread() ;  // should not be needed once HashTable is completely fixed
   HashT::clearPerThreadStats() ;
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
      ((HashT*)order->ht)->updateGlobalStats() ;
   return ;
}

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void hash_test(ThreadPool *user_pool, ostream &out, size_t threads, size_t cycles, HashT *ht,
		      size_t maxsize, KeyT *syms, enum Operation op, bool terse, bool strict = true,
		      uint32_t *randnums = nullptr)
{
   ThreadPool *tpool = user_pool ? user_pool : new ThreadPool(threads) ;
   bool must_wait = (threads != 0) ;
   if (threads == 0) threads = 1 ;
   HashRequestOrder *hashorders = new HashRequestOrder[threads] ;
   //out << "  Dispatching threads" << endl ;
   size_t slice_size = (maxsize + threads/2) / threads ;
   if (ht)
      {
      ht->clearGlobalStats() ;
      ht->clearPerThreadStats() ;
      }
   Timer timer ;
   for (size_t i = 0 ; i < threads ; ++i)
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
      hashorders[i].threads = threads ;
      hashorders[i].pool = tpool ;
      hashorders[i].slice_start = i * slice_size ;
      hashorders[i].slice_size = (i+1 < threads) ? slice_size : (maxsize - hashorders[i].slice_start) ;
      hashorders[i].extra_arg = 0 ;
      hashorders[i].total_ops = 0 ;
      switch (op)
	 {
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
	 default:
	    SystemMessage::missed_case("hash_test") ;
	 }
      tpool->dispatch(&hash_dispatch<HashT>,&hashorders[i],nullptr) ;
      }
   if (must_wait)
      {
      if (!terse)
	 out << "  Waiting for thread completion" << endl ;
      tpool->waitUntilIdle() ;
      }
   double walltime_noreclaim = timer.elapsedSeconds() ;
   if (ht && op == Op_REMOVE)
      {
      ht->reclaimDeletions() ;
      ht->updateGlobalStats() ;
      }
   double time = timer.cpuSeconds() ;
   double walltime = timer.elapsedSeconds() ;
   if (!user_pool)
      delete tpool ;
   size_t ops = cycles * maxsize ;
   if (op == Op_RANDOM || op == Op_RANDOM_LOWREMOVE || op == Op_RANDOM_HIGHREMOVE ||
       op == Op_RANDOM_NOREMOVE)
      {
      // sum up the per-thread counts of operations performed
      ops = 0 ;
      for (size_t i = 0 ; i < threads ; ++i)
	 {
	 ops += hashorders[i].total_ops ;
	 }
      }
   delete[] hashorders ;
   if (time <= 0.0) time = 0.00001 ;
   if (walltime <= 0.0) walltime = 0.00001 ;
   out << "  Time: " << timer << ", " ;
   pretty_print((size_t)(ops / walltime),out) ;
   out << " ops/sec" << endl ;
   if (op == Op_REMOVE)
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
	 out << "   " << (size-maxsize) <<  "spurious additions to hash table!" << endl ;
      else if (size< maxsize)
	 out << "   Failed to add " << (maxsize-size) << " items to hash table!" << endl ;
      }
   if (op == Op_REMOVE || op == Op_RANDOM)
      {
      if (deleted > 0)
	 {
	 if (ht) ht->reclaimDeletions() ;
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
   size_t stat_reclam = ht->numberOfReclamations() ;
   size_t stat_moves = ht->numberOfEntriesMoved() ;
   size_t stat_full = ht->numberOfFullNeighborhoods() ;
   size_t stat_chain = ht->numberOfChainLocks() ;
   size_t stat_chain_coll = ht->numberOfChainLockCollisions() ;
   size_t retries = (stat_ins_att >= stat_ins - stat_ins_dup) ? stat_ins_att - (stat_ins - stat_ins_dup) : 0 ;
   out << "  Stat: " << (stat_ins-stat_ins_forw) << "+" << stat_ins_forw << " ins (" 
       << stat_ins_dup << " dup, " << retries << " retry, " << stat_ins_resize << " resz), "
       << stat_cont_succ << '/' << stat_cont << '+' << stat_cont_forw << " cont, "
       << stat_lookup_succ << '/' << stat_lookup << '+' << stat_lookup_forw << " look, "
       << stat_rem_count << '/' << stat_rem << '+' << stat_rem_forw << " rem"
       << endl ;
   out << "  Admn: " << stat_resize << " resizes (" << stat_resize_assist << " assists), " << stat_full << " congest, "
       << stat_reclam << " reclam, " << stat_moves << " moves, "
       << stat_chain_coll << '/' << stat_chain << " chainlock" << endl ;
#ifdef FrMULTITHREAD
   size_t stat_spin = ht->numberOfSpins() ;
   size_t stat_yield = ht->numberOfYields() ;
   size_t stat_sleep = ht->numberOfSleeps() ;
   size_t stat_CAS = ht->numberOfCASCollisions() ;
   size_t stat_resize_cleanup = ht->numberOfResizeCleanups() ;
   out << "  Thrd: " << stat_spin << " spins, " << stat_yield << " yields, " << stat_sleep << " sleeps, "
       << stat_CAS << " CAS, " << stat_resize_cleanup << " resize cleanups" << endl ;
#endif /* FrMULTITHREAD */
#endif /* FrHASHTABLE_STATS */
   return  ;
}

//----------------------------------------------------------------------

static void print_stats(ostream &out, size_t *values, size_t max_value,	bool nonzero_only)
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

static void print_chain_lengths(ostream &out, size_t which, size_t *chains, size_t max_chain)
{
   if (max_chain > 0 && chains)
      {
      out << "Chain lengths " << which << ": " ;
      print_stats(out,chains,max_chain,true) ;
      }
   return ;
}

//----------------------------------------------------------------------

static void print_neighborhoods(ostream &out, size_t which, size_t *neighborhoods, size_t max_neighbors)
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
static void swap_segments(KeyT *keys, size_t numkeys, size_t threads)
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

#if defined(SHOW_LOST_CHAINS)
template <typename KeyT, typename ValT>
static bool show_lost_keys(KeyT key, ValT, va_list args)
{
   FrVarArg(ostream*,out) ;
   FrVarArg(size_t*,count) ;
   ++(*count) ; 
   (*out) << " #" << *count << ": " << key << endl ;
   return true ;
}
#endif /* SHOW_CHAINS */

//----------------------------------------------------------------------

template <class HashT, typename KeyT>
static void run_tests(size_t threads, size_t startsize, size_t maxsize,
		      size_t cycles, KeyT *keys, uint32_t *randnums, ostream &out,
		      bool terse)
{
   if_SHOW_CHAINS(size_t *chains[6]);
   if_SHOW_CHAINS(size_t max_chain[6]) ;
   if_SHOW_NEIGHBORS(size_t *neighborhoods[5]) ;
   if_SHOW_NEIGHBORS(size_t max_neighbors[5]) ;

   HashT *ht = new HashT(startsize) ;
   ThreadPool tpool(threads) ;
   out << "Filling hash table      " << endl ;
   hash_test(&tpool,out,threads,1,ht,maxsize,keys,Op_ADD,terse) ;
   if_SHOW_CHAINS(chains[0] = ht->chainLengths(max_chain[0]));
   if_SHOW_NEIGHBORS(neighborhoods[0] = ht->neighborhoodDensities(max_neighbors[0])) ;
   out << "Lookups (100% present)  " << endl ;
   hash_test(&tpool,out,threads,cycles,ht,maxsize,keys,Op_CHECK,terse) ;
   out << "Lookups (50% present)   " << endl ;
   size_t half_cycles = (cycles + 1) / 2 ;
   swap_segments(keys,2*maxsize,threads) ;
   hash_test(&tpool,out,threads,half_cycles,ht,2*maxsize,keys,Op_CHECK,terse,false) ;
   swap_segments(keys,2*maxsize,threads) ;
   out << "Lookups (0% present)    " << endl ;
   hash_test(&tpool,out,threads,cycles,ht,maxsize,keys+maxsize,Op_CHECKMISS,terse) ;
   out << "Emptying hash table     " << endl ;
   hash_test(&tpool,out,threads,1,ht,maxsize,keys,Op_REMOVE,terse) ;
   out << "Lookups in empty table  " << endl ;
   hash_test(&tpool,out,threads,cycles,ht,maxsize,keys,Op_CHECKMISS,terse) ;
   delete ht ;
   ht = new HashT(startsize) ;
   out << "Random additions        " << endl ;
   hash_test(&tpool,out,threads,half_cycles,ht,maxsize,keys,Op_RANDOM_ADDONLY,terse,true,
	     randnums) ;
   if_SHOW_CHAINS(chains[1] = ht->chainLengths(max_chain[1])) ;
   out << "Emptying hash table     " << endl ;
   hash_test(&tpool,out,threads,1,ht,maxsize,keys,Op_REMOVE,terse,false) ;
   delete ht ;
   ht = new HashT(startsize) ;
   out << "Random ops (del=1)      " << endl ;
   hash_test(&tpool,out,threads,half_cycles,ht,maxsize,keys,Op_RANDOM_LOWREMOVE,terse,true,
	     randnums + maxsize/2 - 1) ;
   if_SHOW_CHAINS(chains[2] = ht->chainLengths(max_chain[2])) ;
   out << "Random ops (del=1,full) " << endl ;
   hash_test(&tpool,out,threads,half_cycles,ht,maxsize,keys,Op_RANDOM_LOWREMOVE,terse,true,
	     randnums + maxsize/2 - 1) ;
   if_SHOW_CHAINS(chains[2] = ht->chainLengths(max_chain[2])) ;
   out << "Emptying hash table     " << endl ;
   hash_test(&tpool,out,threads,1,ht,maxsize,keys,Op_REMOVE,terse,false) ;
   delete ht ;
   ht = new HashT(startsize) ;
   out << "Random ops (del=3)      " << endl ;
   hash_test(&tpool,out,threads,cycles,ht,maxsize,keys,Op_RANDOM,terse,true,
	     randnums + maxsize - 1) ;
   if_SHOW_CHAINS(chains[3] = ht->chainLengths(max_chain[3])) ;
   if_SHOW_NEIGHBORS(neighborhoods[1] = ht->neighborhoodDensities(max_neighbors[1])) ;
   out << "Emptying hash table     " << endl ;
   hash_test(&tpool,out,threads,1,ht,maxsize,keys,Op_REMOVE,terse,false) ;
   delete ht  ;
   ht = new HashT(startsize) ;
   out << "Random ops (del=7)      " << endl ;
   hash_test(&tpool,out,threads,cycles,ht,maxsize,keys,Op_RANDOM,terse,true,
	     randnums + maxsize - 1) ;
   if_SHOW_CHAINS(chains[4] = ht->chainLengths(max_chain[4])) ;
   if_SHOW_NEIGHBORS(neighborhoods[1] = ht->neighborhoodDensities(max_neighbors[1])) ;
   out << "Emptying hash table     " << endl ;
   hash_test(&tpool,out,threads,1,ht,maxsize,keys,Op_REMOVE,terse,false) ;
   if_SHOW_CHAINS(chains[5] = ht->chainLengths(max_chain[5])) ;
#ifdef SHOW_CHAINS
   for (size_t i = 0 ; i < 6 ; i++)
      {
      print_chain_lengths(out,i,chains[i],max_chain[i]) ;
      delete [] chains[i] ;
      }
#if defined(SHOW_LOST_CHAINS)
   if (chains[5] && max_chain[5] > 0)
      {
      // try to display the lost deletions
      out << "lost keys:" << endl ;
      size_t count = 0 ;
      ht->iterate(show_lost_keys,&out,&count) ;
      out << "(end of list)" << endl ;
      }
#endif /* SHOW_LOST_CHAINS */
#endif /* SHOW_CHAINS */
   delete ht ;
#ifdef SHOW_NEIGHBORHOODS
   for (size_t i = 0 ; i < 2 ; i++)
      {
      print_neighborhoods(out,i,neighborhoods[i],max_neighbors[i]) ;
      delete [] neighborhoods[i] ;
      }
#endif /* SHOW_NEIGHBORHOODS */
   return  ;
}

//----------------------------------------------------------------------

void hash_command(ostream &out, int threads, bool terse, uint32_t* randnums,
		  size_t startsize, size_t maxsize, size_t cycles)
{
   out << "Parallel (threaded) Object Hash Table operations" << endl << endl ;
   Symbol **keys = new Symbol*[2*maxsize] ;
   out << "Preparing symbols       " << endl ;
   // speed up symbol creation and avoid memory fragmentation by
   //  expanding the symbol table to hold all the symbols we will
   //  create
//FIXME   SymbolTable* curr_symtab = SymbolTable::current() ;
//FIXME   size_t needed = curr_symtab->sizeForCapacity(2*maxsize) ;
   size_t needed = 2*maxsize ;//FIXME
   needed *= 1.33 ; // gensyms tend to cluster in the table, since they are so similar in name
//FIXME   curr_symtab->expandTo(needed+1000) ;
   hash_test(nullptr,out,threads,1,(ObjHashTable*)nullptr,2*maxsize,keys,Op_GENSYM,terse) ;
   out << "Checking symbols        " << endl ;
   hash_test(nullptr,out,threads,1,(ObjHashTable*)nullptr,2*maxsize,keys,Op_CHECKSYMS,terse) ;
   run_tests<ObjHashTable>(threads,startsize,maxsize,cycles,keys,randnums,out,terse) ;
   delete[] keys ;
   return ;
}

//----------------------------------------------------------------------

void ihash_command(ostream &out, int threads, bool terse, uint32_t* randnums, size_t startsize,
		   size_t maxsize, size_t cycles, size_t order, size_t stride)
{
   out << "Parallel (threaded) Integer Hash Table operations" << endl << endl ;
   INTEGER_TYPE *keys = new INTEGER_TYPE[2*maxsize] ;
   if (order == 0)
      {
      out << "Generating sequential keys" << endl ;
      for (size_t i = 0 ; i < 2*maxsize ; i++)
	 keys[i] = (uint32_t)i ;
      }
   else if (order == 1 || order == 2)
      {
      // generate a randomly-distributed set of 32-bit integers, less the reserved values
      out << "Generating random keys" << endl ;
#if 0
      RandomInteger rand(0xFFFFFFFC) ;
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
      for (size_t i = 0 ; i < 2*maxsize ; i++)
	 {
	 keys[i] = seed ;
	 do {
	 seed = ((uint64_t)seed * 48271UL) % INT32_MAX ;
	 // skip the special reserved values, just in case we hit them
	 } while (seed >= (uint32_t)~2) ;
	 }
#endif
      }
   else if (order == 3)
      {
      out << "\nGenerating keys spaced by " << stride << endl ;
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
   run_tests<HashSet_U32>(threads,startsize,maxsize,cycles,keys,randnums,out,terse) ;
   delete[] keys ;
   return ;
}

/************************************************************************/
/************************************************************************/

int main(int argc, char** argv)
{
   const char* operation ;
   bool use_int_hashtable ;
   int threads { 0 } ;
   size_t start_size { 1 } ;
   size_t grow_size ;
   size_t repetitions { 1 } ;
   size_t stride { 2 } ;
   int key_order { 0 } ;
   bool terse = false ;

   ArgParser cmdline_flags ;
   cmdline_flags
      .add(use_int_hashtable,"i","int","use integer-keyed hash table instead of Object-keyed")
      .add(operation,"f","function","")
      .add(grow_size,"g","","")
      .add(threads,"j","threads","")
      .add(key_order,"k","keys","key order: 0=seq, 1=random, 2=fixrandom, 3=stride",0,3)
      .add(repetitions,"r","reps","number of repetitions to run")
      .add(start_size,"s","initsize","initial size of hash table")
      .add(stride,"S","stride","")
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
      cout << "Generating random numbers for randomized tests" << endl ;
      uint32_t *randnums = new uint32_t[2*grow_size] ;
      RandomInteger rand(grow_size) ;
      for (size_t i = 0 ; i < 2*grow_size ; i++)
	 {
	 randnums[i] = rand() ;
	 }
      if (use_int_hashtable)
	 ihash_command(cout,threads,terse,randnums,start_size,grow_size,repetitions,key_order,stride) ;
      else
	 hash_command(cout,threads,terse,randnums,start_size,grow_size,repetitions) ;
      delete[] randnums ;
      }
   return 0 ;
}

// end of file parhash.C //
