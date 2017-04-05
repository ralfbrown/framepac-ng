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

#ifndef _Fr_SEMAPHORE_H_INCLUDED
#define _Fr_SEMAPHORE_H_INCLUDED

#include <semaphore.h>
#include "framepac/config.h"

namespace Fr
{

#ifdef FrSINGLE_THREADED
class Semaphore
   {
   public:
      Semaphore(unsigned = 0) {}
      ~Semaphore() {}

      // when single-threaded, all posts and waits are immediately successful
      int post() { return 0 ; }
      int wait() { return 0 ; }
      int trywait() { return 0 ; }
      int timedwait(const struct timespec* /*timeout*/) { return 0 ; }
      int timedwait(time_t /*until_sec*/, long /*until_nsec*/ = 0) { return 0 ; }
      // nobody is ever blocked on the semaphore
      int value() { return 1 ; }
   } ;

#else
class Semaphore
   {
   public:
      Semaphore(unsigned initval = 0) { (void)sem_init(&m_sem,0,initval) ; }
      ~Semaphore() { sem_destroy(&m_sem) ; }

      int post() { return sem_post(&m_sem) ; }
      int wait() { return sem_wait(&m_sem) ; }
      int trywait() { return sem_trywait(&m_sem) ; }
      int timedwait(const struct timespec* until)
         {
	 int status ;
	 while ((status = sem_timedwait(&m_sem,until)) == -1 && errno == EINTR)
	    ; // retry if interrupted by a signal handler
	 return status ;
	 }
      int timedwait(time_t until_sec, long until_nsec = 0)
         {
	 struct timespec until ;
	 until.tv_sec = until_sec ; until.tv_nsec = until_nsec ;
	 return timedwait(&until) ;
	 }
      //note: sem_getvalue is allowed to return 0 if anyone is blocked
      //  on the semaphore (Linux), or the negative of the number of
      //  waiters.  For consistency, we'll map everything to the former
      int value() { int val = 0 ; (void)sem_getvalue(&m_sem,&val) ; if (val < 0) val = 0 ; return val ; }
   protected:
      sem_t m_sem ;
   } ;
#endif

} // end namespace Fr

#endif /* !_Fr_SEMAPHORE_H_INCLUDED */

// end of file semaphore.h //
