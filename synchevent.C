/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/*  File synchevent.C		synchronization events			*/
/*									*/
/*  (c) Copyright 2015,2016,2017 Carnegie Mellon University		*/
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
#include "framepac/synchevent.h"

#undef __IMPLEMENTED__  // will be defined if we have a platform-specific implementation

namespace Fr
{

/************************************************************************/
/************************************************************************/
/*	Platform-specific implementation: Linux				*/
/************************************************************************/
/************************************************************************/

#ifndef FrSINGLE_THREADED
# if defined(__linux__)
#   define __IMPLEMENTED__

// code for sys_futex from http://locklessinc.com/articles/obscure_synch/
#include <linux/futex.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
static long sys_futex(void *addr1, int op, int val1, struct timespec *timeout, void *addr2, int val3)
{
   return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

/************************************************************************/
/*	Methods for class SynchEvent					*/
/************************************************************************/

bool SynchEvent::isSet() const
{
   return m_event.m_set.load() ;
}

//----------------------------------------------------------------------

void SynchEvent::clear()
{
   m_event.m_set.store(false) ;
   return ;
}

//----------------------------------------------------------------------

void SynchEvent::set()
{
   m_event.m_set.store(true) ;
   if (m_event.m_waiters.load())
      {
      m_event.m_waiters.store(false) ;
      sys_futex(this, FUTEX_WAKE_PRIVATE, INT_MAX, NULL, NULL, 0) ;
      }
   return ;
}

//----------------------------------------------------------------------

void SynchEvent::wait()
{
   while (!m_event.m_set.load())
      {
      m_event.m_waiters.store(true) ;
      // wait as long as m_set is still false and m_waiters is still true
      sys_futex(this, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0) ;
      }
   return ;
}

/************************************************************************/
/*	Methods for class SynchEventCounted				*/
/************************************************************************/

bool SynchEventCounted::isSet() const
{
   return (m_futex_val.load() & m_mask_set) != 0 ;
}

//----------------------------------------------------------------------

unsigned SynchEventCounted::numWaiters() const
{
   return (m_futex_val.load() & ~m_mask_set) ;
}

//----------------------------------------------------------------------

void SynchEventCounted::clear()
{
   (void)m_futex_val.test_and_clear_mask(m_mask_set) ;
   return  ;
}

//----------------------------------------------------------------------

void SynchEventCounted::clearAll()
{
   m_futex_val.store(0U) ;
   return  ;
}

//----------------------------------------------------------------------

void SynchEventCounted::set()
{
   unsigned status { m_futex_val.test_and_set_mask(m_mask_set) };
   if ((status & m_mask_set) == 0 && status > 0)
      {
      // event hasn't triggered yet, but there are waiters, so wake them up
      sys_futex(&m_futex_val, FUTEX_WAKE_PRIVATE, INT_MAX, NULL, NULL, 0) ;
      }
   return ;
}

//----------------------------------------------------------------------

void SynchEventCounted::wait()
{
   // we need to loop, since sys_futex can experience spurious wakeups
   while ((m_futex_val++ & m_mask_set) == 0)
      {
      size_t wait_while { m_futex_val.load() & ~m_mask_set };
      // wait as long as 'set' bit is still false and numWaiters is unchanged
      sys_futex(&m_futex_val, FUTEX_WAIT_PRIVATE, wait_while, NULL, NULL, 0) ;
      --m_futex_val ;
      }
   // we're done waiting
   --m_futex_val ;
   return ;
}

//----------------------------------------------------------------------

void SynchEventCounted::waitForWaiters()
{
   set() ;
   while ((m_futex_val.load() & ~m_mask_set) > 0)
      {
      std::this_thread::yield() ;
      }
   return  ;
}

/************************************************************************/
/*	Methods for class SynchEventCountdown				*/
/************************************************************************/

void SynchEventCountdown::consume()
{
   if (m_futex_val.fetch_sub(2) <= 3)
      {
      // if counter was 1 before the decrement, wake everyone who is waiting
      consumeAll() ;
      }
   return ;
}

//----------------------------------------------------------------------

void SynchEventCountdown::consumeAll()
{
   if (m_futex_val.exchange(0) & 1)
      {
      // someone is waiting, so wake all the waiters
      sys_futex(this, FUTEX_WAKE_PRIVATE, INT_MAX,NULL, NULL, 0) ;
      }
   return ;
}

//----------------------------------------------------------------------

void SynchEventCountdown::wait()
{
   // loop until the countdown has expired, because sys_futex can wake
   //   spuriously, or it may have never gone to sleep because someone
   //   called consume() during the interval when we set the 'waiting'
   //   flag
   while (m_futex_val.load() > 1)
      {
      int counter { m_futex_val.test_and_set_mask(1) };
      if (counter > 1)
	 {
	 // wait as long as 'waiting' bit is still set and count > 0
	 sys_futex(this, FUTEX_WAIT_PRIVATE, (counter|1), NULL, NULL, 0) ;
	 }
      }
   return ;
}

# endif /* __linux__ */
#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/
/*	Platform-specific implementation: MSWindows			*/
/************************************************************************/
/************************************************************************/

#ifndef FrSINGLE_THREADED
# if defined(__WINDOWS__)
#   define __IMPLEMENTED__

#include "windows.h"
// Windows API calls from https://msdn.microsoft.com/en-us/magazine/jj721588.aspx

/************************************************************************/
/*	Methods for class SynchEvent					*/
/************************************************************************/

SynchEvent::SynchEvent()
   : m_eventhandle(CreateEvent(NULL, true, false, NULL))
{
   return ;
}

//----------------------------------------------------------------------

SynchEvent::~SynchEvent()
{
   CloseHandle(m_eventhandle) ;
   return ;
}

//----------------------------------------------------------------------

void SynchEvent::set()
{
   SetEvent(m_eventhandle) ;
   return ;
}

//----------------------------------------------------------------------

void SynchEvent::clear()
{
   ResetEvent(m_eventhandle) ;
   return ;
}

//----------------------------------------------------------------------

void SynchEvent::wait()
{
   WaitForSingleObject(m_eventhandle, INFINITE) ;
   return ;
}

/************************************************************************/
/*	Methods for class SynchEventCounted				*/
/************************************************************************/

bool SynchEventCounted::isSet() const
{
   return false ; //FIXME
}

//----------------------------------------------------------------------

unsigned SynchEventCounted::numWaiters() const
{
   return 0 ; //FIXME
}

//----------------------------------------------------------------------

void SynchEventCounted::clear()
{
//FIXME
   return  ;
}

//----------------------------------------------------------------------

void SynchEventCounted::clearAll()
{
//FIXME
   return  ;
}

//----------------------------------------------------------------------

void SynchEventCounted::set()
{
//FIXME
   return ;
}

//----------------------------------------------------------------------

void SynchEventCounted::wait()
{
//FIXME
   return ;
}

//----------------------------------------------------------------------

void SynchEventCounted::waitForWaiters()
{
//FIXME
   return  ;
}

/************************************************************************/
/*	Methods for class SynchEventCountdown				*/
/************************************************************************/

/* looks like InitializeSynchronizationBarrier and
 * EnterSynchronizationBarrier are similar to what I want, but
 * Enter... blocks until all have arrived.  The closest match I can
 * see is semaphores initialized with a negative count [not allowed],
 * where consume() increments the count, but they would only release
 * one thread rather than all.
 */

void SynchEventCountdown::consume()
{
//FIXME
   return ;
}

//----------------------------------------------------------------------

void SynchEventCountdown::consumeAll()
{
//FIXME
   return ;
}

//----------------------------------------------------------------------

void SynchEventCountdown::wait()
{
//FIXME
   return ;
}

# endif /* __WINDOWS__ */
#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/
/*	Platform-specific implementation: generic PThreads		*/
/************************************************************************/
/************************************************************************/

#ifndef FrSINGLE_THREADED
# if !defined(__IMPLEMENTED__)

/************************************************************************/
/*	Global variables for class FrCriticalSection			*/
/************************************************************************/

pthread_mutex_t FrCriticalSection::s_mutex { PTHREAD_MUTEX_INITIALIZER };

/************************************************************************/
/*	Methods for class SynchEvent					*/
/************************************************************************/

SynchEvent::SynchEvent()
{
   phtread_mutex_init(&m_mutex, 0) ;
   pthread_cond_init(&m_condvar, 0) ;
   m_set = false ;
   return ;
}

//----------------------------------------------------------------------

SynchEvent::~SynchEvent()
{
   pthread_mutex_destroy(&m_mutex) ;
   pthread_cond_destroy(&m_condvar) ;
   return ;
}

//----------------------------------------------------------------------

void SynchEvent::isSet() const
{
   return m_set ;
}

//----------------------------------------------------------------------

void SynchEvent::clear()
{
   //FIXME
   pthread_mutex_lock(&m_mutex) ;
   m_set = false ;
   pthread_mutex_unlock(&m_mutex) ;
   return ;
}

//----------------------------------------------------------------------

void SynchEvent::set()
{
   //FIXME
   pthread_mutex_lock(&m_mutex) ;
   m_set = true ;
   pthread_cond_broadcast(&m_condvar) ;
   pthread_mutex_unlock(&m_mutex) ;
   return  ;
}

//----------------------------------------------------------------------

void SynchEvent::wait()
{
   pthread_mutex_lock(&m_mutex) ;
   int result ;
   do
      {
      result = pthread_cond_wait(&m_condvar, &m_mutex) ;
      } while (result == 0 && !m_set) ;
   pthread_mutex_unlock(&m_mutex) ;
   return ;
}

/************************************************************************/
/*	Methods for class SynchEventCounted				*/
/************************************************************************/

bool SynchEventCounted::isSet() const
{
   return false ; //FIXME
}

//----------------------------------------------------------------------

unsigned SynchEventCounted::numWaiters() const
{
   return 0 ; //FIXME
}

//----------------------------------------------------------------------

void SynchEventCounted::clear()
{
//FIXME
   return  ;
}

//----------------------------------------------------------------------

void SynchEventCounted::clearAll()
{
//FIXME
   return  ;
}

//----------------------------------------------------------------------

void SynchEventCounted::set()
{
//FIXME
   return ;
}

//----------------------------------------------------------------------

void SynchEventCounted::wait()
{
//FIXME
   return ;
}

//----------------------------------------------------------------------

void SynchEventCounted::waitForWaiters()
{
   set() ;
//FIXME
   return  ;
}

#  include "critsect-pthread.C"
# endif /* __IMPLEMENTED__ */
#endif /* !FrSINGLE_THREADED */

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file synchevent.C //
