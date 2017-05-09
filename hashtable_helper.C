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

HashTableBase* hash_table_queue = nullptr ;

bool HashTableHelper::s_initialized = false ;
Semaphore HashTableHelper::s_semaphore ; 
std::thread* HashTableHelper::s_thread = nullptr ;

/************************************************************************/
/************************************************************************/

bool HashTableHelper::initialize()
{
   if (!s_initialized)
      {
      std::thread* thr = new thread(helperFunction) ;
      Atomic<std::thread*>& ref = Atomic<std::thread*>::ref(s_thread) ;
      std::thread* nullthr = nullptr ;
      if (ref.compare_exchange_strong(nullthr, thr))
	 {
	 thr->detach() ;
	 s_initialized = true ;
	 }
      else
	 {
	 // someone else beat us to installing the unique instance, so just
	 //  delete the one we created
	 delete thr ;
	 s_semaphore.post() ;
	 }
      }
   return s_initialized ;
}

//----------------------------------------------------------------------------

void HashTableHelper::helperFunction()
{
   for ( ; ; )
      {
      s_semaphore.wait() ;
#if 0
      // pop the first item off the queue
      HashTableBase* ht = s_queue.pop() ;
      if (!ht)
	 {
	 s_semaphore.post() ;
	 continue ;
	 }
      // invoke that hash table's assist function
      bool more = ht->assistResize() ;
      // if it returns true, there's more work to be done, so re-queue the hash table
      if (more)
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
   if (!initialize())
      {
      SystemMessage::warning("unable to initialize background thread for hashtable resize") ;
      return false ;
      }
   //TODO
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
   if (--m_active_resizes == 0)
      {
      //TODO: de-queue this hash table

      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file hashtable_helper.C //
