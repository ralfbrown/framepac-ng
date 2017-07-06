/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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
#include "framepac/argparser.h"
#include "framepac/memory.h"
#include "framepac/threadpool.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

#define ALLOC_SIZE 16

/************************************************************************/
/*	Global variables						*/
/************************************************************************/

// how many blocks to (deliberately) leak on each iteration of the
//   memory-allocation benchmark.  This is used to test the
//   interaction of FramepaC's memory allocator with memory-checker
//   programs like Valgrind.
//static size_t leakage_per_iteration = 0 ;

static size_t alloc_size = ALLOC_SIZE ;

/************************************************************************/
/************************************************************************/

struct BatchInfo
   {
      SmallAlloc* allocator ;
      void**      blocks ;
      size_t      batch_size ;
      size_t      iterations ;
      bool	  done ;
      bool        do_reclaim  ;
   } ;

/************************************************************************/
/************************************************************************/

static void show_test_time(Timer& timer, size_t size, size_t repetitions, bool complete)
{
   double seconds = timer.elapsedSeconds() ;
   cout << " ...time was " ;
   timer.showTimes(cout) ;
   if (repetitions > 1)
      {
      double microsec_per_iter = seconds / repetitions * 1000000.0 ;
      cout << ", " ;
      if (microsec_per_iter > 1000)
	 cout << setprecision(5) << microsec_per_iter / 1000 << "ms" ;
      else if (microsec_per_iter >= 0.5)
	 cout << setprecision(6) << microsec_per_iter << "us" ;
      else
	 cout << setprecision(6) << 1000*microsec_per_iter << "ns" ;
      cout << "/iter" ;
      if (size > 0)
	 {
	 cout << " (" ;
	 if (microsec_per_iter / size < 1.0)
	    cout << setprecision(4) << (1000.0*microsec_per_iter/size) << "ns" ;
	 else
	    cout << setprecision(4) << (microsec_per_iter/size) << "us" ;
	 cout << "/alloc)" ;
	 }
      else
	 cout << "ation" ;
      }
   cout << "." << endl ;
   if (complete)
      {
      cout << "\nThis benchmark is now complete.\n" << endl ;
      }
   return ;
}

//----------------------------------------------------------------------------

void run_suballocator(const void* input, void* /*output*/)
{
   const BatchInfo* info = reinterpret_cast<const BatchInfo*>(input) ;
   size_t size = info->batch_size ;
   SmallAlloc* allocator = info->allocator ;
   bool do_reclaim = info->do_reclaim ;
   LocalAlloc<void*,30000> blocks(size) ;
   Timer timer ;
   for (size_t pass = 0 ; pass < info->iterations ; ++pass)
      {
      for (size_t i = 0 ; i < size ; ++i)
	 blocks[i] = allocator->allocate() ;
      for (size_t i = 0 ; i < size ; ++i)
	 allocator->release(blocks[i]) ;
      if (do_reclaim) allocator->reclaim() ;
      }
   if (!do_reclaim) allocator->reclaim() ;
   show_test_time(timer,size,info->iterations,info->done) ;
   return ;
}

//----------------------------------------------------------------------------

void run_malloc(const void* input, void* /*output*/)
{
   const BatchInfo* info = reinterpret_cast<const BatchInfo*>(input) ;
   size_t size = info->batch_size ;
   LocalAlloc<void*,30000> blocks(size) ;
   Timer timer ;
   for (size_t pass = 0 ; pass < info->iterations ; ++pass)
      {
      for (size_t i = 0 ; i < size ; ++i)
	 blocks[i] = ::malloc(alloc_size) ;
      for (size_t i = 0 ; i < size ; ++i)
	 ::free(blocks[i]) ;
      }
   show_test_time(timer,size,info->iterations,true) ;
   return ;
}

//----------------------------------------------------------------------------

void run_new(const void* input, void* /*output*/)
{
   const BatchInfo* info = reinterpret_cast<const BatchInfo*>(input) ;
   size_t size = info->batch_size ;
   LocalAlloc<char*,30000> blocks(size) ;
   Timer timer ;
   for (size_t pass = 0 ; pass < info->iterations ; ++pass)
      {
      for (size_t i = 0 ; i < size ; ++i)
	 blocks[i] = ::new char[alloc_size] ;
      for (size_t i = 0 ; i < size ; ++i)
	 ::delete blocks[i] ;
      }
   show_test_time(timer,size,info->iterations,true) ;
   return ;
}

//----------------------------------------------------------------------------

void benchmark(size_t size, size_t iterations, bool do_reclaim, ThreadPoolWorkFunc *fn, const char* what)
{
   BatchInfo info ;
   cout << "Benchmark of memory allocation speed (using " << what << ")\n\n"
           "We allocate and then release " << size << " " << alloc_size << "-byte objects a total\n"
           "of " << iterations << " times.\n"
	<< endl ;
   info.allocator = SmallAlloc::create(alloc_size) ;
   info.batch_size = size ;
   info.iterations = iterations ;
   info.done = false ;
   info.do_reclaim = do_reclaim ;
   fn(&info,nullptr) ;
   if (fn != run_suballocator)
      return ;
   cout << "\nWe allocate and then release " << size << " " << alloc_size << "-byte objects a total\n"
           "of " << iterations << " times, but first fragment allocations so that\n"
           "the allocator works entirely from thread-local slabs.\n"
	<< endl ;
   LocalAlloc<void*> dummy(1+size/100) ;
   {
   // allocate 1.01 times as many blocks as requested
   LocalAlloc<void*,30000> blocks(size) ;
   dummy[(1+size/100)-1] = nullptr ;
   for (size_t i = 0 ; i < size ; ++i)
      {
      blocks[i] = info.allocator->allocate() ;
      if (i % 100 == 0)
	 dummy[i/100] = info.allocator->allocate() ;
      }
   // free all of the blocks except every hundredth one, which will
   //   prevent the allocator from returning the memory to the
   //   operating system
   for (size_t i = 0 ; i < size ; ++i)
      {
      info.allocator->release(blocks[i]) ;
      }
   }
   info.done = true ;
   fn(&info,nullptr) ;
   for (size_t i = 0 ; i < 1+size/100 ; ++i)
      {
      info.allocator->release(dummy[i]) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void run_suballocator_batch(const void* input, void* /*output*/)
{
   const BatchInfo* info = reinterpret_cast<const BatchInfo*>(input) ;
   void** blocks = info->blocks ;
   size_t batch_size = info->batch_size ;
   SmallAlloc* allocator = info->allocator ;
   for (size_t pass = 0 ; pass < info->iterations ; ++pass)
      {
      for (size_t i = 0 ; i < batch_size ; ++i)
	 blocks[i] = allocator->allocate() ;
      for (size_t i = 0 ; i < batch_size ; ++i)
	 allocator->release(blocks[i]) ;
      if (info->do_reclaim) allocator->reclaim() ;
      }
   return ;
}

//----------------------------------------------------------------------------

void run_malloc_batch(const void* input, void* /*output*/)
{
   const BatchInfo* info = reinterpret_cast<const BatchInfo*>(input) ;
   void** blocks = info->blocks ;
   size_t batch_size = info->batch_size ;
   for (size_t pass = 0 ; pass < info->iterations ; ++pass)
      {
      for (size_t i = 0 ; i < batch_size ; ++i)
	 blocks[i] = ::malloc(alloc_size) ;
      for (size_t i = 0 ; i < batch_size ; ++i)
	 ::free(blocks[i]) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void run_new_batch(const void* input, void* /*output*/)
{
   const BatchInfo* info = reinterpret_cast<const BatchInfo*>(input) ;
   char** blocks = (char**)info->blocks ;
   size_t batch_size = info->batch_size ;
   for (size_t pass = 0 ; pass < info->iterations ; ++pass)
      {
      for (size_t i = 0 ; i < batch_size ; ++i)
	 blocks[i] = ::new char[alloc_size] ;
      for (size_t i = 0 ; i < batch_size ; ++i)
	 ::delete blocks[i] ;
      }
   return ;
}

//----------------------------------------------------------------------------

void benchmark_parallel(size_t threads, size_t size, size_t iterations, bool do_reclaim,
   ThreadPoolWorkFunc* fn, const char* what)
{
   ThreadPool tpool(threads) ;
   if (threads == 0) threads = 1 ;
//   size_t batch_count = threads >= 4 ? 2 * threads : 4 ;
   size_t batch_count = threads >= 1 ? threads : 1 ;
   size_t batch_size = (size + batch_count - 1) / batch_count ;
   size = batch_count * batch_size ;
   cout << "Benchmark of allocation speed (multiple threads using " << what << ")\n\n"
           "We allocate and then release " << batch_size << " " << alloc_size << "-byte objects in each\n"
           "of " << batch_count << " batches running in " << threads << " concurrent threads\n"
           "a total of " << iterations  << " times.\n"
	<< endl ;
   LocalAlloc<void*,30000> blocks(size) ;
   SmallAlloc* allocator = SmallAlloc::create(alloc_size) ;
   Timer timer ;
   LocalAlloc<BatchInfo> batch_info(batch_count) ;
   for (size_t batch = 0 ; batch < batch_count ; ++batch)
      {
      BatchInfo& info = batch_info[batch] ;
      info.allocator = allocator ;
      info.blocks = &blocks[batch*batch_size] ;
      info.batch_size = batch_size ;
      info.iterations = iterations ;
      info.do_reclaim = do_reclaim ;
      tpool.dispatch(fn,&info) ;
      }
   tpool.waitUntilIdle() ;
   allocator->reclaim() ;
   show_test_time(timer,size,iterations,true) ;
   return ;
}

/************************************************************************/
/************************************************************************/

int main(int argc, char** argv)
{
   size_t size { 100000 } ;
   size_t repetitions { 1 } ;
   size_t threads { 0 } ;  // run single-threaded by default
   bool use_malloc { false } ;
   bool use_new { false } ;
   bool do_reclaim { false } ;
   
   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(alloc_size,"a","alloc","number of bytes per allocation")
      .add(threads,"j","threads","")
      .add(use_malloc,"m","use-malloc","use malloc() instead of custom allocator")
      .add(use_new,"n","use-new","use 'new char[]' instead of custom allocator")
      .add(repetitions,"r","reps","number of repetitions to run")
      .add(do_reclaim,"R","reclaim","run memory reclamation after every iteration")
      .add(size,"s","size","number of allocations per iteration")
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
#ifdef FrSINGLE_THREADED
   threads = 0 ;
#endif
   if (repetitions < 1)
      repetitions = 1 ;
   if (threads == 0)
      {
      if (use_malloc)
	 {
	 benchmark(size,repetitions,false,run_malloc,"malloc") ;
	 }
      else if (use_new)
	 {
	 benchmark(size,repetitions,false,run_new,"new") ;
	 }
      else
	 {
	 benchmark(size,repetitions,do_reclaim,run_suballocator,"suballocator") ;
	 }
      }
   else if (use_malloc)
      {
      benchmark_parallel(threads,size,repetitions,false,run_malloc_batch,"malloc") ;
      }
   else if (use_new)
      {
      benchmark_parallel(threads,size,repetitions,false,run_new_batch,"new") ;
      }
   else
      {
      benchmark_parallel(threads,size,repetitions,do_reclaim,run_suballocator_batch,"suballocator") ;
      }
   return 0 ;
}

// end of file membench.C //
