/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#ifndef _Fr_TIMER_H_INCLUDED
#define _Fr_TIMER_H_INCLUDED

#include <chrono>
#include <ctime>
#include <ostream>

namespace Fr
{

//----------------------------------------------------------------------------

class TimerBase
   {
   public:
      static bool elapsedTime(double elapsed, unsigned &days, unsigned &hours, unsigned &minutes,
			      unsigned &secs, double &fraction) ;
      static char *formattedTime(double elapsed, bool use_colons = true) ;
      static std::ostream& formatTime(std::ostream&, double elapsed, bool use_colons = true) ;
   } ;

//----------------------------------------------------------------------------

class CpuTimer : public TimerBase
   {
   public: // types
      typedef TimerBase super ;
   public:
      CpuTimer() : m_start_time(currTime()) {}
      ~CpuTimer() {}

      void restart() { m_start_time = currTime() ; }
      double seconds() const ; 
      using TimerBase::elapsedTime ;
      bool elapsedTime(unsigned &days, unsigned &hours, unsigned &minutes, unsigned &secs, double &fraction) const ;
      using TimerBase::formattedTime ;
      char *formattedTime(bool use_colons = false) const ;
   private:
#ifdef _POSIX_CPUTIME
      struct timespec m_start_time ;
      struct timespec currTime() const
	 { struct timespec tm ; clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&tm) ; return tm ; }
#else
      std::clock_t m_start_time ;
      std::clock_t currTime() const { return std::clock() ; }
#endif /* _POSIX_CPUTIME */
   } ;

//----------------------------------------------------------------------------

class ElapsedTimer : public TimerBase
   {
   public: // types
      typedef TimerBase super ;
      typedef std::chrono::steady_clock Clock ;
   public:
      ElapsedTimer() : m_start(currTime()) {}
      ~ElapsedTimer() {}

      void restart() { m_start = currTime() ; }
      double seconds() const ;
      using TimerBase::elapsedTime ;
      bool elapsedTime(unsigned &days, unsigned &hours, unsigned &minutes, unsigned &secs, double &fraction) const ;
      using TimerBase::formattedTime ;
      char *formattedTime(bool use_colons = false) const ;
      std::chrono::time_point<Clock> currTime() const { return Clock::now() ; }

   private:
      std::chrono::time_point<Clock> m_start ;
   } ;

//----------------------------------------------------------------------------

class Timer
   {
   public:
      Timer() : m_elapsed(), m_cputime() {}
      ~Timer() {}

      void restart() ;

      double cpuSeconds() const { return m_cputime.seconds() ; }
      double elapsedSeconds() const { return  m_elapsed.seconds() ; }

      std::ostream& showTimes(std::ostream& out) const ;

   protected:
      ElapsedTimer m_elapsed ;
      CpuTimer     m_cputime ;
   } ;

} // end namespace Fr

//----------------------------------------------------------------------------

inline std::ostream& operator << (std::ostream& out, const Fr::Timer& tmr) { return tmr.showTimes(out) ; }

#endif /* !_Fr_TIMER_H_INCLUDED */

// end of file timer.h //
