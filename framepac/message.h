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

#ifndef _Fr_MESSAGE_H_INCLUDED
#define _Fr_MESSAGE_H_INCLUDED

#include <cstdarg>
#include "framepac/config.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

#define _attr_printf0_ [[gnu::format(gnu_printf,1,0)]]
#define _attr_printf_ [[gnu::format(gnu_printf,1,2)]]

class SystemMessage
   {
   public:
      SystemMessage() ;
      virtual ~SystemMessage() ;

      static SystemMessage& instance() ;
      static bool setInstance(SystemMessage& inst) ;

      _attr_printf_ static bool modal(const char* fmt, ...) ;
      static bool modal(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool confirmation(const char* fmt, ...) ;
      static bool confirmation(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool status(const char* fmt, ...) ;
      static bool status(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool warning(const char* fmt, ...) ;
      static bool debug(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool debug(const char* fmt, ...) ;
      static bool warning(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool error(const char* fmt, ...) ;
      static bool error(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool fatal(const char* fmt, ...) ;
      static bool fatal(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool prog_error(const char* fmt, ...) ;
      static bool prog_error(const char* fmt, std::va_list args) ;
      _attr_printf_ static bool missed_case(const char* fmt, ...) ;
      static bool missed_case(const char* fmt, std::va_list args) ;
      _attr_printf0_ static bool nomemory(const char* msg) ;
      static bool no_memory(const char* msg) ;
   protected:
      virtual bool showModal(const char* msg) = 0 ;
      virtual bool showConfirmation(const char* msg) = 0 ;
      virtual bool showMessage(const char* msg) = 0 ;
      virtual bool showWarning(const char* msg) = 0 ;
      virtual bool showDebug(const char* msg) = 0 ;
      virtual bool showError(const char* msg) = 0 ;
      virtual bool showFatal(const char* msg) = 0 ;

      static SystemMessage* s_instance ;
   } ;

#undef _attr_printf_

//----------------------------------------------------------------------------

class ConsoleSystemMessage : public SystemMessage
   {
   public: // types
      typedef SystemMessage super ;
   public:
      ConsoleSystemMessage() : super() {}
      virtual ~ConsoleSystemMessage() {}
   protected:
      virtual bool showModal(const char *msg) ;
      virtual bool showConfirmation(const char *msg) ;
      virtual bool showMessage(const char *msg) ;
      virtual bool showWarning(const char *msg) ;
      virtual bool showDebug(const char* msg) ;
      virtual bool showError(const char *msg) ;
      virtual bool showFatal(const char *msg) ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_PROGRESS_H_INCLUDED */

// end of file progress.h //
