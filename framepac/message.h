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

#ifndef _Fr_MESSAGE_H_INCLUDED
#define _Fr_MESSAGE_H_INCLUDED

#include "framepac/config.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

class SystemMessage
   {
   public:
      SystemMessage() ;
      ~SystemMessage() ;

      SystemMessage& instance() ;
      bool setInstance(SystemMessage& inst) ;

      bool modal(const char* fmt, ...) ;
      bool confirmation(const char* fmt, ...) ;
      bool status(const char* fmt, ...) ;
      bool warning(const char* fmt, ...) ;
      bool error(const char* fmt, ...) ;
      bool fatal(const char* fmt, ...) ;
      bool prog_error(const char* fmt, ...) ;
      bool missed_case(const char* fmt, ...) ;
      bool no_memory(const char* msg) ;
   protected:
      virtual bool showModal(const char *msg) = 0 ;
      virtual bool showConfirmation(const char *msg) = 0 ;
      virtual bool showMessage(const char *msg) = 0 ;


      static SystemMessage& m_instance ;
   }

//----------------------------------------------------------------------------

class ConsoleSystemMessage
   {
   public:
      ConsoleSystemMessage() : SystemMessage() {}
      ~ConsoleSystemMessage() {}
   protected:
      virtual bool showModal(const char *msg) ;
      virtual bool showConfirmation(const char *msg) ;
      virtual bool showMessage(const char *msg) ;
      virtual bool showFatal(const char *msg) ;
   }

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_PROGRESS_H_INCLUDED */

// end of file progress.h //
