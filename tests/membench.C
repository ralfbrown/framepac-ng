/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-21					*/
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
#include "framepac/nonobject.h"
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
   Timer timer ;
   //FIXME: use nonObject allocator of size 16 instead
   FramepaC::Object_VMT<NonObject> nonobj_vmt ;
   Allocator<NonObject> allocator(&nonobj_vmt) ;
   for (size_t pass = 0 ; pass < iterations ; ++pass)
      {
      for (size_t i = 0 ; i < size ; ++i)
	 blocks[i] = allocator.allocate() ;
      for (size_t i = 0 ; i < size ; ++i)
	 allocator.release(blocks[i]) ;
      }
   show_test_time(timer,size,iterations,true) ;
   return ;
}

//----------------------------------------------------------------------------

void benchmark_suballocator_parallel(size_t threads, size_t size, size_t iterations)
{
   cout << "Benchmark of memory sub-allocator speed (multiple threads)\n\n"
           "We allocate and then release " << size << " 16-byte objects a total\n"
           "of " << iterations << " times using " << threads << " concurrent threads.\n"
	<< endl ;
   cerr << "threaded benchmark not implemented yet" << endl ;
   ThreadPool tpool(threads) ;
//FIXME
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
