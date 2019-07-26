/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-16					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018,2019 Carnegie Mellon University		*/
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

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include "framepac/cstring.h"
#include "framepac/message.h"
#include "framepac/texttransforms.h"

using namespace std ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

static ConsoleSystemMessage default_console_message ;

SystemMessage* SystemMessage::s_instance = &default_console_message ;

/************************************************************************/
/*	Methods for class SystemMessage					*/
/************************************************************************/

SystemMessage::SystemMessage()
{

   return ;
}

//----------------------------------------------------------------------------

SystemMessage::~SystemMessage()
{

   return ;
}

//----------------------------------------------------------------------------

SystemMessage& SystemMessage::instance()
{
   if (!s_instance)
      setInstance(default_console_message) ;
   return *s_instance ;
}

//----------------------------------------------------------------------------

bool SystemMessage::setInstance(SystemMessage& inst)
{
   s_instance = &inst ;
   return true ;
}

//----------------------------------------------------------------------------

#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"

bool SystemMessage::modal(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showModal(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::modal(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = modal(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::confirmation(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showConfirmation(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::confirmation(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = confirmation(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::status(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showMessage(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::status(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = status(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::warning(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showWarning(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::warning(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = warning(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::debug(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showDebug(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::debug(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = debug(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::error(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showError(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::error(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = error(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::fatal(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showFatal(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::fatal(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = fatal(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::nomemory(const char* msg)
{
   instance().showError(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::prog_error(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showFatal(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::prog_error(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = prog_error(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::missed_case(const char* fmt, va_list args)
{
   CharPtr msg { vaprintf(fmt,args) } ;
   instance().showFatal(msg) ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::missed_case(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   bool res = missed_case(fmt,args) ;
   va_end(args) ;
   return res ;
}

//----------------------------------------------------------------------------

bool SystemMessage::no_memory(const char* msg)
{
   char msgbuf[1000] ;
   snprintf(msgbuf,sizeof(msgbuf)-1,"OUT OF MEMORY: %s%c",msg,'\0') ;
   instance().showWarning(msgbuf) ;
   return true ;
}

/************************************************************************/
/*	Methods for class ConsoleSystemMessage				*/
/************************************************************************/

bool ConsoleSystemMessage::showModal(const char* msg)
{
   if (!msg)
      return false ;
   cerr << msg << endl ;
   cerr << "Press <Enter> to continue:" << flush  ;
   int c ;
   while ((c = fgetc(stdin)) != EOF && c != '\n')
      ; // loop until Enter pressed
   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showConfirmation(const char* msg)
{
   if (!msg)
      return false ;
   cerr << msg << endl ;
   int c ;
   while ((c = fgetc(stdin)) != EOF)
      {
      c = toupper(c) ;
      if (c == 'Y')
	 {
	 return true ;
	 }
      else if (c == 'N')
	 {
	 return false ;
	 }
      else
	 {
	 cerr << "Please answer Y or N." << endl ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showMessage(const char* msg)
{
   if (!msg)
      return false ;
   cerr << msg << endl ;
   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showWarning(const char* msg)
{
   if (!msg)
      return false ;
   cerr << "WARNING: " << msg << endl ;
   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showDebug(const char* msg)
{
   if (!msg)
      return false ;
   cerr << "DEBUG: " << msg << endl ;
   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showError(const char* msg)
{
   if (!msg)
      return false ;
   cerr << "ERROR: " << msg << endl ;
   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showFatal(const char* msg)
{
   if (!msg)
      return false ;
   cerr << "FATAL: " << msg << endl ;
   std::terminate() ;
   return false ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file message.C //
