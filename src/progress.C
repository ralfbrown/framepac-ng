/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.12, last edit 2018-09-12					*/
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
#include "framepac/stringbuilder.h"
#include "framepac/texttransforms.h"

namespace Fr
{

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#define MAX_STARS 50

/************************************************************************/
/*	Methods for class ProgressIndicator				*/
/************************************************************************/

ProgressIndicator::ProgressIndicator(size_t interval, size_t limit)
   : m_limit(limit), m_interval(interval), m_count((size_t)0),
     m_show_elapsed(false), m_show_estimated(true)
{
   return ;
}

//----------------------------------------------------------------------------

ProgressIndicator::~ProgressIndicator()
{
   finalize() ;
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

//----------------------------------------------------------------------------

CharPtr ProgressIndicator::timeString(double time)
{
   if (time <= 0)
      {
      return dup_string(" --- ") ;
      }
   if (time <= 99)
      {
      return aprintf("%4us",(unsigned)(time + 0.5)) ;
      }
   if (time <= (99 * 60) + 59)
      {
      unsigned minutes = (unsigned)(time / 60) ;
      unsigned seconds = (unsigned)(time - 60 * minutes + 0.5) ;
      return aprintf("%2u:%02u",minutes,seconds) ;
      }
   time = (time + 30) / 60 ;
   if (time <= (99 * 60) + 59)
      {
      unsigned hours = (unsigned)(time / 60) ;
      unsigned minutes = (unsigned)(time - 60 * hours + 0.5) ;
      return aprintf("%2u:%02u",hours,minutes) ;
      }
   time = (time + 30) / 60 ;
   if (time <= (99 * 24) + 23)
      {
      unsigned days = (unsigned)(time / 24) ;
      unsigned hours = (unsigned)(time - 24 * days + 0.5) ;
      return aprintf("%2ud%02u",days,hours) ;
      }
   return dup_string(" >>> ") ;
}

/************************************************************************/
/*	Methods for class ConsoleProgressIndicator			*/
/************************************************************************/

ConsoleProgressIndicator::ConsoleProgressIndicator(size_t interval, size_t limit, size_t per_line,
						   const char* first_prefix, const char* rest_prefix)
   : ProgressIndicator(interval,limit),
     m_firstprefix(dup_string(first_prefix)),
     m_restprefix(dup_string(rest_prefix)),
     m_per_line(per_line),
     m_istty(isatty(fileno(stdout)))
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
   m_showed_estimated = m_show_estimated ;
   return ;
}

//----------------------------------------------------------------------------

ConsoleProgressIndicator::~ConsoleProgressIndicator()
{
   Update lastupdate = lastUpdate() ;
   refreshUpdate(lastupdate,-1,lastupdate.percent) ; // force display even if rate-limit time has not elapsed
   m_count = m_limit ;
   m_show_estimated = false ;
   updateDisplay(m_count) ;
   if (m_limit && !tty())
      {
      cout << ']' ;
      if (m_show_elapsed)
	 {
	 cout << ' ' << timeString(m_timer.seconds()) ;
	 m_show_elapsed = false ;
	 }
      }
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
   if (!tty())
      m_show_estimated = false ;
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
   m_barsize = tty() ? (m_linewidth - overhead) : MAX_STARS ;
   return ;
}

//----------------------------------------------------------------------------

void ConsoleProgressIndicator::updateSettings()
{
   computeBarSize() ;
   return ;
}

//----------------------------------------------------------------------------

void ConsoleProgressIndicator::displayProgressBar(size_t stars, double elapsed, double estimated) const
{
   if (stars > m_barsize)
      stars %= m_barsize ;
   // build up the progress bar so that we can output it all in one call, reducing the chance of
   //  multithread conflicts
   StringBuilder sb ;
   sb += *m_firstprefix ;
   sb += '[' ;
   sb.append(m_star,stars) ;
   sb.append(' ',m_barsize - stars) ;
   sb += ']' ;
   if (m_show_elapsed)
      {
      sb += ' ' ;
      sb += timeString(elapsed) ;
      }
   if (m_show_estimated)
      {
      sb +=  (m_show_elapsed ? '+' : ' ') ;
      sb += timeString(estimated) ;
      }
   else if (m_showed_estimated)
      {
      sb += "      " ;		// wipe out the last estimate
      m_showed_estimated = false ;
      }
   sb += '\r' ;
   sb += '\0' ;
   cout << sb.currentBuffer() << flush ;
   return ;
}

//----------------------------------------------------------------------------

void ConsoleProgressIndicator::updateDisplay(size_t curr_count)
{
   if (m_limit == 0)
      {
      if (m_firstupdate.exchange(false))
	 {
	 cout << *m_firstprefix << flush ;
	 }
      size_t intervals = (curr_count / m_interval) ;
      size_t prev_intervals = (size_t)refreshPercent(intervals) ;
      // if no new dots to print, we can return immediately
      if (intervals == prev_intervals)
	 return ;
      // we don't know what fraction has been completed, so just update progress
      //   by adding a period, wrapping after every 'per_line' 
      while (prev_intervals < intervals)
	 {
	 if (prev_intervals && prev_intervals % m_per_line == 0)
	    {
	    cout << endl << *m_restprefix << (char)('0' + ((prev_intervals / m_per_line) % 10)) ;
	    }
	 cout << '.' << flush ;
	 ++prev_intervals ;
	 }
      return ;
      }
   double frac = (curr_count / (double)m_limit) ;
   if (frac > 1.0)
      {
      // we've gone over 100% completion, so any time estimates become invalid
      m_show_estimated = false ;
      m_star = '+' ;
      }
   double stars = round(m_barsize * frac) ;
   Update lastupdate = lastUpdate() ;
   double prevstars = round(m_barsize * lastupdate.percent) ;
   if (tty())
      {
      if (prevstars == 0 || stars > prevstars || m_show_estimated || m_show_elapsed)
	 {
	 double elapsed = m_timer.seconds() ;
	 // prevent some race conditions by not updating more than 3 times / second
	 //   (plus it would just be too frenetic for the user)
	 if (elapsed - lastupdate.time < 0.34)
	    return ;
	 if (frac < 1.0)			// apply rate limiting except at the 100% mark
	    {
	    // don't update more often than every two seconds unless there has been
	    //   a substantial increase in the proportion completed, to avoid generating
	    //   a huge amount of output (and thus a huge file when capturing output)
	    if (stars <= prevstars && elapsed < lastupdate.time + 2)
	       return ;
	    }
	 double estimated { 0.0 } ;
	 if (m_show_estimated && elapsed * frac > 0.01)
	    {
	    estimated = (elapsed / frac) - elapsed ;
	    }
	 else if (!m_show_elapsed && stars == prevstars && frac < 1.0)
	    return ;
	 if (refreshUpdate(lastupdate,elapsed,frac))
	    {
	    displayProgressBar(stars,elapsed,estimated) ;
	    }
	 }
      }
   else
      {
      if (m_firstupdate.exchange(false))
	 {
	 cout << *m_firstprefix << '[' << flush ;
	 }
      // standard output is not a tty (i.e. it's been redirected), so just print up a line of MAX_STARS
      //   asterisks as the progress bar
      if (stars > prevstars)
	 {
	 double elapsed = m_timer.seconds() ;
	 if (!refreshUpdate(lastupdate,elapsed,frac))
	    return ;			// another thread has already made an update
	 while (prevstars++ < stars)
	    cout << m_star ;
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
