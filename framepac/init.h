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

#ifndef __FrINIT_H_INCLUDED
#define __FrINIT_H_INCLUDED

namespace Fr {

// Initialize the FramepaC system; must be called before any other use of FramepaC
//   library code
bool Initialize() ;

// Clean up after using FramepaC, such as during abnormal program exit (Initialize()
//   enqueues an atexit handler to perform the equivalent action on normal termination).
// It is not valid to call any FramepaC function for the remainder of the program run
//   after calling Shutdown().
void Shutdown() ;

bool ThreadInit() ;
bool ThreadCleanup() ;

//----------------------------------------------------------------------------

typedef void (*ThreadInitFunc)(void*) ;
typedef void (*ThreadCleanupFunc)(void*) ;
class ThreadInitHandle ;
class ThreadCleanupHandle ;

ThreadInitHandle *RegisterThreadInit(ThreadInitFunc *, void *user_data) ;
bool UnregisterThreadInit(ThreadInitHandle*) ;

ThreadCleanupHandle *RegisterThreadCleanup(ThreadCleanupFunc *, void *user_data) ;
bool UnregisterThreadCleanup(ThreadCleanupHandle*) ;

} ; // end of namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

/************************************************************************/
/*	global initializers			       			*/
/************************************************************************/

class InitializerBase
   {
   public:
      InitializerBase() : m_next(nullptr) {}
      InitializerBase(const InitializerBase&) = delete ;
      ~InitializerBase() = default ;
      void operator= (const InitializerBase&) = delete ;

   protected:
      InitializerBase *m_next ;
   } ;

//----------------------------------------------------------------------------
// Register initialization and/or cleanup functions for a class, to be called
//   by Fr::Initialize() and Fr::Shutdown()
//
// using a version of the Member_Detector idiom from
//    https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector

template <class C>
class Initializer : InitializerBase
   {
   private:
      struct addInit { int StaticInitialization ; } ;
      struct addCleanup { int StaticCleanup ; } ;
      struct hasInit : C, addInit {} ;
      struct hasCleanup : C, addCleanup {} ;

      template <class T>
      static int callInit( decltype(T::StaticInitialization)* ) { return 0 ; }
      template <class T>
      static char callInit( T* ) { C::StaticInitialization() ; return 0 ; }

      template <class T>
      static int callCleanup( decltype(T::StaticCleanup)* ) { return 0 ; }
      template <class T>
      static char callCleanup( T* ) { C::StaticCleanup() ; return 0 ; }
   public:
      Initializer() : InitializerBase() {}
      ~Initializer() { cleanup() ; }

      void init() { (void)callInit<hasInit>(nullptr) ; }
      void cleanup() { (void)callCleanup<hasCleanup>(nullptr) ; }
   } ;

/************************************************************************/
/*	per-thread initializers						*/
/************************************************************************/

class ThreadInitializerBase
   {
   public:
      ThreadInitializerBase() : m_next(nullptr) {}
      ThreadInitializerBase(const InitializerBase&) = delete ;
      ~ThreadInitializerBase() = default ;
      void operator= (const ThreadInitializerBase&) = delete ;

   protected:
      ThreadInitializerBase *m_next ;
   } ;

//----------------------------------------------------------------------------
// Register initialization and/or cleanup functions for a class, to be called
//   by Fr::ThreadInit() and Fr::ThreadCleanup()
//
// using a version of the Member_Detector idiom from
//    https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector

template <class C>
class ThreadInitializer : ThreadInitializerBase
   {
   private:
      struct addInit { int threadInit ; } ;
      struct addCleanup { int threadCleanup ; } ;
      struct hasInit : C, addInit {} ;
      struct hasCleanup : C, addCleanup {} ;

      template <class T>
      static int callInit( decltype(T::threadInit)* ) { return 0 ; }
      template <class T>
      static char callInit( T* ) { C::threadInit() ; return 0 ; }

      template <class T>
      static int callCleanup( decltype(T::threadCleanup)* ) { return 0 ; }
      template <class T>
      static char callCleanup( T* ) { C::threadCleanup() ; return 0 ; }
   public:
      ThreadInitializer() : ThreadInitializerBase() {}
      ~ThreadInitializer() { cleanup() ; }

      void init() { (void)callInit<hasInit>(nullptr) ; }
      void cleanup() { (void)callCleanup<hasCleanup>(nullptr) ; }
   } ;

//----------------------------------------------------------------------------

} ; // end namespace FramepaC

#endif /* !__FrINIT_H_INCLUDED */

// end of file init.h //
