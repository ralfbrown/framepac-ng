/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-07					*/
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

#include "framepac/atomic.h"
#include "framepac/hashtable.h"
#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

atom_flag HashTableHelper::s_initialized ;
MPSC_Queue<HashTableBase*> HashTableHelper::s_queue ;
Semaphore HashTableHelper::s_semaphore ; 

/************************************************************************/
/************************************************************************/

void HashTableHelper::initialize()
{
   if (!s_initialized.test_and_set())
      {
      std::thread* thr = new thread(helperFunction) ;
      thr->detach() ;
      delete thr ;
      }
   return ;
}

//----------------------------------------------------------------------------

void HashTableHelper::remove(const HashTableBase* ht)
{
   // scan the queue for the entry for 'ht' and remove it
   (void)ht;
//   s_queue.remove(ht) ;
   return ;
}

//----------------------------------------------------------------------------

void HashTableHelper::helperFunction()
{
   for ( ; ; )
      {
      s_semaphore.wait() ;
#if 0
      // pop the first item off the queue
      HashTableBase* ht ;
      bool popped = s_queue.pop(ht) ;
      if (!popped)
	 {
	 s_semaphore.post() ;
	 continue ;
	 }
      else if (!ht)
	 {
	 // the hash table removed itself because it was being destructed
	 continue ;
	 }
      // invoke that hash table's assist function
      bool more = ht->assistResize() ;
      // if it returns true, there's more work to be done, so re-queue the hash table
      if (more || ht->activeResizes() > 0)
	 {
	 s_queue.push(ht) ;
	 s_semaphore.post() ;
	 }
#endif
      }
}

//----------------------------------------------------------------------------

bool HashTableHelper::queueResize(HashTableBase* ht)
{
   initialize() ;
#if 0
   s_queue.push(ht) ;
   s_semaphore.post() ;
#endif
   (void)ht;
   return true ;
}

/************************************************************************/
/*	Methods for class HashTableBase					*/
/************************************************************************/

void HashTableBase::startResize()
{
   if (m_active_resizes++ == 0)
      HashTableHelper::queueResize(this) ;
   return ;
}

//----------------------------------------------------------------------------

void HashTableBase::finishResize()
{
   --m_active_resizes ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file hashtable_helper.C //
