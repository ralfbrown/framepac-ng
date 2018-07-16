/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.07, last edit 2018-07-16					*/
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

#include "framepac/atomic.h"

#if __cplusplus < 201103L
# error This code requires C++11
#endif

#include <stdlib.h>

using namespace std ;

/************************************************************************/
/************************************************************************/

namespace Fr {

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
	 { return !m_mutex.exchange(true,std::memory_order_acquire) ; }
      void backoff_lock() ;
   public:
      CriticalSection() { m_mutex.store(false) ; }
      [[gnu::always_inline]] ~CriticalSection() {}
      void lock()
	 {
	    if (!try_lock())
	       backoff_lock() ;
	 }
      void unlock()
	 {
	    m_mutex.store(false,std::memory_order_release) ;
	 }
      [[gnu::always_inline]] bool locked() const { return m_mutex.load() ; }
      [[gnu::always_inline]] void incrCollisions() { ++s_collisions ; }
   } ;
#endif /* FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

class ScopeCriticalSection : public CriticalSection
   {
   public:
      typedef CriticalSection super ;
   public:
      ScopeCriticalSection() { lock() ; }
      ~ScopeCriticalSection() { unlock() ; }
   } ;


} // end namespace Fr

#endif /* !__Fr_CRITSECT_H_INCLUDED */

// end of file critsect.h //
