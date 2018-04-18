/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-12					*/
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

#include <iostream>

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
#ifndef FrSINGLE_THREADED
static bool request_exit ;
#endif /* !FrSINGLE_THREADED */

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
      WorkQueue()
	 { for (size_t i = 0 ; i < lengthof(m_orders) ; ++i) m_orders[i] = nullptr ; }
      WorkQueue(const WorkQueue&) = delete ;
      ~WorkQueue() ;
      WorkQueue& operator= (const WorkQueue&) = delete ;

      bool empty() const { return m_head >= m_tail.load() ; }

      WorkOrder* fastPop() ;		// canonly be called by queue's owner
      WorkOrder* pop() ;		// can only be called by queue's owner
      bool push(WorkOrder* order) ;	// can be called by any thread
      WorkOrder* steal() ;		// can be called by any thread
      void clear() ;			// remove all entries from queue

      size_t pushMultiple(ThreadPool* pool, ThreadPoolWorkFunc* fn, size_t maxcount,
			  size_t insize, const void* input, size_t outsize, void* output) ;

      // inter-thread synchronization
      void prepare_wait()
	 {
	 if (m_spurious_wakeup.exchange(false))
	    {
	    m_jobs.wait() ;		// resynchronize after a spurious wakeup
	    }
	 m_waiting = true ;
	 }
      void cancel_wait()
	 {
	 m_spurious_wakeup = true ;
	 if (m_waiting.load())
	    {
	    m_spurious_wakeup = false ;
	    m_waiting = false ;
	    }
	 }
      void commit_wait()
	 {
	 m_jobs.wait() ;
	 }
      void notify()
	 {
	 if (!m_waiting.exchange(false)) return ;
	 m_jobs.post() ;
	 }

   protected:
      atom_bool		 m_waiting ;
      atom_bool		 m_spurious_wakeup ;
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
/************************************************************************/

struct ParallelJob
   {
   public:
      ThreadPoolMapFunc* fn ;
      size_t	         id ;
      va_list	         args ;
   } ;

/************************************************************************/
/*	Global variables						*/
/************************************************************************/

ThreadPool* ThreadPool::s_defaultpool = nullptr ;

/************************************************************************/
/************************************************************************/

#ifndef FrSINGLE_THREADED
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
#endif /* !FrSINGLE_THREADED */

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

WorkOrder* WorkQueue::pop()
{
   while (m_head < m_tail.load())
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
      }
   return nullptr ;	// queue is empty
}

//----------------------------------------------------------------------------

WorkOrder* WorkQueue::steal()
{
   size_t tail = m_tail.load() ;
   // try to grab a task by atomically swapping the pointer for the last item in the queue
   WorkOrder* order = m_orders[tail%FrWORKQUEUE_SIZE].exchange(nullptr) ;
   if (order && !order->worker())
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
   size_t fullpos = m_head + FrWORKQUEUE_SIZE ;
   for ( ; ; )
      {
      size_t tail = m_tail.load() ;
      if (tail >= fullpos)
	 return false ;			// queue is full
      order = m_orders[tail % FrWORKQUEUE_SIZE].exchange(order) ;
      if (!order && m_tail.compare_exchange_weak(tail,tail+1))
	 break ;
      // if the previous value of the entry was non-null, that means someone else snuck in and
      //   added a task, so retry
      }
   notify() ;				// signal worker to restart if it was waiting
   return true ;
}

//----------------------------------------------------------------------------

size_t WorkQueue::pushMultiple(ThreadPool* pool, ThreadPoolWorkFunc* fn, size_t maxcount,
			       size_t insize, const void* input, size_t outsize, void* output)
{
   size_t fullpos = m_head + FrWORKQUEUE_SIZE ;
   size_t tail = m_tail.load() ;
   size_t available = fullpos - tail ;
   if (available == 0)
      return 0 ;
   if (available < maxcount)
      maxcount = available ;
   for (size_t i = 0 ; i < maxcount ; ++i)
      {
      WorkOrder* order = pool->makeWorkOrder(fn,((char*)input)+i*insize,((char*)output)+i*outsize) ;
      m_orders[(tail + i) % FrWORKQUEUE_SIZE] = order ;
      }
   m_tail += maxcount ;
   notify() ;
   return maxcount ;
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
   m_numCPUs = std::thread::hardware_concurrency() ;
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
   if (!order) return false ;
   for ( ; ; )
      {
      // do a round-robin scan of the worker threads for one with space in its request queue
      unsigned start_thread = m_prev_thread ;
      unsigned threadnum = start_thread ;
      do {
         // advance to next thread in pool
         threadnum = (threadnum+1) % numThreads() ;
         // atomically attempt to insert the request in the queue; this can fail if there
         //   was only one free entry and another thread beat us to the punch, in addition
         //   to failing if the queue is already full
         if (m_queues[threadnum].push(order))
	    {
	    m_prev_thread = threadnum ;
	    return true ;
	    }
         } while (threadnum != start_thread) ;
      // if all of the queues are full, go to sleep until a request is completed
//FIXME: (Q&D: just sleep for a millisecond)
//      this_thread::sleep_for(std::chrono::milliseconds(1)) ;
      this_thread::yield() ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ThreadPool::dispatchBatch(ThreadPoolWorkFunc* fn, size_t count, size_t insize, const void* input,
			       size_t outsize, void* output)
{
   if (numThreads() == 0)
      {
      // we don't have any worker threads enabled, so directly invoke the worker function on each
      //   pair of input and output
      for (size_t i = 0 ; i < count ; ++i)
	 {
	 fn(((char*)input)+i*insize,((char*)output)+i*outsize) ;
	 }
      return true ;
      }
   size_t prev_item { 0 } ;
   size_t curr_item { 0 } ;
   if (input == nullptr) insize = 0 ;
   if (output == nullptr) outsize = 0 ;
   while (curr_item < count)
      {
      size_t batchsize = 1 ;
      size_t remaining = count - curr_item ;
      if (remaining > numThreads())
	 batchsize = remaining / numThreads() ;
      for (size_t t = 0 ; t < numThreads() && curr_item < count ; ++t)
	 {
	 curr_item += m_queues[t].pushMultiple(this,fn,batchsize,insize,((char*)input)+curr_item*insize,
					       outsize,((char*)output)+curr_item*outsize) ;
	 }
      if (curr_item == prev_item)
	 {
	 // all queues were full, so wait a bit to allow the workers to free up space
	 this_thread::yield() ;
	 }
      prev_item = curr_item ;
      }
   return true ;
}

//----------------------------------------------------------------------------

static void parallelize_worker(const void*, void* output)
{
   ParallelJob* job = reinterpret_cast<ParallelJob*>(output) ;
   if (job->fn)
      (void)job->fn(job->id,job->args) ;
   return ;
}

//----------------------------------------------------------------------------

bool ThreadPool::parallelize(ThreadPoolMapFunc* fn, size_t num_items, va_list args)
{
   if (!fn) return false ;
   if (numThreads() == 0 || 1) //FIXME
      {
      // we don't have any worker threads enabled, so directly invoke the mapping function
      bool success = true ;
      for (size_t i = 0 ; success && i < num_items ; ++i)
	 {
	 va_list arg_copy ;
	 va_copy(arg_copy,args) ;
	 success = fn(i,arg_copy) ;
	 va_end(arg_copy) ;
	 }
      return success ;
      }
   // dispatch a job request for each item in the input
   ParallelJob* jobs = new ParallelJob[num_items] ;
   for (size_t i = 0 ; i < num_items ; ++i)
      {
      jobs[i].id = i ;
      jobs[i].fn = fn ;
      va_copy(jobs[i].args,args) ;
      this->dispatch(parallelize_worker,nullptr,&jobs[i]) ;
      }
   this->waitUntilIdle() ;
   delete[] jobs ;
   return false ;
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
   WorkQueue& q = m_queues[index] ;
   WorkOrder* wo = q.fastPop() ;
   if (wo)
      return wo ;
   if ((wo = q.pop()) == nullptr)
      {
      // try to steal something from another queue
      // TODO: be more sophisticated than a simple round-robin scan
      unsigned nt = numThreads() ;
      // if only makes sense to try to steal if our hardware threads are not massively over-subscribed
      if (m_numCPUs == 0 || nt < 4 * m_numCPUs)
	 {
	 for (unsigned next = (index+1)%nt ; next != index ; next = (next+1)%nt)
	    {
	    wo = m_queues[next].steal() ;
	    if (wo)
	       return wo ;
	    }
	 }
      }
   // if there were no jobs on our queue and we weren't able to steal any work,
   //   we should go to sleep until work is available
   if (!wo)
      {
      // two-stage commit to avoid the need for a condition variable
      // 1a. announce that we will be blocking
      q.prepare_wait() ;
      // 1b. verify whether the queue is still empty
      if ((wo = q.pop()) == nullptr)
	 {
	 // 2(usual). actually block until something is added to the queue 
	 q.commit_wait() ;
	 return nextOrder(index) ;
	 }
      else
	 {
	 // 2(alt). someone added something to the queue already, so abort the blocking
	 q.cancel_wait() ;
	 }
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
