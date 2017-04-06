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

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

static ConsoleSystemMessage default_console_message ;

SystemMessage* SystemMessage::s_instance = &default_console_message ;

/************************************************************************/
/************************************************************************/

[[gnu::format(gnu_printf,1,0)]]
static char* vaprintf(const char *fmt, va_list args)
{
   // we need to make a copy of the arglist, because va_lists are passed by
   //   reference, so the first call to vsprintf would clobber 'args' for
   //   the second call!
   va_list argcopy ;
   va_copy(argcopy,args) ;
   size_t len = vsnprintf(nullptr,0,fmt,argcopy) ;
   char* buf = new char[len+1] ;
   if (buf)
      vsnprintf(buf,len+1,fmt,args) ;
   return buf ;
}

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

bool SystemMessage::modal(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showModal(msg) ;
   delete [] msg ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::confirmation(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showConfirmation(msg) ;
   delete [] msg ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::status(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showMessage(msg) ;
   delete [] msg ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::warning(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showWarning(msg) ;
   delete [] msg ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::error(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showError(msg) ;
   delete [] msg ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::fatal(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showFatal(msg) ;
   delete [] msg ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::prog_error(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showFatal(msg) ;
   delete [] msg ;
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::missed_case(const char* fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   char* msg = vaprintf(fmt,args) ;
   va_end(args) ;
   instance().showFatal(msg) ;
   delete [] msg ;
   return true ;
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
   //TODO: wait for user to acknowledge
   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showConfirmation(const char* msg)
{
   if (!msg)
      return false ;
   cerr << msg << endl ;
   //TODO: wait for user confirmation (Y/N)
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

// end of file vector_u32_dbl.C //
