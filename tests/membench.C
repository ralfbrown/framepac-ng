/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
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

/************************************************************************/
/*	Global variables						*/
/************************************************************************/

// how many blocks to (deliberately) leak on each iteration of the
//   memory-allocation benchmark.  This is used to test the
//   interaction of FramepaC's memory allocator with memory-checker
//   programs like Valgrind.
//static size_t leakage_per_iteration = 0 ;

/************************************************************************/
/************************************************************************/

static void show_test_time(Timer& timer, size_t size, size_t repetitions, bool complete)
{
   double seconds = timer.elapsedSeconds() ;
   cout << " time was " ;
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
      cout << " per iter" ;
      if (size > 0)
	 {
	 cout << " (" ;
	 if (microsec_per_iter / size < 1.0)
	    cout << setprecision(4) << (1000.0*microsec_per_iter/size) << "ns" ;
	 else
	    cout << setprecision(4) << (microsec_per_iter/size) << "us" ;
	 cout << " per alloc)" ;
	 }
      else
	 cout << "ation" ;
      }
   cout << "." << endl ;
   if (complete)
      {
      cout << "This benchmark is now complete.\n" << endl ;
      }
   return ;
}

//----------------------------------------------------------------------------

void benchmark_suballocator(size_t size, size_t iterations)
{
   cout << "Benchmark of memory sub-allocator speed\n\n"
           "We allocate and then release " << size << " 16-byte objects a total\n"
           "of " << iterations << " times.\n"
	<< endl ;
   LocalAlloc<void*,30000> blocks(size) ;
   SmallAlloc* allocator = SmallAlloc::create(16) ;
   Timer timer ;
   for (size_t pass = 0 ; pass < iterations ; ++pass)
      {
      for (size_t i = 0 ; i < size ; ++i)
	 blocks[i] = allocator->allocate() ;
      for (size_t i = 0 ; i < size ; ++i)
	 allocator->release(blocks[i]) ;
      }
   allocator->reclaim() ;
   show_test_time(timer,size,iterations,true) ;
   return ;
}

//----------------------------------------------------------------------------

struct BatchInfo
   {
      SmallAlloc* allocator ;
      void**      blocks ;
      size_t      batch_size ;
      size_t      iterations ;
   } ;

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
      }
   return ;
}

//----------------------------------------------------------------------------

void benchmark_suballocator_parallel(size_t threads, size_t size, size_t iterations)
{
   ThreadPool tpool(threads) ;
   if (threads == 0) threads = 1 ;
   size_t batch_count = threads >= 4 ? 2 * threads : 4 ;
   size_t batch_size = (size + batch_count - 1) / batch_count ;
   size = batch_count * batch_size ;
   cout << "Benchmark of memory sub-allocator speed (multiple threads)\n\n"
           "We allocate and then release " << size << " 16-byte objects a total\n"
           "of " << iterations << " times split across " << threads << " concurrent threads.\n"
	<< endl ;
   LocalAlloc<void*,30000> blocks(size) ;
   SmallAlloc* allocator = SmallAlloc::create(16) ;
   Timer timer ;
   LocalAlloc<BatchInfo> batch_info(batch_count) ;
   for (size_t batch = 0 ; batch < batch_count ; ++batch)
      {
      BatchInfo& info = batch_info[batch] ;
      info.allocator = allocator ;
      info.blocks = &blocks[batch*batch_size] ;
      info.batch_size = batch_size ;
      info.iterations = iterations ;
      tpool.dispatch(run_suballocator_batch,&info) ;
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

   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(threads,"j","threads","")
      .add(repetitions,"r","reps","number of repetitions to run")
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
      benchmark_suballocator(size,repetitions) ;
   else
      {
      benchmark_suballocator_parallel(threads,size,repetitions) ;
      }
   return 0 ;
}

// end of file membench.C //
