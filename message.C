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

#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

static ConsoleSystemMessage default_console_message ;

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
   if (!m_instance)
      setInstance(default_console_message) ;
   return m_instance ;
}

//----------------------------------------------------------------------------

void SystemMessage::setInstance(SystemMessage& inst)
{
   delete m_instance ;
   m_instance = &inst ;
   return ;
}

//----------------------------------------------------------------------------

bool SystemMessage::modal(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::confirmation(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::status(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::warning(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::error(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::fatal(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::prog_error(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::missed_case(const char* fmt, ...)
{
   (void)fmt;
//FIXME
   return true ;
}

//----------------------------------------------------------------------------

bool SystemMessage::no_memory(const char* msg)
{
   (void)fmt;
//FIXME
   return true ;
}

/************************************************************************/
/*	Methods for class ConsoleSystemMessage
/************************************************************************/

bool ConsoleSystemMessage::showModal(const char* msg)
{
   if (!msg)
      return false ;

   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showConfirmation(const char* msg)
{
   if (!msg)
      return false ;

   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showMessage(const char* msg)
{
   if (!msg)
      return false ;

   return true ;
}

//----------------------------------------------------------------------------

bool ConsoleSystemMessage::showFatal(const char* msg)
{
   if (!msg)
      return false ;

   return true ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file vector_u32_dbl.C //
