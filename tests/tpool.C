/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-12					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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

#include <iostream>
#include "framepac/argparser.h"
#include "framepac/random.h"
#include "framepac/threadpool.h"
#include "framepac/timer.h"

using namespace std ;
using namespace Fr ;

/************************************************************************/
/************************************************************************/

static void null_work(const void*, void*)
{
   return ;
}

void (*null_function)(const void*,void*) = &null_work ;

//----------------------------------------------------------------------------

#ifndef FrSINGLE_THREADED
static void variable_work(const void* data, void* returnval)
{
   size_t count = *((size_t*)data) ;
   size_t value = 1234 ;
   for (size_t i = 0 ; i < count ; ++i)
      {
      value ^= (value << 2) ;
      value ^= (value >> 1) ;
      }
   *((size_t*)returnval) = value ;
   return ;
}
#endif /* !FrSINGLE_THREADED */

//----------------------------------------------------------------------------

#ifndef FrSINGLE_THREADED
static void variable_sleep(const void* data, void*)
{
   size_t delay = *((size_t*)data) ;
   this_thread::sleep_for(chrono::milliseconds(delay)) ;
   return ;
}
#endif /* !FrSINGLE_THREADED */

//----------------------------------------------------------------------------

#ifndef FrSINGLE_THREADED
static void run_null(size_t numthreads, size_t task_count, bool batch_mode)
{
   ThreadPool tp(numthreads) ;
   Timer timer1 ;
   for (size_t i = 0 ; i < task_count ; ++i)
      {
      null_function(nullptr,nullptr) ;
      }
   cout << "Single-threaded: " << timer1 << endl ;
   Timer timer2 ;
   if (batch_mode)
      {
      cout << "Running in batch mode" << endl ;
      tp.dispatchBatch<char*,char*>(null_function,task_count,nullptr,nullptr) ;
      }
   else
      {
      for (size_t i = 0 ; i < task_count ; ++i)
	 {
	 tp.dispatch(null_function,nullptr,nullptr) ;
	 }
      }
   tp.waitUntilIdle() ;
   cout << "Thread pool: " << timer2 << endl ;
   return ;
}
#endif /* !FrSINGLE_THREADED */

//----------------------------------------------------------------------------

#ifndef FrSINGLE_THREADED
static void run_work(size_t numthreads, size_t task_count, size_t max_count, bool batch_mode)
{
   ThreadPool tp(numthreads) ;
   size_t* jobsizes = New<size_t>(task_count) ;
   size_t dummy ;
   RandomInteger rand(max_count) ;
   for (size_t i = 0 ; i < task_count ; ++i)
      {
      jobsizes[i] = rand.get() ;
      }
   Timer timer1 ;
   for (size_t i = 0 ; i < task_count ; ++i)
      {
      variable_work(&jobsizes[i],&dummy) ;
      }
   cout << "Single-threaded: " << timer1 << endl ;
   Timer timer2 ;
   if (batch_mode)
      {
      cout << "Running in batch mode" << endl ;
      tp.dispatchBatch(variable_work,task_count,jobsizes,jobsizes) ;
      }
   else
      {
      for (size_t i = 0 ; i < task_count ; ++i)
	 {
	 tp.dispatch(variable_work,&jobsizes[i],&dummy) ;
	 }
      }
   tp.waitUntilIdle() ;
   cout << "Thread pool: " << timer2 << endl ;
   return ;
}
#endif /* !FrSINGLE_THREADED */

//----------------------------------------------------------------------------

#ifndef FrSINGLE_THREADED
static void run_sleep(size_t numthreads, size_t task_count, size_t max_sleep, bool batch_mode)
{
   ThreadPool tp(numthreads) ;
   size_t* jobsizes = New<size_t>(task_count) ;
   size_t dummy ;
   RandomInteger rand(max_sleep) ;
   for (size_t i = 0 ; i < task_count ; ++i)
      {
      jobsizes[i] = rand.get() ;
      }
   Timer timer1 ;
   for (size_t i = 0 ; i < task_count ; ++i)
      {
      variable_sleep(&jobsizes[i],&dummy) ;
      }
   cout << "Single-threaded: " << timer1 << endl ;
   Timer timer2 ;
   if (batch_mode)
      {
      cout << "Running in batch mode" << endl ;
      tp.dispatchBatch(variable_sleep,task_count,jobsizes,jobsizes) ;
      }
   else
      {
      for (size_t i = 0 ; i < task_count ; ++i)
	 {
	 tp.dispatch(variable_sleep,&jobsizes[i],&dummy) ;
	 }
      }
   tp.waitUntilIdle() ;
   cout << "Thread pool: " << timer2 << endl ;
   return ;
}
#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

int main(int argc, char** argv)
{
   size_t numthreads { 0 } ;
   size_t task_count { 100 } ;
   size_t max_value { 0 } ;
   bool do_sleep { false } ;
   bool batch_mode { false } ;
   
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(batch_mode,"b","batch","")
      .add(max_value,"c","count","")
      .add(numthreads,"j","threads","")
      .add(task_count,"n","numreps","")
      .add(do_sleep,"s","sleep","")
      .addHelp("h","help","show this usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
#ifdef FrSINGLE_THREADED
   cerr << "Compiled without thread support.  Terminating...." << endl ;
#else
   if (max_value)
      {
      if (do_sleep)
	 {
	 run_sleep(numthreads,task_count,max_value,batch_mode) ;
	 }
      else
	 {
	 run_work(numthreads,task_count,max_value,batch_mode) ;
	 }
      }
   else
      {
      run_null(numthreads,task_count,batch_mode) ;
      }
#endif /* FrSINGLE_THREADED */
   return 0 ;
}

// end of file tpool.C //
