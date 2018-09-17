/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.12, last edit 2018-09-15					*/
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

#include <cstdlib>
#include <mutex>
#include "framepac/init.h"
#include "framepac/symboltable.h"
#ifdef __SANITIZE_THREAD__
#include "framepac/hashhelper.h"
#endif /* __SANITIZE_THREAD__ */

#ifndef FrSINGLE_THREADED
#include <thread>
#endif /* !FrSINGLE_THREADED */

namespace Fr
{

/************************************************************************/
/************************************************************************/

static bool initialized = false ;

//#ifndef FrSINGLE_THREADED
static InitializerBase* static_init_funcs ;
static InitializerBase* static_cleanup_funcs ;

static ThreadInitializerBase* registered_init_funcs ;
static ThreadInitializerBase* registered_cleanup_funcs ;

static std::mutex registry_lock ;
//#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

bool Initialize()
{
   if (!initialized)
      {
      initialized = true ;
#ifdef __SANITIZE_THREAD__
      // TSAN throws up warnings about races during thread shutdown if the hashtable helper thread is started
      //   from a threadpool worker thread, so start it now
      HashTableHelper::startHelper() ;
#endif /* __SANITIZE_THREAD__ */
//TODO: perform any other required FramepaC-ng initialization

      // install termination handler
      std::atexit(Shutdown) ;
      // call any registered static-initialization functions
      for (auto init = static_init_funcs ; init ; init = init->nextInit())
	 {
	 init->init() ;
	 }
      }
   // ensure that any needed thread registration is carried out on the original thread
   ThreadInit() ;
   return true ;
}

//----------------------------------------------------------------------------

void Shutdown()
{
   if (initialized)
      {
      // call any registered static-cleanup functions
      for (auto cln = static_cleanup_funcs ; cln ; cln = cln->nextCleanup())
	 {
	 cln->cleanup() ;
	 }
//TODO: perform any required FramepaC-ng cleanup

      initialized = false ;
      }
   return ;
}

//----------------------------------------------------------------------------

void RegisterStaticInit(InitializerBase *ti)
{
   ti->setNextInit(static_init_funcs) ;
   static_init_funcs = ti ;
   return ;
}

//----------------------------------------------------------------------------

void RegisterStaticCleanup(InitializerBase *ti)
{
   ti->setNextCleanup(static_cleanup_funcs) ;
   static_cleanup_funcs = ti ;
   return ;
}

//----------------------------------------------------------------------------

static thread_local bool thread_registered = false ;

bool ThreadInit()
{
   if (!thread_registered)
      {
      // call any registered thread-initialization functions
      for (auto ti = registered_init_funcs ; ti ; ti = ti->nextInit())
	 {
	 // do so under a lock so that the initialization function doesn't need to take any
	 //   precautions against reentrant calls
	 ScopedGlobalThreadLock guard ;
	 ti->init() ;
	 }
      thread_registered = true ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ThreadCleanup()
{
   if (thread_registered)
      {
      // invoke any registered thread-cleanup functions
      for (auto ti = registered_cleanup_funcs ; ti ; ti = ti->nextCleanup())
	 {
	 // do so under a lock so that the cleanup function doesn't need to take any
	 //   precautions against reentrant calls
	 ScopedGlobalThreadLock guard ;
	 ti->cleanup() ;
	 }
      thread_registered = false ;
      return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

void RegisterThreadInit(ThreadInitializerBase *ti)
{
   ScopedGlobalThreadLock guard ;
   ti->setNextInit(registered_init_funcs) ;
   registered_init_funcs = ti ;
   return ;
}

//----------------------------------------------------------------------------

void RegisterThreadCleanup(ThreadInitializerBase *ti)
{
   ScopedGlobalThreadLock guard ;
   ti->setNextCleanup(registered_cleanup_funcs) ;
   registered_cleanup_funcs = ti ;
   return ;
}

//----------------------------------------------------------------------------

void UnregisterThreadInit(ThreadInitializerBase *ti)
{
   ScopedGlobalThreadLock guard ;
   if (registered_init_funcs == ti)
      {
      registered_init_funcs = ti->nextInit() ;
      return ;
      }
   if (!registered_init_funcs)
      return;
   auto prev = registered_init_funcs ;
   while (auto next = prev->nextInit())
      {
      if (next == ti)
	 {
	 prev->setNextInit(ti->nextInit()) ;
	 break ;
	 }
      prev = next ;
      }
   return ;
}

//----------------------------------------------------------------------------

void UnregisterThreadCleanup(ThreadInitializerBase *ti)
{
   ScopedGlobalThreadLock guard ;
   if (registered_cleanup_funcs == ti)
      {
      registered_cleanup_funcs = ti->nextCleanup() ;
      return ;
      }
   if (!registered_cleanup_funcs)
      return;
   auto prev = registered_cleanup_funcs ;
   while (auto next = prev->nextCleanup())
      {
      if (next == ti)
	 {
	 prev->setNextCleanup(ti->nextCleanup()) ;
	 break ;
	 }
      prev = next ;
      }
   return ;
}

//----------------------------------------------------------------------------

void GlobalThreadLock()
{
#ifndef FrSINGLE_THREADED
   registry_lock.lock() ;
#endif /* !FrSINGLE_THREADED */
   return ;
}

//----------------------------------------------------------------------------

void GlobalThreadUnlock()
{
#ifndef FrSINGLE_THREADED
   registry_lock.unlock() ;
#endif /* !FrSINGLE_THREADED */
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file init.C //
