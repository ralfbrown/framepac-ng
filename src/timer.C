/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-26					*/
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
#include <cstdio>
#include <iostream>
#include "framepac/timer.h"
#include "framepac/texttransforms.h"

using namespace std ;

namespace Fr
{

/************************************************************************/
/*	Methods for class TimerBase					*/
/************************************************************************/

//----------------------------------------------------------------------------

bool TimerBase::elapsedTime(double elapsed, unsigned &days, unsigned &hours, unsigned &minutes, unsigned &secs,
			    double &fraction)
{
   fraction = elapsed - floor(elapsed) ;
   size_t whole_sec = (size_t)elapsed ;
   secs = whole_sec % 60 ;
   size_t whole_min = whole_sec / 60 ;
   minutes = whole_min % 60 ;
   size_t whole_hours = whole_min / 60 ;
   hours = whole_hours % 24 ;
   days = whole_hours / 24 ;
   return true ;
}

//----------------------------------------------------------------------------

CharPtr TimerBase::formattedTime(double elapsed, bool use_colons)
{
   unsigned days, hours, minutes, secs ;
   double frac ;
   CharPtr formatted ;
   if (elapsedTime(elapsed,days,hours,minutes,secs,frac))
      {
      // round fraction to milliseconds
      unsigned millisecs = (unsigned)((frac + 0.0005) * 1000) ;
      if (days)
	 {
	 formatted = aprintf(use_colons ? "%u:%02u:%02u:%02u.%03u" : "%ud%02uh%02um%02u.%03us",
			     days,hours,minutes,secs,millisecs) ;
	 }
      else if (hours)
	 {
	 formatted = aprintf(use_colons ? "%u:%02u:%02u.%03u" : "%uh%02um%02u.%03us",
			     hours,minutes,secs,millisecs) ;
	 }
      else if (minutes)
	 {
	 formatted = aprintf(use_colons ? "%02u:%02u.%03u" : "%um%02u.%03us",
			     minutes,secs,millisecs) ;
	 }
      else
	 {
	 formatted = aprintf(use_colons ? "00:%u.%03u" : "%u.%03us",
			     secs,millisecs) ;
	 }
      }
   return formatted ;
}

//----------------------------------------------------------------------------

std::ostream& TimerBase::formatTime(std::ostream& out, double elapsed, bool use_colons)
{
   CharPtr formatted = formattedTime(elapsed,use_colons) ;
   out << formatted ;
   return out ;
}

/************************************************************************/
/*	Methods for class CpuTimer					*/
/************************************************************************/

double CpuTimer::seconds() const
{
#ifdef CLOCK_PROCESS_CPUTIME_ID
   struct timespec now = currTime() ;
   time_t seconds = now.tv_sec - m_start_time.tv_sec ;
   long nanos = now.tv_nsec - m_start_time.tv_nsec ;
   if (nanos < 0)
      {
      seconds-- ;
      nanos += 1000000000L ;
      }
   return seconds + (nanos / 1E9) ;
#else
   std::clock_t now = currTime() ;
   return (now - m_start_time) / (double)(CLOCKS_PER_SEC) ;
#endif /* CLOCK_PROCESS_CPUTIME_ID */
}

//----------------------------------------------------------------------------

bool CpuTimer::elapsedTime(unsigned &days, unsigned &hours, unsigned &minutes, unsigned &secs,
			   double &fraction) const
{
   return elapsedTime(seconds(),days,hours,minutes,secs,fraction) ;
}

//----------------------------------------------------------------------------

CharPtr CpuTimer::formattedTime(bool use_colons) const
{
   return formattedTime(seconds(),use_colons) ;
}

/************************************************************************/
/*	Methods for class ElapsedTimer					*/
/************************************************************************/

double ElapsedTimer::seconds() const
{
   auto time_now = currTime() ;
   std::chrono::duration<double> elapsed = time_now - m_start ;
   return elapsed.count() ;
}

//----------------------------------------------------------------------------

bool ElapsedTimer::elapsedTime(unsigned &days, unsigned &hours, unsigned &minutes, unsigned &secs,
			       double &fraction) const
{
   return elapsedTime(seconds(),days,hours,minutes,secs,fraction) ;
}

//----------------------------------------------------------------------------

CharPtr ElapsedTimer::formattedTime(bool use_colons) const
{
   return formattedTime(seconds(),use_colons) ;
}

/************************************************************************/
/*	Methods for class Timer						*/
/************************************************************************/

void Timer::restart()
{
   m_cputime.restart() ;
   m_elapsed.restart() ;
   return ;
}

//----------------------------------------------------------------------------

std::ostream& Timer::showTimes(std::ostream& out) const
{
   double cpu_sec = m_cputime.seconds() ;
   double elapsed_sec = m_elapsed.seconds() ;
   double percentage = elapsed_sec ? (cpu_sec / elapsed_sec) : 100.0 ;
   // round CPU seconds and elapsed time to thousandths
   cpu_sec = ((size_t)(1000*cpu_sec + 0.5)) / 1000.0 ;
   elapsed_sec = ((size_t)(1000*elapsed_sec + 0.5)) / 1000.0 ;
   // round percent utilization to tenths (with a slight upward bias)
   percentage = ((size_t)(1000.0*percentage + 0.75)) / 10.0 ;
   // show the elapsed time, using XXhYYmZZs for longer times
   if (elapsed_sec < 6000) // 100 minutes
      {
      out << elapsed_sec <<"s" ;
      }
   else
      {
      TimerBase::formatTime(out,elapsed_sec,false) ;
      }
   out << " (" << cpu_sec << "s CPU, " << percentage << "%)" ;
   return out ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file elapsedtime.C //
