/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-15					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#include <csignal>
#include "framepac/message.h"
#include "framepac/signal.h"

namespace Fr
{

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

// forward declaration
static void invoke_handler(int signum, int arg) ;

typedef void SigHandler(int) ;

#define defhandler(i) static void handler##i(int arg) { invoke_handler(i,arg) ; }

/************************************************************************/
/*	Global data for this module					*/
/************************************************************************/

defhandler(0) ;
defhandler(1) ;
defhandler(2) ;
defhandler(3) ;
defhandler(4) ;
defhandler(5) ;
defhandler(6) ;
defhandler(7) ;
defhandler(8) ;
defhandler(9) ;
defhandler(10) ;
defhandler(11) ;
defhandler(12) ;
defhandler(13) ;
defhandler(14) ;
defhandler(15) ;
defhandler(16) ;
defhandler(17) ;
defhandler(18) ;
defhandler(19) ;
defhandler(20) ;
defhandler(21) ;
defhandler(22) ;
defhandler(23) ;
defhandler(24) ;
defhandler(25) ;
defhandler(26) ;
defhandler(27) ;
defhandler(28) ;
defhandler(29) ;
defhandler(30) ;
defhandler(31) ;

static SigHandler* handlers[] =
   {
   handler0, handler1, handler2, handler3, handler4,
   handler5, handler6, handler7, handler8, handler9,
   handler10, handler11, handler12, handler13, handler14,
   handler15, handler16, handler17, handler18, handler19,
   handler20, handler21, handler22, handler23, handler24,
   handler25, handler26, handler27, handler28, handler29,
   handler30, handler31 } ;

SignalHandler* signal_handlers[lengthof(handlers)] ;
   
static volatile std::sig_atomic_t active_signal = 999 ;

/************************************************************************/
/*	Helper functions						*/
/************************************************************************/

static void invoke_handler(int signum, int arg)
{
   if ((size_t)signum < lengthof(signal_handlers) && signal_handlers[signum] &&
      signal_handlers[signum]->currentHandler())
      {
      active_signal = signal_handlers[signum]->signalNumber() ;
      SignalHandlerFn* fn = signal_handlers[signum]->currentHandler() ;
      fn(arg) ;
      }
   return ;
}

//----------------------------------------------------------------------------

[[gnu::noreturn]]
static void error(int arg)
{
   char errmsg[] = "received signal 00(00)" ;
   int signum = active_signal ;
   errmsg[17] += (signum/10) ;
   errmsg[18] += (signum%10) ;
   errmsg[20] += ((arg/10)%10) ;
   errmsg[21] += (arg%10) ;
   SystemMessage::fatal(errmsg) ;
   std::abort() ;
}

/************************************************************************/
/*	Methods for class SignalHandler					*/
/************************************************************************/

SignalHandler::SignalHandler(int signal_number, SignalHandlerFn* handler)
{
   m_handler = nullptr ;
   m_signum = signal_number ;
   this->set(handler) ;
   // insert ourselves into the array of handlers
   for (size_t i = 0 ; i < lengthof(signal_handlers) ; ++i)
      {
      if (signal_handlers[i])
	 continue ;
      signal_handlers[i] = this ;
      m_old_handler = (void*)std::signal(m_signum,handlers[i]) ;
      break ;
      }
   return ;
}

//----------------------------------------------------------------------------

SignalHandler::~SignalHandler()
{
   // reset the signal handler
   std::signal(m_signum,(__sighandler_t)m_old_handler) ;
   // and remove ourself from  the array of handlers
   for (size_t i = 0 ; i <  lengthof(signal_handlers) ; ++i)
      {
      if (signal_handlers[i] == this)
	 {
	 signal_handlers[i] = nullptr ;
	 break ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

SignalHandlerFn* SignalHandler::set(SignalHandlerFn* new_handler)
{
   if (new_handler == (SignalHandlerFn*)SIG_IGN)
      new_handler = nullptr ;
   else if (new_handler == (SignalHandlerFn*)SIG_ERR)
      new_handler = error ;
   SignalHandlerFn* old_handler = m_handler ;
   m_handler = new_handler ;
   return old_handler ;
}

//----------------------------------------------------------------------------

void SignalHandler::raise(int arg) const
{
   if (m_handler)
      m_handler(arg) ;
   return ;
}

//----------------------------------------------------------------------------


} // end of namespace Fr

// end of file signal.C //

