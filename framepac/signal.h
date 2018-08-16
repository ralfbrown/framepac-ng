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

#ifndef __FrSIGNAL_H_INCLUDED
#define __FrSIGNAL_H_INCLUDED

namespace Fr
{

typedef void SignalHandlerFn(int) ;

class SignalHandler
   {
   public:
      SignalHandler(int signal, SignalHandlerFn*) ;
      ~SignalHandler() ;

      SignalHandlerFn* set(SignalHandlerFn* new_handler) ;
      void raise(int arg) const ;

      // access to internal state
      int signalNumber() const { return m_signum ; }
      SignalHandlerFn* currentHandler() const { return m_handler ; }
      
   protected:
      void*            m_old_handler ;
      SignalHandlerFn* m_handler ;
      long             m_old_mask ;
      long             m_old_flags ;
      int              m_signum ;
   } ;

} // end namespace Fr


#endif /* !__FrSIGNAL_H_INCLUDED */

// end of file signal.h //
