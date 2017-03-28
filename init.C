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

#include <cstdlib>
#include "framepac/init.h"

#ifndef FrSINGLE_THREADED
# ifdef __STDC_NO_THREADS__
// fall back to PThreads if no C11-compliant threads
// for a possible Win32 implementation of C11 threads, see
// https://github.com/glfw/glfw/blob/master/deps/tinycthread.h
#  include <errno.h>
#  include <pthread.h>
# else
#  include <threads.h>
# endif
#endif /* !FrSINGLE_THREADED */

namespace Fr
{

/************************************************************************/
/************************************************************************/

static bool initialized = false ;
#ifndef FrSINGLE_THREADED
 static bool  have_thread_cleanup ;
# ifdef __STDC_NO_THREADS__
// fall back to PThreads
 static pthread_key_t cleanup_key ;
# else
 static tss_t cleanup_key ;
# endif /* __STDC_NO_THREADS__ */
#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

void Shutdown()
{
   if (initialized)
      {
//FIXME

      initialized = false ;
      }
   return ;
}

//----------------------------------------------------------------------------

static void thread_cleanup(void*)
{
   // call any registered thread-cleanup functions
   //FIXME
   return ;
}

//----------------------------------------------------------------------------

static void register_thread_cleanup()
{
#ifndef FrSINGLE_THREADED
# ifdef __STDC_NO_THREADS__
   // fall back to PThreads
   have_thread_cleanup = (0 == pthread_key_create(&cleanup_key, thread_cleanup)) ;
//FIXME
# else
   have_thread_cleanup = (thrd_success == tss_create(&cleanup_key, thread_cleanup)) ;
# endif /* !__STDC_NO_THREADS__ */
#endif /* !FrSINGLE_THREADED */
   return ;
}

//----------------------------------------------------------------------------

bool Initialize()
{
   if (!initialized)
      {
      initialized = true ;
      register_thread_cleanup() ;
//FIXME
      // install termination handler
      std::atexit(Shutdown) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ThreadInit()
{
#ifdef FrSINGLE_THREADED
   return true ;
#else
   void *value = (void*)1 ; //FIXME
# ifdef __STDC_NO_THREADS__
   //FIXME: fall back on PThreads
   errno = pthread_setspecific(cleanup_key,value) ;
   return errno == 0 ;
# else
   if (have_thread_cleanup)
      {
      tss_set(cleanup_key, value) ;
      }
   return true ;
# endif /* __STDC_NO_THREADS__ */
   // call any registered thread-initialization functions
//FIXME
#endif /* FrSINGLE_THREADED */
}

//----------------------------------------------------------------------------

ThreadInitHandle *RegisterThreadInit(ThreadInitFunc *, void *)
{

   return nullptr ;
}

//----------------------------------------------------------------------------

ThreadCleanupHandle *RegisterThreadCleanup(ThreadInitFunc *, void *)
{

   return nullptr ;
}

//----------------------------------------------------------------------------

bool UnregisterThreadInit(ThreadInitHandle*)
{

   return true ;
}

//----------------------------------------------------------------------------

bool UnregisterThreadCleanup(ThreadCleanupHandle*)
{

   return true ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file init.C //
