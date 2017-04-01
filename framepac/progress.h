/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-31					*/
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

#ifndef _Fr_PROGRESS_H_INCLUDED
#define _Fr_PROGRESS_H_INCLUDED

#include "framepac/atomic.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

class ProgressIndicator
   {
   public:
      ProgressIndicator(size_t interval, size_t limit) ;
      virtual ~ProgressIndicator() ;
      ProgressIndicator(const ProgressIndicator&) = delete ;
      ProgressIndicator& operator= (const ProgressIndicator&) = delete ;

      // configuration
      void showElapsedTime(bool show) { m_show_elapsed = show ; }
      void showRemainingTime(bool show) { m_show_estimated = show ; }

      void incr(size_t add = 1) ;
      ProgressIndicator& operator+= (size_t add) { incr(add) ; return *this ; }
      ProgressIndicator& operator++ () { incr() ; return *this ; }

   protected:
      virtual void updateDisplay(size_t curr) = 0 ;

   protected:
      class ElapsedTimer* m_timer ;
      size_t          m_limit ;
      size_t          m_interval ;
      Atomic<size_t>  m_count ;
      Atomic<size_t>  m_prev_update ;
      bool            m_show_elapsed ;
      bool	      m_show_estimated ;
   } ;

//----------------------------------------------------------------------------

class NullProgressIndicator : public ProgressIndicator
   {
   public:
      NullProgressIndicator() : ProgressIndicator(0,0) {}
      virtual ~NullProgressIndicator() {}
      NullProgressIndicator(const NullProgressIndicator&) = delete ;
      NullProgressIndicator& operator= (const NullProgressIndicator&) = delete ;

      NullProgressIndicator& operator+= (size_t add) { incr(add) ; return *this ; }
      NullProgressIndicator& operator++ () { incr() ; return *this ; }

   protected:
      virtual void updateDisplay(size_t) { return ; }

   protected:

   } ;

//----------------------------------------------------------------------------

class ConsoleProgressIndicator : public ProgressIndicator
   {
   public:
      ConsoleProgressIndicator(size_t interval, size_t limit, size_t per_line,
			       const char *first_prefix, const char *rest_prefix) ;
      virtual ~ConsoleProgressIndicator() ;
      ConsoleProgressIndicator(const ConsoleProgressIndicator&) = delete ;
      ConsoleProgressIndicator& operator= (const ConsoleProgressIndicator&) = delete ;

      ConsoleProgressIndicator& operator+= (size_t add) { incr(add) ; return *this ; }
      ConsoleProgressIndicator& operator++ () { incr() ; return *this ; }

   protected:
      virtual void updateDisplay(size_t curr) ;

   protected:
      double m_prevfrac ;
      char*  m_firstprefix ;
      char*  m_restprefix ;
      size_t m_per_line ;
      size_t m_linewidth ;
      size_t m_lastupdate ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_PROGRESS_H_INCLUDED */

// end of file progress.h //
