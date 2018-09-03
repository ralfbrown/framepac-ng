/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-09-01					*/
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

#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sys/ioctl.h>
#include <unistd.h>
#include "framepac/progress.h"
#include "framepac/timer.h"
#include "framepac/texttransforms.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class ProgressIndicator				*/
/************************************************************************/

ProgressIndicator::ProgressIndicator(size_t interval, size_t limit)
   : m_timer(new ElapsedTimer), m_limit(limit), m_interval(interval), m_count((size_t)0), m_prev_update((size_t)0),
     m_show_elapsed(false), m_show_estimated(true)
{
   return ;
}

//----------------------------------------------------------------------------

ProgressIndicator::~ProgressIndicator()
{
   finalize() ;
   delete m_timer ;
   return ;
}

//----------------------------------------------------------------------------

void ProgressIndicator::incr(size_t increment)
{
   size_t newcount = (m_count += increment) ;
   updateDisplay(newcount) ;
   return ;
}

//----------------------------------------------------------------------------

void ProgressIndicator::showElapsedTime(bool show)
{
   m_show_elapsed = show ;
   updateSettings() ;
   return ;
}

//----------------------------------------------------------------------------

void ProgressIndicator::showRemainingTime(bool show)
{
   m_show_estimated = show ;
   updateSettings() ;
   return ;
}

/************************************************************************/
/*	Methods for class ConsoleProgressIndicator			*/
/************************************************************************/

ConsoleProgressIndicator::ConsoleProgressIndicator(size_t interval, size_t limit, size_t per_line,
						   const char* first_prefix, const char* rest_prefix)
   : ProgressIndicator(interval,limit), m_prevfrac(0.0), m_lastupdate(0.0),
     m_firstprefix(dup_string(first_prefix)), m_restprefix(dup_string(rest_prefix)),
     m_per_line(per_line), m_linewidth(80), m_istty(isatty(fileno(stdout)))
{
   // get actual line width rather than always assuming an 80-column display
#ifdef TIOCGWINSZ
   struct winsize ws ;
   if (tty() && ioctl(fileno(stdout), TIOCGWINSZ, &ws) != -1 && ws.ws_col)
      {
      m_linewidth = ws.ws_col ;
      }
#endif /* TIOCGWINSZ */
   computeBarSize() ;
   return ;
}

//----------------------------------------------------------------------------

ConsoleProgressIndicator::~ConsoleProgressIndicator()
{
   cout << endl ;
   return ;
}

//----------------------------------------------------------------------------

void ConsoleProgressIndicator::computeBarSize()
{
   // figure out how many columns we have for the progress bar
   //   we can't use the very last column (may cause autowrap), we use
   //   two columns for the open/close brackets, and we need six
   //   columns each if displaying elapsed or estimated time
   size_t overhead = strlen(m_firstprefix) + 3 ;
   if (m_show_elapsed)
      overhead += 6 ;
   if (m_show_estimated)
      overhead += 6 ;
   if (overhead > m_linewidth)
      {
      m_show_elapsed = false ;
      overhead -= 6 ;
      }
   if (overhead > m_linewidth)
      {
      m_show_estimated = false ;
      overhead -= 6 ;
      }
   m_barsize = m_linewidth - overhead ;
   return ;
}

//----------------------------------------------------------------------------

void ConsoleProgressIndicator::updateSettings()
{
   computeBarSize() ;
   return ;
}

//----------------------------------------------------------------------------

static void display_time(double time)
{
   if (time <= 0)
      {
      cout << " --- " ;
      return ;
      }
   if (time <= 99)
      {
      cout << setw(4) << (unsigned)(time + 0.5) << 's' ;
      return  ;
      }
   if (time <= (99 * 60) + 59)
      {
      size_t minutes = (size_t)(time / 60) ;
      size_t seconds = (size_t)(time - 60 * minutes + 0.5) ;
      cout << setw(2) << minutes << ':' << (char)('0' + (seconds/10)) << (char)('0' + (seconds%10)) ;
      return ;
      }
   time = (time + 30) / 60 ;
   if (time <= (99 * 60) + 59)
      {
      size_t hours = (size_t)(time / 60) ;
      size_t minutes = (size_t)(time - 60 * hours + 0.5) ;
      cout << setw(2) << hours << 'h' << (char)('0' + (minutes/10)) << (char)('0' + (minutes%10)) ;
      return ;
      }
   time = (time + 30) / 60 ;
   if (time <= (99 * 24) + 23)
      {
      size_t days = (size_t)(time / 24) ;
      size_t hours = (size_t)(time - 24 * days + 0.5) ;
      cout << setw(2) << days << 'd' << (char)('0' + (hours/10)) << (char)('0' + (hours%10)) ;
      return ;
      }
   cout << " >>> " ;
   return ;
}

//----------------------------------------------------------------------------

void ConsoleProgressIndicator::updateDisplay(size_t curr_count)
{
   size_t intervals = (curr_count / m_interval) ;
   size_t prev_intervals = m_prev_update.exchange(intervals) ;
   if (intervals == prev_intervals)
      return ;
   
   if (m_limit == 0)
      {
      // we don't know what fraction has been completed, so just update progress
      //   by adding a period, wrapping after every 'per_line' 
      if (prev_intervals == 0)
	 cout << *m_firstprefix ;
      while (prev_intervals++ < intervals)
	 {
	 if (prev_intervals % m_per_line == 0)
	    {
	    cout << endl << *m_restprefix << (char)('0' + ((prev_intervals / m_per_line) % 10)) ;
	    }
	 cout << '.' << flush ;
	 }
      }
   else if (tty())
      {
      double elapsed { 0.0 } ;
      double estimated { 0.0 } ;
      double frac = (curr_count / (double)m_limit) ;
      size_t count = (size_t)(m_barsize * frac + 0.8) ;
      size_t prevcount = (size_t)(m_barsize * m_prevfrac + 0.8) ;
      if (m_show_elapsed || m_show_estimated)
	 {
	 elapsed = m_timer ? m_timer->seconds() : 0.0 ;
	 // don't update more often than every two seconds unless there has been
	 //   a substantial increase in the proportion completed, to avoid generating
	 //   a huge amount of output (and thus a huge file when capturing output)
	 if (elapsed && frac < 1.0)
	    {
	    if (frac < m_prevfrac + 0.01)
	       {
	       if (elapsed < m_lastupdate + 2)
		  return ;
	       }
	    else if (elapsed < m_lastupdate + 0.5)
	       return;
	    }
	 m_lastupdate = elapsed ;
	 if (m_show_estimated && elapsed >= 1.0 && frac > 0.01)
	    {
	    estimated = (elapsed / frac) - elapsed ;
	    }
	 }
      else if (count == prevcount)
	 return ;
      if (frac > 1.0)
	 {
	 // we've gone over 100% completion, so any time estimates become invalid
	 frac = 1.0 ;
	 if (m_prevfrac == 1.0)
	    {
	    m_show_estimated = false ;
	    m_show_elapsed = false ;
	    return ;
	    }
	 }
      m_prevfrac = frac ;
      cout << *m_firstprefix << '[' ;
      for (size_t i = 0 ; i < count ; ++i)
	 {
	 cout << '*' ;
	 }
      for (size_t i = count ; i < m_barsize ; ++i)
	 {
	 cout  << ' ' ;
	 }
      cout << ']' ;
      if (m_show_elapsed)
	 {
	 cout << ' ' ;
	 display_time(elapsed) ;
	 }
      if (m_show_estimated)
	 {
	 if (frac == 1.0)
	    cout << "      " ;		// wipe out the last estimate
	 else
	    {
	    cout << (m_show_elapsed ? '+' : ' ') ;
	    display_time(estimated) ;
	    }
	 }
      cout << '\r' << flush ;
      }
   else
      {
      // standard output is not a tty (i.e. it's been redirected), so just print up a line of 50
      //   asterisks as the progress bar
      double frac = (curr_count / (double)m_limit) ;
      size_t stars = round(50 * frac) ;
      size_t prevstars = round(50 * m_prevfrac) ;
      m_prevfrac = frac ;
      if (stars > prevstars)
	 {
	 if (prevstars == 0)
	    cout << *m_firstprefix << '[' ;
	 while (prevstars++ < stars)
	    cout << '*' ;
	 if (frac >= 1.0 && prevstars <= 50)
	    {
	    cout << ']' ;
	    if (m_show_elapsed)
	       {
	       double elapsed = m_timer ? m_timer->seconds() : 0.0 ;
	       cout << ' ' ;
	       display_time(elapsed) ;
	       }
	    }
	    cout << flush ;
	 }
      }
   return  ;
}

//----------------------------------------------------------------------------

void ConsoleProgressIndicator::finalize()
{
   if (!m_finalized)
      {
      cout << endl ;
      m_finalized = true ;
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file progress.C //
