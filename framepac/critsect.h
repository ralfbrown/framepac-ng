/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.12, last edit 2018-09-12					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/*  File critsect.h		short-duration critical section mutex	*/
/*									*/
/*  (c) Copyright 2010,2013,2015,2016,2017,2018				*/
/*		Carnegie Mellon University				*/
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

#ifndef __Fr_CRITSECT_H_INCLUDED
#define __Fr_CRITSECT_H_INCLUDED

#include <stdlib.h>
#include "framepac/atomic.h"

#if __cplusplus < 201103L
# error This code requires C++11
#endif

using namespace std ;

/************************************************************************/
/************************************************************************/

namespace Fr {

// my GCC 8.2 installation has a different header file than is in /usr/include/sanitizer, and it
//  doesn't declare this symbol....
#define __tsan_mutex_not_static 0

/************************************************************************/
/************************************************************************/

#ifdef FrSINGLE_THREADED
// non-threaded version
class CriticalSection
   {
   private:
      // no data members
   public:
      [[gnu::always_inline]] CriticalSection() {}
      [[gnu::always_inline]] ~CriticalSection() {}
      [[gnu::always_inline]] void lock() {}
      [[gnu::always_inline]] void unlock() {}
      [[gnu::always_inline]] bool locked() { return false ; }
      [[gnu::always_inline]] void incrCollisions() {}
   } ;
#else
// threaded version, using C++11 atomic<T>
class CriticalSection
   {
   private:
      static Atomic<size_t> s_collisions ;
      Atomic<bool> m_mutex ;
   protected:
      bool try_lock()
	 {
	    TSAN(__tsan_mutex_pre_lock(this,__tsan_mutex_try_lock)) ;
	    bool status = !m_mutex.exchange(true,std::memory_order_acquire) ;
	    TSAN(__tsan_mutex_post_lock(this,status?__tsan_mutex_try_lock:__tsan_mutex_try_lock_failed,1)) ;
	    return status ;
	 }
      void backoff_lock() ;
   public:
      CriticalSection()
	 {
	    TSAN(__tsan_mutex_create(this,__tsan_mutex_not_static)) ;
	    m_mutex.store(false) ;
	 }
      [[gnu::always_inline]]
      ~CriticalSection()
	 {
	    TSAN(__tsan_mutex_destroy(this,__tsan_mutex_not_static)) ;
	 }
      void lock()
	 {
	    TSAN(__tsan_mutex_pre_lock(this,__tsan_mutex_try_lock)) ;
	    if (m_mutex.exchange(true,std::memory_order_acquire))
	       backoff_lock() ;		// flag already set, to back off and try again
	    TSAN(__tsan_mutex_post_lock(this,__tsan_mutex_try_lock,1)) ;
	 }
      void unlock()
	 {
	    TSAN(__tsan_mutex_pre_unlock(this,0)) ;
	    m_mutex.store(false,std::memory_order_release) ;
	    TSAN(__tsan_mutex_post_unlock(this,0)) ;
	 }
      [[gnu::always_inline]]
      bool locked() const
	 {
	    TSAN(__tsan_mutex_pre_lock((void*)this,__tsan_mutex_read_lock)) ;
	    bool status = m_mutex.load() ;
	    TSAN(__tsan_mutex_post_lock((void*)this,__tsan_mutex_read_lock,0)) ;
	    return status ;
	 }
      [[gnu::always_inline]] void incrCollisions() { ++s_collisions ; }
   } ;
#endif /* FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

class ScopeCriticalSection
   {
   public:
      typedef CriticalSection super ;
   public:
      ScopeCriticalSection(CriticalSection* cs) : m_critsect(cs) { cs->lock() ; }
      ScopeCriticalSection(CriticalSection& cs) : m_critsect(&cs) { cs.lock() ; }
      ~ScopeCriticalSection() { m_critsect->unlock() ; }
   protected:
      CriticalSection* m_critsect ;
   } ;


} // end namespace Fr

#endif /* !__Fr_CRITSECT_H_INCLUDED */

// end of file critsect.h //
