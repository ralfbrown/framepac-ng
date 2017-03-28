/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/*  File synchevent.h		synchronization events			*/
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

#ifndef __Fr_SYNCHEVENT_H_INCLUDED
#define __Fr_SYNCHEVENT_H_INCLUDED

#include <cstdint>
#include <limits>
#include "framepac/atomic.h"

using namespace std ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

#ifdef FrSINGLE_THREADED
class SynchEvent
   {
   private:
      Atomic<bool> m_set ;
   public:
      SynchEvent() { clear() ; }
      ~SynchEvent() {}

      bool isSet() const [[gnu::always_inline]]
         { return m_set ; }
      void clear() [[gnu::always_inline]]
         { m_set = false ; }
      void set() [[gnu::always_inline]]
         { m_set = true ; }
      void wait() [[gnu::always_inline]]
         {}
   } ;
#else // multi-threaded version
class SynchEvent
   {
#ifdef  __linux__
      union
         {
	 atom_uint m_futex_val ;
	 struct
	    {
	    Atomic<bool> m_waiters ;
	    Atomic<bool> m_set ;
	    } m_event ;
         } ;
   public:
      SynchEvent() { clearAll() ; }
      ~SynchEvent() { set() ; }
      void clearAll() { m_futex_val.store(0U) ; }
#elif defined(__WINDOWS__)
      HANDLE m_eventhandle ;
   public:
      SynchEvent() ;
      ~SynchEvent() ;
#else // use pthreads
      phtread_mutex_t  m_mutex ;
      pthread_cond_t   m_condvar ;
      bool	       m_set ;
      bool	       m_waiting ;
   public:
      SynchEvent() ;
      ~SynchEvent() ;
#endif /* __WINDOWS__ */

      bool isSet() const ;
      void clear() ;
      void set() ;

      void wait() ;
   } ;
#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

#ifdef FrSINGLE_THREADED
// non-threaded version -- wait always returns immediately, since
//   otherwise we'd just block forever
class SynchEventCounted
   {
   private:
      bool m_set ;
   public:
      SynchEventCounted() { clearAll() ; }
      ~SynchEventCounted() { waitForWaiters() ; }

      bool isSet() const { return m_set ; }
      unsigned numWaiters() const { return 0 ; }
      void clear() { m_set = false ; }
      void clearAll() { clear() ; }
      void set() { m_set = true ; }
      void wait() [[gnu::always_inline]] {}
      void waitForWaiters() [[gnu::always_inline]] {}
   } ;
#else
// multi-threaded version
class SynchEventCounted
   {
   private:
#ifdef __linux__
      atom_uint m_futex_val ;
      static const unsigned m_mask_set =
	 (std::numeric_limits<unsigned>::max() - std::numeric_limits<int>::max()) ;
//(UINT_MAX - INT_MAX) ;
      // high bit is set/clear status of event, remainder is number of waiters
#else
      //FIXME: implementations for other platforms
#endif /* __linux__ */
   public:
      SynchEventCounted() { clearAll() ; }
      ~SynchEventCounted() { waitForWaiters() ; } // release any waiters

      bool isSet() const ;
      unsigned numWaiters() const ;
      void clear() ;
      void clearAll() ;
      void set() ;
      void wait() ;
      void waitForWaiters() ;
   } ;
#endif /* FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

#ifdef FrSINGLE_THREADED
// non-threaded version -- wait always returns immediately, since
//   otherwise we'd just block forever
class SynchEventCountdown
   {
   private:
      int m_counter ;
   public:
      SynchEventCountdown() { clear() ; }
      ~SynchEventCountdown() { consumeAll() ; }

      void init(int count) { m_counter = (count << 1) ; }
      bool isDone() const { return m_counter <= 1 ; }
      int countRemaining() const { return m_counter >> 1 ; }
      void clear() { m_counter = 0 ; }
      void consume() { m_counter -= 2 ; }
      void consumeAll() { m_counter = 0 ; }
      bool haveWaiters() const { return (m_counter & 1) != 0 ; }
      void wait() [[gnu::always_inline]] {}
   } ;
#else
class SynchEventCountdown
   {
   private:
#ifdef __linux__
      // low bit indicates whether anybody is blocked on the
      //   countdown, remaining bits are the count until the event
      //   fires
      atom_int m_futex_val ;
#else
      //FIXME: implementation for other platforms
#endif /* __linux__ */
   public: 
      SynchEventCountdown() { clear() ; }
      ~SynchEventCountdown() { consumeAll() ; }

#ifdef __linux__
      void init(int count) { m_futex_val.store(count << 1) ; }
      bool isDone() const { return m_futex_val.load() <= 1 ; }
      int countRemaining() const { return m_futex_val.load() >> 1 ; }
      void clear() { m_futex_val.store(0) ; }
      bool haveWaiters() const { return (m_futex_val.load() & 1) != 0 ; }
#endif /* __linux__ */
      void consume() ;
      void consumeAll() ;
      void wait() ;
   } ;
#endif /* FrSINGLE_THREADED */

} // end namespace Fr

#endif /* !__Fr_SYNCHEVENT_H_INCLUDED */

// end of file syncheven.h //
