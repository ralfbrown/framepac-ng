/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-01					*/
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

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include "framepac/thread.h"
#include "framepac/threadpool.h"
using namespace std ;

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

// size of request queue for each worker thread in pool; should be power of 2
// workers can steal from another's queue if theirs is empty
#define FrWORKQUEUE_SIZE 512

// increment in which we allocate WorkOrder records
#define BATCH_SIZE 250

/************************************************************************/
/************************************************************************/

namespace Fr
{

// some addresses to pass to get specific actions from the worker thread
static bool request_ack ;
static bool request_exit ;

/************************************************************************/
/************************************************************************/

class WorkOrder
   {
   public:
      WorkOrder() {}
      WorkOrder(ThreadPoolWorkFunc* fn, const void* in, void* out)
	 : m_func(fn), m_input(in), m_output(out) {}
      WorkOrder(const WorkOrder&) = delete ;
      ~WorkOrder() {}
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
   } ;

/************************************************************************/
/************************************************************************/

class WorkQueue
   {
   public:
      WorkQueue() {}
      WorkQueue(const WorkQueue&) = delete ;
      ~WorkQueue() ;
      WorkQueue& operator= (const WorkQueue&) = delete ;

      bool empty() const { return m_head >= m_tail.load() ; }

      WorkOrder* fastPop() ;		// canonly be called by queue's owner
      WorkOrder* pop(bool block = true) ; // can only be called by queue's owner
      bool push(WorkOrder* order) ;	// can be called by any thread
      WorkOrder* steal() ;		// can be called by any thread
      void clear() ;			// remove all entries from queue

      // inter-thread synchronization
      void post()
	 {
	    {
	    std::unique_lock<std::mutex> lock(m_mutex) ;
	    m_posted = true ;
	    } // release the lock so that the awakened thread doesn't block on the mutex
	    m_cond.notify_one() ;
	 }
      void wait()
	 {
	    std::unique_lock<std::mutex> lock(m_mutex) ;
	    while (!m_posted)
	       m_cond.wait(lock) ;
	    m_posted = false ;		// reset for the next time before we unlock
	 }

   protected:
      std::mutex	 m_mutex ;
      std::condition_variable m_cond ;
      Semaphore		 m_jobs { 0 } ;
      size_t             m_head { 0 } ;
      Atomic<WorkOrder*> m_orders[FrWORKQUEUE_SIZE] = { nullptr } ;
      Atomic<size_t>     m_tail { 0 } ;
      bool		 m_posted { false } ;
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
/*	Global variables						*/
/************************************************************************/

ThreadPool* ThreadPool::s_defaultpool = nullptr ;

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
      if (fn)
	 {
	 fn(in,out); 
	 }
      else if (in == &request_ack)
	 {
	 pool->ack(thread_index) ;
	 }
      else if (in == &request_exit)
	 {
	 pool->threadExiting(thread_index) ;
	 return ;
	 }
      }
   return ;
}

/************************************************************************/
/*	Methods for class WorkQueue					*/
/************************************************************************/

WorkQueue::~WorkQueue()
{
   clear() ;
   return  ;
}

//----------------------------------------------------------------------------

WorkOrder* WorkQueue::fastPop()
{
   // implement the fast-path retrieval of the next task, for the case where the
   //   queue is non-empty and the entry pointed at by m_head has not been stolen
   if (m_head < m_tail.load())
      {
      size_t idx = (m_head++) % FrWORKQUEUE_SIZE ;
      return m_orders[idx].exchange(nullptr) ;
      }
   return nullptr ;
}

//----------------------------------------------------------------------------

WorkOrder* WorkQueue::pop(bool block)
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
   if (block)
      {
      wait() ;
      }
   return nullptr ;	// queue is empty
}

//----------------------------------------------------------------------------

WorkOrder* WorkQueue::steal()
{
   return nullptr; //FIXME: have temporarily disabled stealing
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
   else if (!order->worker())
      {
      // we're not allowed to steal commands to the worker
      push(order) ;
      return nullptr ;
      }
   return order ;
}

//----------------------------------------------------------------------------

bool WorkQueue::push(WorkOrder* order)
{
   if (!order)
      return false ;			// nothing to push
   size_t tail = m_tail.load() ;
   size_t fullpos = m_head + FrWORKQUEUE_SIZE ;
   for ( ; ; )
      {
      if (tail >= fullpos)
	 return false ;			// queue is full
      order = m_orders[tail % FrWORKQUEUE_SIZE].exchange(order) ;
      if (!order && m_tail.compare_exchange_weak(tail,tail+1))
	 break ;
      // if the previous value of the entry was non-null, that means someone else snuck in and
      //   added a task, so retry
      tail = m_tail.load() ;
      }
   post() ;				// signal worker to restart if it was waiting
   return true ;
}

//----------------------------------------------------------------------------

void WorkQueue::clear()
{
//FIXME
   m_tail = 0 ;
   m_head = 0 ;
   return ;
}

/************************************************************************/
/*	Methods for class ThreadPool					*/
/************************************************************************/

ThreadPool::ThreadPool(unsigned num_threads)
   : m_numthreads(num_threads)
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
   allocateWorkOrders() ;
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
      WorkOrder *order = makeWorkOrder(nullptr,&request_exit,nullptr) ;
      while (!m_queues[i].push(order))
	 {
	 this_thread::sleep_for(std::chrono::milliseconds(1)) ;
//FIXME: can we use wait() instead of sleeping?
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
   discardRecycledOrders() ;
#endif /* !FrSINGLE_THREADED */
   m_numthreads = 0 ;
   return ;
}

//----------------------------------------------------------------------------

ThreadPool* ThreadPool::defaultPool()
{
   if (!s_defaultpool)
      {
      // we haven't yet set the default thread pool, so construct one now
      unsigned threads = 0 ;
#ifndef FrSINGLE_THREADED
      threads = std::thread::hardware_concurrency() ;
      if (threads < 2) threads = 2 ;
#endif /* FrSINGLE_THREADED */
      s_defaultpool = new ThreadPool(threads) ;
      }
   return s_defaultpool ;
}

//----------------------------------------------------------------------------

void ThreadPool::defaultPool(ThreadPool* pool)
{
   delete s_defaultpool ;
   s_defaultpool = pool ;
   return ;
}

//----------------------------------------------------------------------------

void ThreadPool::allocateWorkOrders()
{
   if (m_freeorders)
      return ;
   // pre-allocate up to 1,000,000 WorkOrder instances or enough to
   //   fill all of the work queues plus extras for worker commands
   //   and blocked insertions, whichever is less
   size_t desired = std::min(1000000U, (FrWORKQUEUE_SIZE+3) * numThreads()) ;
   size_t num_batches = (desired + BATCH_SIZE) / BATCH_SIZE ;
   for (size_t i = 0 ; i < num_batches ; i++)
      {
      WorkBatch *batch = new WorkBatch(m_batches) ;
      m_batches = batch ;
      batch->addToFreeList(m_freeorders) ;
      }
   return ;
}

//----------------------------------------------------------------------------

WorkOrder* ThreadPool::makeWorkOrder(ThreadPoolWorkFunc* fn, const void* in, void* out)
{
   if (!m_freeorders)
      {
      // allocate a batch of WorkOrder
      m_flguard.lock() ;
      WorkBatch* batch = new WorkBatch(m_batches) ;
      m_batches = batch ;
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
   m_flguard.lock() ;
   m_freeorders = nullptr ;
   WorkBatch* batches = m_batches ;
   m_batches = nullptr ;
   m_flguard.unlock() ;
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
   //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

bool ThreadPool::dispatch(WorkOrder* order)
{
   if (!order) return false ;
   if (numThreads() == 0)
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
         // advance to next thread in pool
         threadnum++ ;
	 if (threadnum >= numThreads())
	    threadnum = 0 ;
         // atomically attempt to insert the request in the queue; this can fail if there
         //   was only one free entry and another thread beat us to the punch, in addition
         //   to failing if the queue is already full
         if (m_queues[threadnum].push(order))
	    {
	    m_next_thread = threadnum ;
	    return true ;
	    }
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
   if (numThreads() == 0)
      {
      // we don't have any worker threads enabled, so directly invoke the worker function
      fn(input,output) ;
      return true ;
      }
   WorkOrder* order = makeWorkOrder(fn,input,output) ;
   return dispatch(order) ;
}

//----------------------------------------------------------------------------

void ThreadPool::waitUntilIdle()
{
   if (numThreads() == 0)
      return ;				// all jobs were handled immediately
   // tell all the workers to post when they've finished everything currently in
   //   their queue
   for (size_t i = 0 ; i < numThreads() ; ++i)
      {
      WorkOrder* wo = makeWorkOrder(nullptr,&request_ack,nullptr) ;
      while (!m_queues[i].push(wo))
	 {
	 // queue was full, so retry in a little bit
	 this_thread::yield() ;
	 }
      }
   // wait until all of the workers have responded
   for (size_t i = 0 ; i < numThreads() ; ++i)
      {
      m_ack.wait() ;
      }
//FIXME
   return ;
}

//----------------------------------------------------------------------------

WorkOrder* ThreadPool::nextOrder(unsigned index)
{
   assert(index < numThreads()) ;
   WorkOrder* wo = m_queues[index].fastPop() ;
   if (wo)
      return wo ;
   bool block = false ;
   while ((wo = m_queues[index].pop(block)) == nullptr)
      {
      // try to steal something from another queue
      // TODO: be more sophisticated than a simple round-robin scan
      unsigned nt = numThreads() ;
      for (unsigned next = (index+1)%nt ; next != index ; next = (next+1)%nt)
	 {
	 wo = m_queues[next].steal() ;
	 if (wo)
	    return wo ;
	 }
      // couldn't steal anything, so try again, but this time, block until
      //   something gets added to the queue
      block = true ;
      }
   return wo ;
}

//----------------------------------------------------------------------------

void ThreadPool::ack(unsigned /*index*/)
{
   m_ack.post() ;
   return  ;
}

//----------------------------------------------------------------------------

void ThreadPool::threadExiting(unsigned index)
{
   assert(index < numThreads()) ;

//FIXME
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file threadpool.C //
