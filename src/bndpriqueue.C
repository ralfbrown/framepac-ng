/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-07					*/
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

#include "framepac/priqueue.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/*	Methods for class BoundedPriorityQueue				*/
/************************************************************************/

BoundedPriorityQueue::BoundedPriorityQueue(size_t cap)
{
   if (cap < 2) cap = 2 ;
   m_elements = new Object*[cap] ;
   m_priorities = new float[cap] ;
   m_capacity = cap ;
   m_qhead = 0 ;
   m_qtail = 0 ;
   return ;
}

//----------------------------------------------------------------------

BoundedPriorityQueue::~BoundedPriorityQueue()
{
   delete[] m_elements ;
   delete[] m_priorities ;
   m_capacity = 0 ;
   return ;
}

//----------------------------------------------------------------------

bool BoundedPriorityQueue::push(Object* obj, double priority, bool del_if_dropped)
{
   if (m_qhead > 0)
      {
      // we have space at the start of the array due to pops, so push elements down
      size_t dest = 0 ;
      for (size_t i = m_qhead ; i  < m_qtail ; ++i)
	 {
	 if (priority <= m_priorities[i])
	    continue ;
	 // we've found the insertion point
	 m_elements[dest] = obj ;
	 m_priorities[dest++] = priority ;
	 // move all the rest of the elements down if there were multiple pops
	 if (m_qhead == 1) break ;	// only one pop, so no more moves needed
	 while (i < m_qtail)
	    {
	    m_elements[dest] = m_elements[i] ;
	    m_priorities[dest++] = m_priorities[i++] ;
	    }
	 m_qhead = 0 ;
	 m_qtail = dest ;
	 }
      }
   else if (m_qtail == m_capacity && priority <= m_priorities[m_qtail-1])
      {
      // queue is full and the new element's priority is too low to be added
      if (del_if_dropped && obj)
	 obj->free() ;
      return false ;
      }
   else
      {
      if (m_qtail == m_capacity)
	 {
	 // queue is full, so drop the lowest-priority item to make space
	 if (del_if_dropped)
	    m_elements[m_capacity-1]->free() ;
	 --m_qtail ;
	 }
      // push elements up
      size_t pos = m_qtail ;
      while (pos > 0 && m_priorities[pos-1] < priority)
	 {
	 m_elements[pos] = m_elements[pos-1] ;
	 m_priorities[pos] = m_priorities[pos-1] ;
	 pos-- ;
	 }
      // 'pos' now points at the proper insertion point, so put the new item there
      m_elements[pos] = obj ;
      m_priorities[pos] = priority ;
      }
   return true ;
}

//----------------------------------------------------------------------

void BoundedPriorityQueue::changePrio(Object* obj, double new_priority)
{
   if (!obj) return ;
   // scan the elements to find the object, so that we can get its current priority
   size_t pos = m_capacity ;
   for (size_t i = m_qhead ; i < m_qtail ; ++i)
      {
      if (m_elements[i] == obj)
	 {
	 pos = i ;
	 break ;
	 }
      }
   if (pos == m_capacity)
      return  ;
   // get current priority
   double curr_prio = m_priorities[pos] ;
   if (new_priority > curr_prio)
      {
      // bubble toward head

      //TODO
      }
   else if (new_priority < curr_prio)
      {
      // bubble toward tail
      
      //TODO
      }
   return ;
}


} // end namespace Fr

// end of file bndpriqueue.C //
