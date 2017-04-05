/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-04					*/
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
#include "framepac/critsect.h"
#include "framepac/semaphore.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

// size of request queue for each worker thread in pool; should be power of 2
// workers can steal from another's queue if theirs is empty
#define FrWORKQUEUE_SIZE 512

/************************************************************************/
/************************************************************************/

typedef void ThreadPoolWorkFunc(const void *input, void *output) ;

// classes used internally by ThreadPool
class WorkOrder ;
class WorkBatch  ;
class WorkQueue ;

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
      bool dispatch(ThreadPoolWorkFunc* fn, const void* input, void* output) ;
      bool dispatch(ThreadPoolWorkFunc* fn, void* in_out)
	 { return dispatch(fn, in_out, in_out) ; }
      void discardRecycledOrders() ;

      // status
      bool idle() const { return idleThreads() >= availThreads() ; }

      // synchronization
      void waitUntilIdle() ;

      // functions called by worker threads
      WorkOrder* nextOrder(unsigned index) ;
      void recycle(WorkOrder*) ;
      void threadExiting(unsigned index) ;

   protected:
      WorkOrder* makeWorkOrder(ThreadPoolWorkFunc* fn, const void* in, void* out) ;
      bool dispatch(WorkOrder* order) ;

   private:
      unsigned   m_numthreads ;		// total number of worker threads
      unsigned   m_availthreads ;	// number of threads allowed to work
#ifndef FrSINGLE_THREADED
      thread**   m_pool { nullptr } ;	// the actual thread objects
#endif /* !FrSINGLE_THREADED */
      unsigned	 m_next_thread { 0 } ;	// next thread to which to try to assign a request
      WorkQueue* m_queues { nullptr } ;	// work queues, one per worker thread
      WorkBatch* m_batches { nullptr } ;
      WorkOrder* m_freeorders { nullptr } ;
      CriticalSection m_flguard ;	// critical section for guarding the work-order freelist
      Semaphore  m_workavail { 0 } ;
      Semaphore  m_idle { 0 } ;
      Semaphore  m_paused { 0 } ;
      Semaphore  m_jobs { 1 } ;
   } ;


} // end namespace Fr

// end of file threadpool.h //
