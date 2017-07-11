/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-10					*/
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
#include <mutex>
#include "framepac/init.h"

#ifndef FrSINGLE_THREADED
#include <thread>
#endif /* !FrSINGLE_THREADED */

namespace Fr
{

/************************************************************************/
/************************************************************************/

static bool initialized = false ;

#ifndef FrSINGLE_THREADED
static InitializerBase* static_init_funcs ;
static InitializerBase* static_cleanup_funcs ;

static ThreadInitializerBase* registered_init_funcs ;
static ThreadInitializerBase* registered_cleanup_funcs ;

static std::mutex registry_lock ;
#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

bool Initialize()
{
   if (!initialized)
      {
      initialized = true ;
//TODO: perform any requred FramepaC-ng initialization

      // install termination handler
      std::atexit(Shutdown) ;
      // call any registered static-initialization functions
      for (InitializerBase* init = static_init_funcs ; init ; init = init->nextInit())
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
      for (InitializerBase* cln = static_cleanup_funcs ; cln ; cln = cln->nextCleanup())
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

#ifndef FrSINGLE_THREADED
static thread_local bool thread_registered = false ;
#endif /* !FrSINGLE_THREADED */

bool ThreadInit()
{
#ifdef FrSINGLE_THREADED
   return true ;
#else
   if (!thread_registered)
      {
      // call any registered thread-initialization functions
      for (ThreadInitializerBase* ti = registered_init_funcs ; ti ; ti = ti->nextInit())
	 {
	 ti->init() ;
	 }
      thread_registered = true ;
      }
#endif /* FrSINGLE_THREADED */
   return true ;
}

//----------------------------------------------------------------------------

bool ThreadCleanup()
{
   if (thread_registered)
      {
      // invoke any registered thread-cleanup functions
      for (ThreadInitializerBase* ti = registered_cleanup_funcs ; ti ; ti = ti->nextCleanup())
	 {
	 // do so under a lock so that the cleanup function doesn't need to take any
	 //   precautions against reentrant calls
	 std::lock_guard<std::mutex> guard(registry_lock) ;
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
   ti->setNextInit(registered_init_funcs) ;
   registered_init_funcs = ti ;
   return ;
}

//----------------------------------------------------------------------------

void RegisterThreadCleanup(ThreadInitializerBase *ti)
{
   ti->setNextCleanup(registered_cleanup_funcs) ;
   registered_cleanup_funcs = ti ;
   return ;
}

//----------------------------------------------------------------------------

void UnregisterThreadInit(ThreadInitializerBase *ti)
{
   std::lock_guard<std::mutex> guard(registry_lock) ;
   if (registered_init_funcs == ti)
      {
      registered_init_funcs = ti->nextInit() ;
      return ;
      }
   if (!registered_init_funcs)
      return;
   ThreadInitializerBase* prev = registered_init_funcs ;
   ThreadInitializerBase* next ;
   while ((next = prev->nextInit()) != nullptr)
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
   std::lock_guard<std::mutex> guard(registry_lock) ;
   if (registered_cleanup_funcs == ti)
      {
      registered_cleanup_funcs = ti->nextCleanup() ;
      return ;
      }
   if (!registered_cleanup_funcs)
      return;
   ThreadInitializerBase* prev = registered_cleanup_funcs ;
   ThreadInitializerBase* next ;
   while ((next = prev->nextCleanup()) != nullptr)
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

} // end namespace Fr

// end of file init.C //
