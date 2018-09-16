/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.12, last edit 2018-09-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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
#include "framepac/hashhelper.h"
#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

atom_flag HashTableHelper::s_initialized ;
MPSC_Queue<HashTableBase*>* HashTableHelper::s_queue = nullptr ;
Semaphore HashTableHelper::s_semaphore ; 

/************************************************************************/
/************************************************************************/

void HashTableHelper::initialize()
{
   if (!s_initialized.test_and_set())
      {
#ifndef FrSINGLE_THREADED
      s_queue = new MPSC_Queue<HashTableBase*> ;
      ScopedPtr<std::thread> thr { new thread(helperFunction) } ;
      thr->detach() ;
#endif /* !FrSINGLE_THREADED */
      }
   return ;
}

//----------------------------------------------------------------------------

void HashTableHelper::helperFunction()
{
   ThreadInit() ;
   for ( ; ; )
      {
      s_semaphore.wait() ;
      // pop the first item off the queue
      HashTableBase* ht = s_queue->pop() ;
      // invoke that hash table's assist function
      bool more = ht->assistResize() ;
      // if it returns true, there's more work to be done, so re-queue the hash table
      if (more)
	 {
	 s_queue->push(ht) ;
	 s_semaphore.post() ;
	 this_thread::yield() ;
	 }
      }
}

//----------------------------------------------------------------------------

bool HashTableHelper::startHelper(HashTableBase* ht)
{
   initialize() ;
   if (ht)
      {
      s_queue->push(ht) ;
      s_semaphore.post() ;
      }
   return true ;
}

/************************************************************************/
/*	Methods for class HashTableBase					*/
/************************************************************************/

void HashTableBase::startResize()
{
   m_active_resizes++ ;
   HashTableHelper::startHelper(this) ;
   return ;
}

//----------------------------------------------------------------------------

void HashTableBase::finishResize()
{
   m_active_resizes-- ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file hashtable_helper.C //
