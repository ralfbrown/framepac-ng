/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.12, last edit 2018-09-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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
#include "framepac/atomic.h"
#include "framepac/hashtable.h"

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

size_t small_primes[] =  // all primes < 2^7
   { 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73, 71, 67, 61, 59,
     53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5, 3, 2 } ;
size_t num_small_primes = (sizeof(small_primes)/sizeof(small_primes[0])) ;

thread_local size_t my_job_id ;

/************************************************************************/
/*	Methods for class HashBase					*/
/************************************************************************/

HashBase::HashBase()
{
   m_next.store(nullptr) ;
   ifnot_INTERLEAVED(m_ptrs = nullptr ;)
   m_resizelock.store(false) ;
   m_resizedone.store(false) ;
   m_resizestarted.clear() ;
   m_resizepending.clear() ;
   m_segments_assigned.store((size_t)0) ;
   m_last_incomplete.store(0) ;
   m_size = 0 ;
   m_fullsize = 0 ;
   return ;
}

//----------------------------------------------------------------------------

HashBase::HashBase(size_t size) : HashBase()
{
   size_t searchrange = FrHASHTABLE_SEARCHRANGE ;
   if (size < searchrange/2)
      {
      m_size = size ;
      m_fullsize = size + size/2 ;
      }
   else if (size < 2*searchrange)
      {
      m_size = size - searchrange/4 ;
      m_fullsize = size + searchrange/4 ;
      }
   else if (size < 8*searchrange)
      {
      m_size = size - searchrange/2 ;
      m_fullsize = size + searchrange/4 ;
      }
   else
      {
      m_size = size - searchrange ;
      m_fullsize = size ;
      }
   size_t num_segs = (capacity() + FrHASHTABLE_SEGMENT_SIZE - 1) / FrHASHTABLE_SEGMENT_SIZE ;
   m_resizepending.init(num_segs) ;
   m_segments_total.store(num_segs) ;
   m_first_incomplete.store(size) ;
   if (capacity() > 0)
      {
      ifnot_INTERLEAVED(m_ptrs = new HashPtr[capacity()]) ;
      }
   return ;
}

//----------------------------------------------------------------------------

HashBase::~HashBase()
{
   return ;
}

/************************************************************************/
/*	Methods for class HashTable_Stats				*/
/************************************************************************/

void HashTable_Stats::clear()
{
   insert = insert_dup = insert_attempt = insert_forwarded = insert_resize = 0 ;
   remove = remove_found = remove_forwarded = 0 ;
   contains = contains_found = contains_forwarded = 0 ;
   lookup = lookup_found = lookup_forwarded = 0 ;
   resize = resize_assist = resize_cleanup = resize_wait = 0 ;
   reclaim = 0 ;
   CAS_coll = 0 ;
   neighborhood_full = 0 ;
   spin = yield = sleep = none = 0 ;
   return ;
}

//----------------------------------------------------------------------------

#define REF(x) Fr::Atomic<size_t>::ref(x)
#define REF32(x) Fr::Atomic<uint32_t>::ref(x)

void HashTable_Stats::add(const HashTable_Stats* other)
{
   if (!other) return;
   REF(insert) += other->insert ;
   REF(insert_attempt) += other->insert_attempt ;
   REF(insert_forwarded) += other->insert_forwarded ;
   REF(insert_resize) += other->insert_resize ;
   REF(remove) += other->remove ;
   REF(remove_found) += other->remove_found ;
   REF(remove_forwarded) += other->remove_forwarded ;
   REF(contains) += other->contains ;
   REF(contains_found) += other->contains_found ;
   REF(contains_forwarded) += other->contains_forwarded ;
   REF(lookup) += other->lookup ;
   REF(lookup_found) += other->lookup_found ;
   REF(lookup_forwarded) += other->lookup_forwarded ;
   REF(reclaim) += other->reclaim ;
   REF(spin) += other->spin ;
   REF(yield) += other->yield ;
   REF32(sleep) += other->sleep ;
   REF32(insert_dup) += other->insert_dup ;
   REF32(CAS_coll) += other->CAS_coll ;
   REF32(neighborhood_full) += other->neighborhood_full ;
   REF32(resize) += other->resize ;
   REF32(resize_assist) += other->resize_assist ;
   REF32(resize_cleanup) += other->resize_cleanup ;
   REF32(resize_wait) += other->resize_wait ;
   REF32(none) += other->none ;
   return ;
}

} // end namespace FramepaC //

/************************************************************************/
/************************************************************************/

namespace Fr
{

CriticalSection HashTableBase::s_global_lock ;

} // end of namespace Fr //

// end of file hashtable_data.C //
