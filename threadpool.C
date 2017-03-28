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

#include "framepac/threadpool.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

ThreadPool::ThreadPool(unsigned num_threads)
{
#ifdef FrSINGLETHREADED
   (void)num_threads ;
   m_numthreads = 0 ;
   m_pool = 0 ;
#else
   m_numthreads = num_threads ;
   m_pool = new thread*[num_threads] ;
   if (!m_pool)
      {
      m_numthreads = 0 ;
      return ;
      }
   //FIXME
#endif /* FrSINGLETHREADED */
   return ;
}

//----------------------------------------------------------------------------

ThreadPool::~ThreadPool()
{
#ifndef FrSINGLE_THREADED
   //FIXME
   delete [] m_pool ;
#endif /* !FrSINGLE_THREADED */
   m_numthreads = 0 ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file threadpool.C //
