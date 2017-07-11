/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-10					*/
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
#include "framepac/string.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

static void run_serial(size_t num_strings, size_t repetitions)
{
   LocalAlloc<String*> strings(num_strings) ;
   char numbuf[100] ;

   cout << "Generating strings with numbers from 0 to " << num_strings << "-1, repeated "
	<< repetitions << " times." << endl ;
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
	 strings[i] = String::create(numbuf) ;
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

int main(int argc, char** argv)
{
   size_t repetitions { 1 } ;
   size_t num_strings { 1000000 } ;
   size_t threads { 0 } ;
   
   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(num_strings,"n","","number of strings to generate")
//      .add(threads,"j","threads","number of parallel threads to use")
      .add(repetitions,"r","reps","number of repetitions to run")
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
#ifdef FrSINGLE_THREADED
   threads = 0 ;
#endif
   if (threads == 0)
      {
      run_serial(num_strings,repetitions) ;
      }
   else
      {
      //run_parallel(threads,num_strings,repetitions) ;
      }
   return 0 ;
}
