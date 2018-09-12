/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-17					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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
#include "framepac/cstring.h"
#include "framepac/timer.h"

namespace Fr
{

// forward declaration
class ElapsedTimer  ;

/************************************************************************/
/************************************************************************/

class ProgressIndicator
   {
   public:
      ProgressIndicator(size_t interval, size_t limit) ;
      virtual ~ProgressIndicator() ;
      ProgressIndicator(const ProgressIndicator&) = delete ;
      ProgressIndicator& operator= (const ProgressIndicator&) = delete ;

      void free() { delete this ; }

      virtual void finalize() { return ; }

      static CharPtr timeString(double time) ;

      // configuration
      void showElapsedTime(bool show) ;
      void showRemainingTime(bool show) ;

      void incr(size_t add = 1) ;
      ProgressIndicator& operator+= (size_t add) { incr(add) ; return *this ; }
      ProgressIndicator& operator++ () { incr() ; return *this ; }

   protected:
      virtual void updateSettings() { return ; }
      virtual void updateDisplay(size_t curr) = 0 ;

   protected:
      ElapsedTimer    m_timer ;
      double          m_lastupdate { 0.0 } ;	// time of last display update
      size_t          m_limit ;
      size_t          m_interval ;
      Atomic<size_t>  m_count ;
      Atomic<size_t>  m_prev_update ;
      bool            m_show_elapsed ;
      bool	      m_show_estimated ;
      bool	      m_finalized { false } ;
   } ;

//----------------------------------------------------------------------------

class NullProgressIndicator : public ProgressIndicator
   {
   public:
      typedef ProgressIndicator super ;
   public:
      NullProgressIndicator() : super(0,0)
	 {
	    showElapsedTime(false) ;
	    showRemainingTime(false) ;
	 }
      virtual ~NullProgressIndicator() {}
      NullProgressIndicator(const NullProgressIndicator&) = delete ;
      NullProgressIndicator& operator= (const NullProgressIndicator&) = delete ;

      NullProgressIndicator& operator+= (size_t add) { incr(add) ; return *this ; }
      NullProgressIndicator& operator++ () { incr() ; return *this ; }

   protected:
      virtual void updateDisplay(size_t) { return ; }
      virtual void finalize() { return ; }

   protected:

   } ;

//----------------------------------------------------------------------------

class ConsoleProgressIndicator : public ProgressIndicator
   {
   public:
      typedef ProgressIndicator super ;
   public:
      ConsoleProgressIndicator(size_t interval, size_t limit, size_t per_line,
			       const char *first_prefix, const char *rest_prefix) ;
      virtual ~ConsoleProgressIndicator() ;
      ConsoleProgressIndicator(const ConsoleProgressIndicator&) = delete ;
      ConsoleProgressIndicator& operator= (const ConsoleProgressIndicator&) = delete ;

      ConsoleProgressIndicator& operator+= (size_t add) { incr(add) ; return *this ; }
      ConsoleProgressIndicator& operator++ () { incr() ; return *this ; }

      bool tty() const { return m_istty ; }

   protected:
      void computeBarSize() ;
      void displayProgressBar(size_t stars, double elapsed, double estimated) const ;
      virtual void updateSettings() ;
      virtual void updateDisplay(size_t curr) ;
      virtual void finalize() ;

   protected:
      double  m_prevfrac ;		// completion fraction at last incr() call
      CharPtr m_firstprefix ;		// prefix to show on one-line display or first line of multiline display
      CharPtr m_restprefix ;		// prefix to show on subsequent lines of multi-line display
      size_t  m_per_line ;		// dots per line for multi-line display
      size_t  m_linewidth ;		// console width
      size_t  m_barsize ;		// number of stars in progress bar
      char    m_star { '*' } ;		// character to display in progress bar
      bool    m_istty ;			// is output a console?
      mutable bool m_showed_estimated ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_PROGRESS_H_INCLUDED */

// end of file progress.h //
