/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
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

#include <thread>
#include "framepac/atomic.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

// size of request queue for each worker thread in pool
// workers can steal from another's queue if theirs is empty
#define FrWORKQUEUE_SIZE 256

/************************************************************************/
/************************************************************************/

typedef void ThreadPoolWorkFunc(const void *input, void *output) ;

class WorkOrder
   {
   public:
      WorkOrder() ;
      WorkOrder(ThreadPoolWorkFunc* fn, const void* in, void* out)
	 : m_func(fn), m_input(in), m_output(out), m_inprogress(false), m_complete(false) {}
      WorkOrder(const WorkOrder&) ;
      ~WorkOrder() ;
      WorkOrder& operator= (const WorkOrder&) ;

      ThreadPoolWorkFunc* worker() const { return m_func ; }
      const void* input() const { return m_input ; }
      void* output() const { return m_output ; }

   private:
      ThreadPoolWorkFunc* m_func ;	// function to call
      const void* m_input ;
      void*       m_output ;
      atomic_bool m_inprogress ;
      atomic_bool m_complete ;
   } ;

/************************************************************************/
/************************************************************************/

class WorkQueue
   {
   public:
      WorkQueue() ;
      WorkQueue(const WorkQueue&) ;
      ~WorkQueue() ;
      WorkQueue& operator= (const WorkQueue&) ;

   private:
      WorkOrder* m_orders[FrWORKQUEUE_SIZE] ;
      unsigned   m_start ;
      unsigned   m_head ;
      unsigned   m_tail ;
   } ;

/************************************************************************/
/************************************************************************/

class ThreadPool
   {
   public:
      ThreadPool(unsigned num_threads) ;
      ThreadPool(const ThreadPool&) ;
      ~ThreadPool() ;
      ThreadPool& operator= (const ThreadPool&) ;

      // accessors
      unsigned numThreads() const { return m_numthreads ; }
      unsigned availThreads() const { return m_availthreads ; }
      unsigned inactiveThreads() const { return numThreads() - availThreads() ; }
      unsigned idleThreads() const ;

      // manipulators
      bool limitThreads(unsigned N) ;
      bool dispatch(WorkOrder* order) ;
      bool dispatch(ThreadPoolWorkFunc* fn, void* in_out)
	 { return dispatch(fn, in_out, in_out) ; }
      bool dispatch(ThreadPoolWorkFunc* fn, const void* input, void* output) ;

      // status
      bool idle() const { return idleThreads() >= availThreads() ; }

      // synchronization
      void waitUntilIdle() ;

      // functions called by worker threads
      WorkOrder* nextOrder(unsigned index) ;
      void threadExiting(unsigned index) ;

   private:
      unsigned   m_numthreads ;		// total number of worker threads
      unsigned   m_availthreads ;	// number of threads allowed to work
#ifndef FrSINGLE_THREADED
      unsigned	 m_next_thread ;	// next thread to which to try to assign a request
      thread**   m_pool ;		// the actual thread objects
      WorkQueue* m_queues ;		// work queues, one per worker thread

#endif /* !FrSINGLE_THREADED */
   } ;


} // end namespace Fr

// end of file threadpool.h //
