/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-09					*/
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

class InitializerBase ;
class ThreadInitializerBase ;

typedef void ThreadInitFunc() ;
typedef void ThreadCleanupFunc() ;

void RegisterStaticInit(InitializerBase*) ;
void RegisterStaticCleanup(InitializerBase*) ;
void RegisterThreadInit(ThreadInitializerBase*) ;
void RegisterThreadCleanup(ThreadInitializerBase*) ;

/************************************************************************/
/************************************************************************/

/************************************************************************/
/*	global initializers			       			*/
/************************************************************************/

class InitializerBase
   {
   public:
      InitializerBase() {}
      InitializerBase(const InitializerBase&) = delete ;
      ~InitializerBase() = default ;
      void operator= (const InitializerBase&) = delete ;

      InitializerBase* nextInit() const { return m_next_init ; }
      void setNextInit(InitializerBase* nxt) { m_next_init = nxt ; }

      InitializerBase* nextCleanup() const { return m_next_cleanup ; }
      void setNextCleanup(InitializerBase* nxt) { m_next_cleanup = nxt ; }

      void init() { m_init() ; }
      void cleanup() { m_cleanup() ; }
   protected:
      InitializerBase* m_next_init { nullptr } ;
      InitializerBase* m_next_cleanup { nullptr } ;
      ThreadInitFunc*  m_init { nullptr } ;
      ThreadInitFunc*  m_cleanup { nullptr } ;
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
      int registerInit( decltype(T::StaticInitialization)* ) { return 0 ; }
      template <class T>
      char registerInit( T* )
	 { m_init = C::StaticInitialization ; RegisterStaticInit(this) ; return 0 ; }

      template <class T>
      int registerCleanup( decltype(T::StaticCleanup)* ) { return 0 ; }
      template <class T>
      char registerCleanup( T* )
	 { m_cleanup = C::StaticCleanup ; RegisterStaticCleanup(this) ; return 0 ; }
   public:
      Initializer() : InitializerBase()
	 {
	 registerInit<hasInit>(nullptr) ;
	 registerCleanup<hasCleanup>(nullptr) ; 
	 }
      ~Initializer() {}
   } ;

/************************************************************************/
/*	per-thread initializers						*/
/************************************************************************/

class ThreadInitializerBase
   {
   public:
      ThreadInitializerBase() {}
      ThreadInitializerBase(const InitializerBase&) = delete ;
      ~ThreadInitializerBase() = default ;
      void operator= (const ThreadInitializerBase&) = delete ;

      ThreadInitializerBase* nextInit() const { return m_next_init ; }
      void setNextInit(ThreadInitializerBase* nxt) { m_next_init = nxt ; }

      ThreadInitializerBase* nextCleanup() const { return m_next_cleanup ; }
      void setNextCleanup(ThreadInitializerBase* nxt) { m_next_cleanup = nxt ; }

      void init() { m_init() ; }
      void cleanup() { m_cleanup() ; }
   protected:
      ThreadInitializerBase* m_next_init { nullptr } ;
      ThreadInitializerBase* m_next_cleanup { nullptr } ;
      ThreadInitFunc*        m_init { nullptr } ;
      ThreadInitFunc*        m_cleanup { nullptr } ;
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
      int registerInit( decltype(T::threadInit)* ) { return 0 ; }
      template <class T>
      char registerInit( T* )
	 { m_init = C::threadInit ; RegisterThreadInit(this) ; return 0 ; }

      template <class T>
      int registerCleanup( decltype(T::threadCleanup)* ) { return 0 ; }
      template <class T>
      char registerCleanup( T* )
	 { m_cleanup = C::threadCleanup ; RegisterThreadCleanup(this) ; return 0 ; }
   public:
      ThreadInitializer() : ThreadInitializerBase()
	 {
	    registerInit<hasInit>(nullptr) ;
	    registerCleanup<hasCleanup>(nullptr) ;
	 }
      ~ThreadInitializer() {}
   } ;

//----------------------------------------------------------------------------

} ; // end namespace Fr

#endif /* !__FrINIT_H_INCLUDED */

// end of file init.h //
