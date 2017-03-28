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

#include <cmath>
#include <cstdio>
#include "framepac/timer.h"

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

char *TimerBase::formattedTime(double elapsed, bool use_colons)
{
   unsigned days, hours, minutes, secs ;
   double frac ;
   char *formatted = nullptr ;
   if (elapsedTime(elapsed,days,hours,minutes,secs,frac))
      {
      bool success = false ;
      // round fraction to milliseconds
      unsigned millisecs = (unsigned)((frac + 0.0005) * 1000) ;
      if (days)
	 {
	 success = asprintf(&formatted,
			    use_colons ? "%u:%02u:%02u:%02u.%03u" : "%ud%02uh%02um%02u.%03us",
			    days,hours,minutes,secs,millisecs) > 0 ;
	 }
      else if (hours)
	 {
	 success = asprintf(&formatted,
			    use_colons ? "%u:%02u:%02u.%03u" : "%uh%02um%02u.%03us",
			    hours,minutes,secs,millisecs) > 0 ;
	 }
      else if (minutes)
	 {
	 success = asprintf(&formatted,
			    use_colons ? "%02u:%02u.%03u" : "%um%02u.%03us",
			    minutes,secs,millisecs) > 0 ;
	 }
      else
	 {
	 success = asprintf(&formatted,
			    use_colons ? "00:%u.%03u" : "%u.%03us",
			    secs,millisecs) > 0 ;
	 }
      if (!success)
	 formatted = nullptr ;
      }
   return formatted ;
}

//----------------------------------------------------------------------------

std::ostream& TimerBase::formatTime(std::ostream& out, double elapsed, bool use_colons)
{
   char *formatted = formattedTime(elapsed,use_colons) ;
   out << formatted ;
   free(formatted) ;
   return out ;
}

/************************************************************************/
/*	Methods for class CpuTimer					*/
/************************************************************************/

double CpuTimer::seconds() const
{
#ifdef _POSIX_CPUTIME
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
#endif /* _POSIX_CPUTIME */
}

//----------------------------------------------------------------------------

bool CpuTimer::elapsedTime(unsigned &days, unsigned &hours, unsigned &minutes, unsigned &secs,
			   double &fraction) const
{
   return elapsedTime(seconds(),days,hours,minutes,secs,fraction) ;
}

//----------------------------------------------------------------------------

char *CpuTimer::formattedTime(bool use_colons) const
{
   return formattedTime(seconds(),use_colons) ;
}

/************************************************************************/
/*	Methods for class ElapsedTimer					*/
/************************************************************************/

double ElapsedTimer::seconds() const
{
   auto time_now = std::chrono::steady_clock::now() ;
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

char *ElapsedTimer::formattedTime(bool use_colons) const
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
   percentage = ((size_t)(10*percentage + 0.75)) / 10.0 ;
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
