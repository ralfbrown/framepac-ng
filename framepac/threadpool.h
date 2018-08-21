/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-20					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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

#ifndef _Fr_THREADPOOL_H_INCLUDED
#define _Fr_THREADPOOL_H_INCLUDED

#include <stdarg.h>
#include <thread>
#include "framepac/atomic.h"
#include "framepac/critsect.h"
#include "framepac/semaphore.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

/************************************************************************/
/************************************************************************/

typedef void ThreadPoolWorkFunc(const void *input, void *output) ;
typedef bool ThreadPoolMapFunc(size_t index, va_list args) ;

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
      ThreadPool(const ThreadPool&) = delete ;
      ~ThreadPool() ;
      ThreadPool& operator= (const ThreadPool&) = delete ;

      // accessors
      unsigned numThreads() const { return m_numthreads ; }
      unsigned idleThreads() const ;  //TODO

      static ThreadPool* defaultPool() ;

      // manipulators
      bool dispatch(ThreadPoolWorkFunc* fn, const void* input, void* output) ;
      bool dispatch(ThreadPoolWorkFunc* fn, void* in_out)
	 { return dispatch(fn, in_out, in_out) ; }

      // limitation: no other threads are allowed to dispatch jobs until dispatchBatch() returns; that
      //   especially includes worker functions dispatching additional work
      template <typename InT, typename OutT>
      bool dispatchBatch(ThreadPoolWorkFunc* fn, size_t count, const InT* input, OutT* output)
	 { return dispatchBatch(fn,count,sizeof(InT),input,sizeof(OutT),output) ; }
      template <typename InOutT>
      bool dispatchBatch(ThreadPoolWorkFunc* fn, size_t count, InOutT* in_out)
	 { return dispatchBatch(fn,count,sizeof(InOutT),in_out,sizeof(InOutT),in_out) ; }

      // simplified interface for map/reduce applications
      //   we use void* and va_list to avoid bloating the object code; the worker function needs to
      //   cast appropriately
      bool parallelize(ThreadPoolMapFunc* fn, size_t num_items, va_list args) ;
      bool parallelize(ThreadPoolMapFunc* fn, size_t num_items, ...)
	 {
	    va_list args ;
	    va_start(args,num_items) ;
	    bool status = parallelize(fn,num_items,args) ;
	    va_end(args) ;
	    return status ;
	 }

      static void defaultPool(ThreadPool*) ;
      static void defaultPool(size_t numthreads) { defaultPool(new ThreadPool(numthreads)) ; }

      // status
      bool idle() const ;  //TODO

      // synchronization
      void waitUntilIdle() ;

      // functions called by worker threads
      WorkOrder* nextOrder(unsigned index) ;
      WorkOrder* makeWorkOrder(ThreadPoolWorkFunc* fn, const void* in, void* out) ;
      void recycle(WorkOrder*) ;
      void threadExiting(unsigned index) ;
      void ack(unsigned index) ;

   protected:
      void allocateWorkOrders() ;
      void discardRecycledOrders() ;
      bool dispatchBatch(ThreadPoolWorkFunc* fn, size_t count, size_t insize, const void* input,
			 size_t outsize, void* output) ;

   private:
      static ThreadPool* s_defaultpool ;
      unsigned   m_numthreads ;		// total number of worker threads
      unsigned   m_numCPUs { 0 } ;	// hardware threads, used to limit work-stealing scan
      unsigned	 m_prev_thread { 0 } ;	// next thread to which to try to assign a request
      WorkQueue* m_queues { nullptr } ;	// work queues, one per worker thread
      WorkBatch* m_batches { nullptr } ;
      WorkOrder* m_freeorders { nullptr } ;
      CriticalSection m_flguard ;	// critical section for guarding the work-order freelist
      Semaphore  m_ack { 0 } ;
#ifndef FrSINGLE_THREADED
      thread**   m_pool { nullptr } ;	// the actual thread objects
#endif /* !FrSINGLE_THREADED */
   } ;

} // end namespace Fr

#endif /* !_Fr_THREADPOOL_H_INCLUDED */

// end of file threadpool.h //
