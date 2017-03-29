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

#include <chrono>
#include <thread>

#include "framepac/threadpool.h"
using namespace std ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

static void work_function(ThreadPool* pool, unsigned thread_index)
{
   if (!pool)
      return ;
   for ( ; ; )
      {
      WorkOrder* order = pool->nextOrder(thread_index) ;
      if (!order)
	 {
	 pool->threadExiting(thread_index) ;
	 return ;
	 }
      ThreadPoolWorkFunc* fn = order->worker() ;
      if (fn)
	 fn(order->input(),order->output()); 
      }
   return ;
}

/************************************************************************/
/************************************************************************/

ThreadPool::ThreadPool(unsigned num_threads)
   : m_numthreads(num_threads), m_availthreads(0)
#ifndef FrSINGLE_THREADED
   , m_next_thread(0), m_pool(nullptr), m_queues(nullptr)
#endif /* !FrSINGLE_THREADED */
{
#ifdef FrSINGLE_THREADED
   m_numthreads = 0 ;
#else
   m_numthreads = num_threads ;
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
      m_pool[i] = new thread(work_function,this,i) ;
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
//FIXME
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
   (void)queue; (void)order; //FIXME

   // if the request queue was empty and the thread is paused, wake it up

   return true ;
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
      std::this_thread::sleep_for(std::chrono::milliseconds(1)) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ThreadPool::dispatch(ThreadPoolWorkFunc* fn, const void* input, void* output)
{
   if (fn == nullptr) return false ;
   WorkOrder* order = new WorkOrder(fn,input,output) ;
   return dispatch(order) ;
}

//----------------------------------------------------------------------------

void ThreadPool::waitUntilIdle()
{
//FIXME
   return ;
}

//----------------------------------------------------------------------------

WorkOrder* ThreadPool::nextOrder(unsigned index)
{
   if (index < numThreads())
      {
//FIXME
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
