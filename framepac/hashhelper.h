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

#ifndef _FrHASHHELPER_H_INCLUDED
#define _FrHASHHELPER_H_INCLUDED

#include "framepac/queue_mpsc.h"
#include "framepac/semaphore.h"

namespace Fr
{

/************************************************************************/
/*	Declarations for class HashTableHelper				*/
/************************************************************************/

class HashTableHelper
   {
   public:
      static bool startHelper(HashTableBase* ht = nullptr) ;

   protected:  // internal methods
      HashTableHelper() {}
      ~HashTableHelper() { delete s_queue ; }

      static void initialize() ;
      [[gnu::noreturn]] static void helperFunction() ;

   protected:
      static Semaphore	                 s_semaphore ;
      // TSAN throws up warnings because the main thread accesses a field only the owning thread should touch,
      //   so make s_queue a pointer and have the helper thread allocate it
      static MPSC_Queue<HashTableBase*>* s_queue ;
      static atom_flag                   s_initialized ;
   } ;

} // end namespace Fr

#endif /* !_FrHASHHELPER_H_INCLUDED */

// end of file hashhelper.h //
