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

#include <thread>
#include "framepac/atomic.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

class WorkOrder
   {
   private:
      //function to call
      void       *m_input ;
      void       *m_output ;
      atomic_bool m_inprogress ;
      atomic_bool m_complete ;
   public:
      WorkOrder() ;
      ~WorkOrder() ;
   } ;

/************************************************************************/
/************************************************************************/

#define FrWORKQUEUE_SIZE 256

class WorkQueue
   {
   private:
      WorkOrder m_orders[FrWORKQUEUE_SIZE] ;
      unsigned  m_start ;
      unsigned  m_head ;
      unsigned  m_tail ;
   public:
      WorkQueue() ;
      ~WorkQueue() ;
   } ;

/************************************************************************/
/************************************************************************/

class ThreadPool
   {
   private:
      thread **m_pool ;
      unsigned m_numthreads ;

   public:
      ThreadPool(unsigned num_threads) ;
      ~ThreadPool() ;
   } ;


} // end namespace Fr

// end of file threadpool.h //
