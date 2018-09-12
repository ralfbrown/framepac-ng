/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.12, last edit 2018-09-12					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/*  File critsect.C		short-duration critical section mutex	*/
/*									*/
/*  (c) Copyright 2015,2016,2017,2018 Carnegie Mellon University	*/
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

#include <thread>
#include "framepac/critsect.h"

namespace Fr
{

#ifndef FrSINGLE_THREADED
Atomic<size_t> CriticalSection::s_collisions ;
#endif /* !FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

#ifndef FrSINGLE_THREADED
void CriticalSection::backoff_lock()
{
   size_t loops = 0 ;
   while (true)
      {
      incrCollisions() ;
      if (try_lock())
	 return ;
      if (++loops > 5)
	 {
	 // if there are still cores unused, yield() immediately reschedules us, so a somewhat lengthy
	 //   lock by another thread would cause us to burn up lots of CPU time unnecessarily
	 std::this_thread::sleep_for(std::chrono::microseconds(250)) ;
	 }
      else
	 std::this_thread::yield() ;
      }
   return ;
}
#endif /* FrSINGLE_THREADED */

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file critsect.C //
