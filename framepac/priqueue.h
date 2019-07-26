/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-26					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#ifndef __Fr_PRIQUEUE_H_INCLUDED
#define __Fr_PRIQUEUE_H_INCLUDED

#include "framepac/object.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

/************************************************************************/
/************************************************************************/

// the implementation of this class currently has
//    O(1) worst-case for front(), pop(), clear(), size(), and empty()
//    O(N) worst-case for push() and changePrio()
// so it should only be used for relatively small queues

class BoundedPriorityQueue
   {
   public:
      BoundedPriorityQueue(size_t cap = 0) ;
      ~BoundedPriorityQueue() ;

      size_t size() const { return m_qtail - m_qhead ; }
      bool empty() const { return m_qtail == m_qhead ; }
      Object* front() const { return m_qtail > m_qhead ? m_elements[m_qhead] : nullptr ; }
      Object* pop() { return (m_qtail > m_qhead) ? m_elements[m_qhead++] : nullptr ; }
      void clear() { m_qtail = m_qhead = 0 ; }
      bool push(Object*, double priority, bool del_if_dropped = false) ;
      void changePrio(Object*, double new_priority) ;

   protected:
      Object**  m_elements ;
      float*    m_priorities ;
      size_t    m_capacity ;
      size_t    m_qhead ;
      size_t    m_qtail ;
   } ;

//----------------------------------------------------------------------

} // end namespace Fr

#endif /* !__Fr_PRIQUEUE_H_INCLUDED */

// end of file priqueue.h //
