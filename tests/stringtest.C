/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-12					*/
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

#include "framepac/argparser.h"
#include "framepac/memory.h"
#include "framepac/symbol.h"
#include "framepac/threadpool.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

typedef String* CreateStringFunc(const char*) ;
typedef Symbol* CreateSymbolFunc(const char*) ;

struct BatchInfo
   {
   public:
      CreateStringFunc* create ;
      String**          strings ;
      size_t            start ;
      size_t            batch_size ;
      size_t            repetitions ;
   } ;

/************************************************************************/
/************************************************************************/

static void run_serial(size_t num_strings, size_t repetitions, CreateStringFunc *create)
{
   Object* obj = create(" ") ;
   const char *objtype = obj->typeName() ;
   obj->free() ;
   cout << "Generating " << objtype << "s with numbers from 0 to " << num_strings << "-1, repeated "
	<< repetitions << " times." << endl ;
   LocalAlloc<String*> strings(num_strings) ;
   char numbuf[100] ;
   Timer timer ;
   for (size_t pass = 0 ; pass < repetitions ; ++pass)
      {
      for (size_t i = 0 ; i < num_strings ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 }
      bool failed { false } ;
      for (size_t i = 0 ; i < num_strings ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 if (!strcmp(numbuf,numbuf))
	    {
	    failed = true ;
	    break ;
	    }
	 }
      if (failed) cout << flush ;
      }
   double elapsed = timer.elapsedSeconds() ;
   double CPU = timer.cpuSeconds() ;
   cout << "  Overhead: " << timer << endl ;
   timer.restart() ;
   for (size_t pass = 0 ; pass < repetitions ; ++pass)
      {
      for (size_t i = 0 ; i < num_strings ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 strings[i] = create(numbuf) ;
	 }
      bool failed { false } ;
      for (size_t i = 0 ; i < num_strings ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 if (!strings[i] || strcmp(numbuf,strings[i]->c_str()) != 0)
	    {
	    failed = true ;
	    break ;
	    }
	 strings[i]->free() ;
	 }
      if (failed) cout << "String compare failed!" << endl ;
      }
   elapsed = timer.elapsedSeconds() - elapsed ;
   CPU = timer.cpuSeconds() - CPU ;
   cout << "  Time:   " << timer << endl ;
   uint64_t total_ops = repetitions * num_strings ;
   size_t ops_per_sec = (size_t)(total_ops / elapsed + 0.5) ;
   size_t ops_per_CPU = (size_t)(total_ops / CPU + 0.5) ;
   cout << "  Net:    " << elapsed << "s elapsed, " << CPU << "s CPU, " << endl ;
   cout << "  Speed:  " << ops_per_sec << " ops/sec, " << ops_per_CPU << " ops/CPUsec" << endl ;
   return  ;
}

//----------------------------------------------------------------------------

static void run_parallel_overhead(const void* in, void* /*out*/)
{
   const BatchInfo *info = reinterpret_cast<const BatchInfo*>(in) ;
   char numbuf[100] ;
   for (size_t pass = 0 ; pass < info->repetitions ; ++pass)
      {
      for (size_t i = info->start ; i < info->start + info->batch_size ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 }
      bool failed { false } ;
      for (size_t i = info->start ; i < info->start + info->batch_size ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 if (!strcmp(numbuf,numbuf))
	    {
	    failed = true ;
	    break ;
	    }
	 }
      if (failed) cout << flush ;
      }
   return ;
}

//----------------------------------------------------------------------------

static void run_parallel_test(const void* in, void* /*out*/)
{
   const BatchInfo *info = reinterpret_cast<const BatchInfo*>(in) ;
   char numbuf[100] ;
   for (size_t pass = 0 ; pass < info->repetitions ; ++pass)
      {
      for (size_t i = info->start ; i < info->start + info->batch_size ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 info->strings[i] = info->create(numbuf) ;
	 }
      bool failed { false } ;
      for (size_t i = info->start ; i < info->start + info->batch_size ; ++i)
	 {
	 snprintf(numbuf,sizeof(numbuf),"%ld%c",i,'\0') ;
	 if (!info->strings[i] || strcmp(numbuf,info->strings[i]->c_str()) != 0)
	    {
	    failed = true ;
	    break ;
	    }
	 }
      if (failed) cout << flush ;
      }
   return ;
}

//----------------------------------------------------------------------------

static void run_parallel(size_t threads, size_t num_strings, size_t repetitions, CreateStringFunc *create)
{
   if (threads < 1) threads = 1 ;
   size_t batch_size = (num_strings + threads - 1) / threads ;
   num_strings = threads * batch_size ;
   Object* obj = create(" ") ;
   const char *objtype = obj->typeName() ;
   obj->free() ;
   cout << "Generating " << objtype << "s with numbers from 0 to " << num_strings << "-1 in " << threads
	<< " parallel batches\n"
	<< "of " << batch_size << " each, repeated " << repetitions << " times." << endl  ;
   LocalAlloc<String*> strings(num_strings) ;
   ThreadPool tpool(threads) ;
   // set up per-batch information
   LocalAlloc<BatchInfo> batch_info(threads) ;
   for (size_t i = 0 ; i < threads ; ++i)
      {
      batch_info[i].strings = strings ;
      batch_info[i].start = i * batch_size ;
      batch_info[i].batch_size = batch_size ;
      batch_info[i].create = create ;
      batch_info[i].repetitions = repetitions ;
      }
   Timer timer ;
   // run overhead check in parallel
   for (size_t i = 0 ; i < threads ; ++i)
      {
      tpool.dispatch(run_parallel_overhead,&batch_info[i]) ;
      }
   tpool.waitUntilIdle() ;
   double elapsed = timer.elapsedSeconds() ;
   double CPU = timer.cpuSeconds() ;
   cout << "  Overhead: " << timer << endl ;
   timer.restart() ;
   // run real test in parallel
   for (size_t i = 0 ; i < threads ; ++i)
      {
      tpool.dispatch(run_parallel_test,&batch_info[i]) ;
      }
   tpool.waitUntilIdle() ;
   elapsed = timer.elapsedSeconds() - elapsed ;
   CPU = timer.cpuSeconds() - CPU ;
   cout << "  Time:   " << timer << endl ;
   uint64_t total_ops = repetitions * num_strings ;
   size_t ops_per_sec = (size_t)(total_ops / elapsed + 0.5) ;
   size_t ops_per_CPU = (size_t)(total_ops / CPU + 0.5) ;
   cout << "  Net:    " << elapsed << "s elapsed, " << CPU << "s CPU, " << endl ;
   cout << "  Speed:  " << ops_per_sec << " ops/sec, " << ops_per_CPU << " ops/CPUsec" << endl ;
   return ;
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
   size_t repetitions { 1 } ;
   size_t num_strings { 1000000 } ;
   size_t threads { 0 } ;
   bool use_symbols { false } ;
   
   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(num_strings,"n","","number of strings to generate")
      .add(threads,"j","threads","number of parallel threads to use")
      .add(repetitions,"r","reps","number of repetitions to run")
      .add(use_symbols,"y","symbols","generate Symbols instead of Strings")
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
   CreateSymbolFunc* create_sym = Symbol::create ;
   CreateStringFunc* create_str = String::create ;
   CreateStringFunc* create = use_symbols ? (CreateStringFunc*)create_sym : create_str ;
#ifdef FrSINGLE_THREADED
   threads = 0 ;
#endif
   if (threads == 0)
      {
      run_serial(num_strings,repetitions,create) ;
      }
   else
      {
      run_parallel(threads,num_strings,repetitions,create) ;
      }
   return 0 ;
}
