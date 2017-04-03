/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-03					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

#ifndef _Fr_THREAD_H_INCLUDED
#define _Fr_THREAD_H_INCLUDED

#include <functional>
#include <thread>
#include "framepac/init.h"

namespace Fr
{

template <typename C>
void thread_main(C closure)
{
   ThreadInit() ; // per-thread initialization
   try
      {
      closure() ;
      }
   catch (const std::exception& e)
      {
      // perform thread cleanup and then re-throw
      ThreadCleanup() ;
      throw ;
      }
   catch (...)
      {
      // perform thread cleanup and then re-throw
      ThreadCleanup() ;
      throw ;
      }
   //finally
      {
      ThreadCleanup() ;
      }
   return;
}

template <typename F,typename ...Args>
std::thread* new_thread(F fn,Args&&... args)
{
   auto closure = std::bind(fn,args...) ;
   return new std::thread(thread_main<decltype(closure)>,closure) ;
}

} // end namespace Fr

#endif /* !_Fr_THREAD_H_INCLUDED */

// end of file thread.h //
