/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-08					*/
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

#ifndef __Fr_QUEUE_MPSC_H_INCLUDED
#define __Fr_QUEUE_MPSC_H_INCLUDED

#include "framepac/atomic.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

// A Vyukovic-style multi-producer/single-consumer unbounded queue
// see: http://www.1024cores.net/home/lock-free-algorithms/queues/non-intrusive-mpsc-node-based-queue

template <typename T>
class MPSC_Queue
   {
   public:
      class Node
         {
	 public: // methods
	    Node() {}
	    Node(T val, MPSC_Queue* nxt = 0) { m_value = val ; m_next = nxt ; }
	    ~Node() {}
	 public: // data
	    Node* m_next ;
	    T     m_value ;
	 } ;
      //--------------------------
      MPSC_Queue()
	 {
	 Node* dummy = new Node ;
	 dummy->m_next = nullptr ;
	 m_head = m_tail = dummy ;
	 }
      ~MPSC_Queue()
	 {
	 // clear out any items remaining in the queue to avoid memory leaks
	 while (m_tail)
	    {
	    Node* tmp = m_tail ;
	    m_tail = m_tail->m_next ;
	    delete tmp ;
	    }
	 m_head = nullptr ;
	 return ;
	 }

      // any thread may push values
      void push(T value)
         {
	 Node* n = new Node(value) ;
	 Node* prev = Atomic<Node*>::ref(m_head).exchange(n) ;
	 prev->m_next = n ;
	 }
      // only the 'owning' thread can pop values!
      T pop()
         {
	 Node* n = m_tail  ;
	 Node* next ;
	 while ((next = Atomic<Node*>::ref(n->m_next).load()) == nullptr)
	    {
	    std::this_thread::yield() ;
	    }
	 m_tail = next ;
	 delete n ;
	 return next->m_value ;
	 }
      // only the 'owning' thread can pop values!
      T nonblockingPop()
         {
	 Node* n = m_tail  ;
	 Node* next = Atomic<Node*>::ref(n->m_next).load() ;
	 if (next)
	    {
	    m_tail = next ;
	    delete n ;
	    return next->m_value ;
	    }
	 return null() ;
	 }
   protected: // methods
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value, RetT>::type
      null() { return (T)nullptr ; }
      template <typename RetT = T>
      typename std::enable_if<!std::is_pointer<T>::value, RetT>::type
      null() { return T(0) ; }

   protected: // data
      Node*  m_head ;
      Node*  m_tail ;
   } ;


} // end namespace Fr

#endif /* !__Fr_QUEUE_MPSC_H_INCLUDED */

// end of file queue_mpsc.h //
