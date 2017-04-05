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

#include <chrono>
#include <mutex>
#include "framepac/thread.h"
#include "framepac/threadpool.h"
using namespace std ;

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#define BATCH_SIZE 511

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/************************************************************************/

class WorkOrder
   {
   public:
      WorkOrder() ;
      WorkOrder(ThreadPoolWorkFunc* fn, const void* in, void* out)
	 : m_func(fn), m_input(in), m_output(out) {}
      WorkOrder(const WorkOrder&) = delete ;
      ~WorkOrder() ;
      WorkOrder& operator= (const WorkOrder&) = delete ;

      ThreadPoolWorkFunc* worker() const { return m_func ; }
      const void* input() const { return m_input ; }
      void* output() const { return m_output ; }

      // free-list management
      WorkOrder* next() const { return m_next ; }
      void next(WorkOrder* nxt) { m_next = nxt ; }

   private:
      union {
	 ThreadPoolWorkFunc* m_func ;	// function to call
	 WorkOrder*          m_next ;   // next item in free list
      } ;
      const void* m_input ;
      void*       m_output ;
      atomic_bool m_inprogress { false } ;
      atomic_bool m_complete { false } ;
   } ;

/************************************************************************/
/************************************************************************/

class WorkBatch
   {
   public:
      WorkBatch(WorkBatch* nxt = nullptr) { m_next = nxt ; }
      WorkBatch(const WorkBatch&) = delete ;
      WorkBatch& operator= (const WorkBatch&) = delete ;
      ~WorkBatch() {}

      WorkBatch* next() const { return m_next ; }
      void addToFreeList(WorkOrder*& freelist)
	 {
	    m_orders[0].next(freelist) ;
	    for (size_t i = 1 ; i < BATCH_SIZE ; i++)
	       {
	       m_orders[i].next(&m_orders[i-1]) ;
	       }
	    freelist = &m_orders[BATCH_SIZE-1] ;
	 }

   protected:
      WorkOrder  m_orders[BATCH_SIZE] ;
      WorkBatch* m_next ;
   } ;

/************************************************************************/
/************************************************************************/

class WorkQueue
   {
   public:
      WorkQueue() {}
      WorkQueue(const WorkQueue&) ;
      ~WorkQueue() ;
      WorkQueue& operator= (const WorkQueue&) ;

      bool empty() const { return m_head >= m_tail.load() ; }
      void clear() ;

      WorkOrder* pop() ;		// can only be called by queue's owner
      bool push(WorkOrder* order) ;	// can be called by any thread
      WorkOrder* steal() ;		// can be called by any thread

   private:
      size_t             m_head { 0 } ;
      Atomic<WorkOrder*> m_orders[FrWORKQUEUE_SIZE] = { nullptr } ;
      Atomic<size_t>     m_tail { 0 } ;
   } ;

/************************************************************************/
/************************************************************************/

static void work_function(ThreadPool* pool, unsigned thread_index)
{
   if (!pool)
      return ;
   for ( ; ; )
      {
      WorkOrder* order = pool->nextOrder(thread_index) ;
      ThreadPoolWorkFunc* fn = order->worker() ;
      const void* in = order->input() ;
      void* out = order->output() ;
      pool->recycle(order) ;
      if (!fn)
	 {
	 pool->threadExiting(thread_index) ;
	 return ;
	 }
      fn(in,out); 
      }
   return ;
}

/************************************************************************/
/*	Methods for class WorkQueue					*/
/************************************************************************/

WorkQueue::WorkQueue(const WorkQueue&)
{
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

WorkQueue::~WorkQueue()
{
   //FIXME
   return;
}

//----------------------------------------------------------------------------

WorkOrder* WorkQueue::pop()
{
   size_t tail = m_tail.load() ;
   while (m_head < tail)
      {
      // figure out the index of the next task
      size_t idx = (m_head++) % FrWORKQUEUE_SIZE ;
      // try to grab the next task
      WorkOrder* order = m_orders[idx].exchange(nullptr) ;
      if (order)
	 {
	 return order ;
	 }
      // someone stole the next task, so loop until we
      //   get a non-null pointer, or m_head reaches m_tail
      tail = m_tail.load() ;
      }
   return nullptr ;	// queue is empty
}

//----------------------------------------------------------------------------

WorkOrder* WorkQueue::steal()
{
   size_t tail = --m_tail ;
   // try to grab a task
   WorkOrder* order = m_orders[tail%FrWORKQUEUE_SIZE].exchange(nullptr) ;
   if (!order)
      {
      // someone else already claimed the last task on the queue (as
      //   of the time we started this call), so re-increment the tail
      //   pointer because we didn't actually steal anything
      ++m_tail ;
      }
   return order ;
}

//----------------------------------------------------------------------------

bool WorkQueue::push(WorkOrder* order)
{
   if (!order)
      return false ;			// nothing to push
   size_t tail = m_tail.load() ;
   if (tail - m_head >= FrWORKQUEUE_SIZE)
      return false ;			// queue is full
   for ( ; ; )
      {
      order = m_orders[tail % FrWORKQUEUE_SIZE].exchange(order) ;
      if (!order && m_tail.compare_exchange_weak(tail,tail+1))
	 break ;
      // if the previous value of the entry was non-null, that means someone else snuck in and
      //   added a task, so retry
      tail = m_tail.load() ;
      } while (order) ;
   return true ;
}

//----------------------------------------------------------------------------

void WorkQueue::clear()
{
//FIXME
   m_tail.store(0) ;
   m_head = 0 ;
   return ;
}

/************************************************************************/
/*	Methods for class ThreadPool					*/
/************************************************************************/

ThreadPool::ThreadPool(unsigned num_threads)
   : m_numthreads(num_threads), m_availthreads(0)
{
#ifdef FrSINGLE_THREADED
   m_numthreads = 0 ;
#else
   m_queues = new WorkQueue[num_threads] ;
   m_pool = new thread*[num_threads] ;
   if (!m_queues || !m_pool)
      {
      delete m_queues ;
      m_queues = nullptr ;
      delete m_pool ;
      m_pool = nullptr ;
      m_numthreads = 0 ;
      return ;
      }
   for (unsigned i = 0 ; i < num_threads ; i++)
      {
      m_pool[i] = new_thread(work_function,this,i) ;
      }
#endif /* FrSINGLE_THREADED */
   return ;
}

//----------------------------------------------------------------------------

ThreadPool::~ThreadPool()
{
#ifndef FrSINGLE_THREADED
   // tell each thread to terminate by sending it a request with a special termination function
   for (unsigned i = 0 ; i < numThreads() ; i++)
      {
      WorkOrder *order = makeWorkOrder(nullptr,nullptr,nullptr) ;
      while (!m_queues[i].push(order))
	 {
	 this_thread::sleep_for(std::chrono::milliseconds(1)) ;
//FIXME: use wait() instead of sleeping
	 }
      }
   // join all the worker threads and then delete them
   for (unsigned i = 0 ; i < numThreads() ; i++)
      {
      m_pool[i]->join() ;
      delete m_pool[i] ;
      m_pool[i] = nullptr ;
      }
   delete [] m_pool ;
   delete [] m_queues ;
#endif /* !FrSINGLE_THREADED */
   m_numthreads = 0 ;
   return ;
}

//----------------------------------------------------------------------------

WorkOrder* ThreadPool::makeWorkOrder(ThreadPoolWorkFunc* fn, const void* in, void* out)
{
   if (!m_freeorders)
      {
      static std::mutex mtx ;
      // allocate a batch of WorkOrder
      std::lock_guard<std::mutex> lock(mtx) ;
      WorkBatch* batch = new WorkBatch(m_batches) ;
      m_batches = batch ;
      m_flguard.lock() ;
      batch->addToFreeList(m_freeorders) ;
      m_flguard.unlock() ;
      }
   m_flguard.lock() ;
   WorkOrder* order = (WorkOrder*)m_freeorders ;
   m_freeorders = m_freeorders->next() ;
   m_flguard.unlock() ;
   return new (order) WorkOrder(fn,in,out) ;
}

//----------------------------------------------------------------------------

void ThreadPool::recycle(WorkOrder* order)
{
   if (order)
      {
      m_flguard.lock() ;
      order->next(m_freeorders) ;
      m_freeorders = order ;
      m_flguard.unlock() ;
      }
   return  ;
}

//----------------------------------------------------------------------------

void ThreadPool::discardRecycledOrders()
{
   m_freeorders = nullptr ;  //FIXME: make an atomic exchange
   WorkBatch* batches = m_batches ;
   m_batches = nullptr ;
   while (batches)
      {
      WorkBatch* batch = batches ;
      batches = batches->next() ;
      delete batch ;
      }
   return ;
}

//----------------------------------------------------------------------------

unsigned ThreadPool::idleThreads() const
{
   //FIXME
   return 0 ;
}

//----------------------------------------------------------------------------

bool ThreadPool::limitThreads(unsigned numthreads)
{
   if (numthreads > m_numthreads)
      return false ;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

static bool insert_request(WorkQueue& queue, WorkOrder* order)
{
   bool was_empty = queue.empty() ;
   bool success = queue.push(order) ;
   if (success && was_empty)
      {
      // if the request queue was empty and the thread is paused, wake it up
//FIXME
      }
   return success ;
}

//----------------------------------------------------------------------------

bool ThreadPool::dispatch(WorkOrder* order)
{
   if (!order) return false ;
   if (availThreads() == 0)
      {
      // we don't have any worker threads enabled, so directly invoke the worker function
      if (order->worker())
	 order->worker()(order->input(),order->output()) ;
      return true ;
      }
   for ( ; ; )
      {
      // do a round-robin scan of the worker threads for one with space in its request queue
      unsigned start_thread = m_next_thread ;
      unsigned threadnum = start_thread ;
      do {
         // atomically attempt to insert the request in the queue; this can fail if there
         //   was only one free entry and another thread beat us to the punch, in addition
         //   to failing if the queue is already full
         if (insert_request(m_queues[m_next_thread],order))
	    {
	    m_next_thread = threadnum ;
	    return true ;
	    }
         threadnum++ ;
	 if (threadnum >= m_availthreads)
	    threadnum = 0 ;
         } while (threadnum != start_thread) ;
      // if all of the queues are full, go to sleep until a request is completed
//FIXME: (Q&D: just sleep for a millisecond)
      this_thread::sleep_for(std::chrono::milliseconds(1)) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ThreadPool::dispatch(ThreadPoolWorkFunc* fn, const void* input, void* output)
{
   if (fn == nullptr) return false ;
   WorkOrder* order = makeWorkOrder(fn,input,output) ;
   return dispatch(order) ;
}

//----------------------------------------------------------------------------

void ThreadPool::waitUntilIdle()
{
//FIXME
   discardRecycledOrders() ;		// keep memory use from growing excessively
   return ;
}

//----------------------------------------------------------------------------

WorkOrder* ThreadPool::nextOrder(unsigned index)
{
   if (index < numThreads())
      {
      WorkOrder* order ;
      while ((order = m_queues[index].pop()) == nullptr)
	 {
	 this_thread::sleep_for(std::chrono::milliseconds(1)) ;
//FIXME: wait until something is pushed
	 }
      }
   return nullptr ;
}

//----------------------------------------------------------------------------

void ThreadPool::threadExiting(unsigned index)
{
   if (index < numThreads())
      {
//FIXME
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file threadpool.C //
