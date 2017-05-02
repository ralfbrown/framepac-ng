/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-02					*/
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


#include <cstdlib>
#include "framepac/hashtable.h"

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

size_t small_primes[] =
   { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 } ;  //FIXME
size_t num_small_primes = (sizeof(small_primes)/sizeof(small_primes[0])) ;

thread_local size_t my_job_id ;

} // end namespace FramepaC //

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

void HashTable_Stats::clear()
{
   insert = insert_dup = insert_attempt = insert_forwarded = insert_resize = 0 ;
   remove = remove_found = remove_forwarded = 0 ;
   contains = contains_found = contains_forwarded = 0 ;
   lookup = lookup_found = lookup_forwarded = 0 ;
   resize = resize_assist = resize_cleanup = 0 ;
   reclaim = 0 ;
   move = 0 ;
   neighborhood_full = 0 ;
   chain_lock_count = 0 ;
   chain_lock_coll = 0 ;
   spin = yield = sleep = none = 0 ;
   return ;
}


} // end namespace FramepaC //

// end of file hashtable_data.C //
