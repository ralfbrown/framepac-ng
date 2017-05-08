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

HashTableHelper::HashTableHelper()
{
//TODO
   return  ;
}

//----------------------------------------------------------------------------

HashTableHelper::~HashTableHelper()
{
//TODO
   return  ;
}

//----------------------------------------------------------------------------

bool HashTableHelper::good() const
{
//TODO
   return false ; 
}

//----------------------------------------------------------------------------

HashTableHelper* HashTableHelper::instance()
{
   if (!s_instance)
      {
      HashTableHelper* inst = new HashTableHelper ;
      if (inst && inst->good())
	 {
	 Atomic<HashTableHelper*>& ref = Atomic<HashTableHelper*>::ref(s_instance) ;
	 HashTableHelper* nullinst = nullptr ;
	 if (!ref.compare_exchange_strong(nullinst, inst))
	    {
	    // someone else beat us to installing the unique instance, so just
	    //  delete the one we created
	    delete inst ;
	    }
	 }
      }
   return s_instance ;
}

//----------------------------------------------------------------------------

bool HashTableHelper::queueResize(HashTableBase* ht)
{
   HashTableHelper* inst = instance() ;
   if (!inst)
      return false ;
   return inst->queueResize_(ht) ;
}

//----------------------------------------------------------------------------

bool HashTableHelper::queueReclamation(HashTableBase* ht)
{
   HashTableHelper* inst = instance() ;
   if (!inst)
      {
      SystemMessage::warning("unable to initialize background thread for hashtable memory reclamation") ;
      return false ;
      }
   return inst->queueReclamation_(ht) ;
}

//----------------------------------------------------------------------------

bool HashTableHelper::queueResize_(HashTableBase* ht)
{
   (void)ht; //FIXME

   return false ;
}

//----------------------------------------------------------------------------

bool HashTableHelper::queueReclamation_(HashTableBase* ht)
{
   (void)ht; //FIXME

   return false ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file hashtable_helper.C //
