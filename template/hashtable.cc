/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-10					*/
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

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <thread>
#include <type_traits>
#include <x86intrin.h>
#include "framepac/hashtable.h"
#include "framepac/message.h"

#if DYNAMIC_ANNOTATIONS_ENABLED != 0
#  include "dynamic_annotations.h"
#endif /* DYNAMIC_ANNOTATIONS_ENABLED */

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#define FrNAP_TIME 250

namespace FramepaC
{
extern size_t small_primes[] ;
extern size_t num_small_primes ;

} // end namespace FramepaC //

namespace Fr {

#undef FrSPIN_COUNT
#define FrSPIN_COUNT 20

#undef FrYIELD_COUNT
#define FrYIELD_COUNT 50

#define FramepaC_display_width 132

} // end namespace Fr

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#if defined(FrSINGLE_THREADED)
#define FORWARD(delegate,tab,counter)
#define FORWARD_IF_COPIED(delegate,counter)
#define FORWARD_IF_STALE(delegate,counter)
#else
#define FORWARD(delegate,tab,counter)					\
      Table* tab = next() ;						\
      if (tab)								\
	 {								\
	 /* ensure that our bucket has been copied to the	*/	\
	 /*   successor table, then add the key to that table	*/	\
         if (!copyChain(bucketnum))					\
	    {								\
	    resizeCopySegments(1) ;					\
	    waitUntilCopied(bucketnum) ;				\
	    }								\
	 /* help out with the copying in general */			\
	 resizeCopySegments(1) ;					\
	 INCR_COUNT(counter) ;						\
	 tab->announceTable() ;						\
	 return tab->delegate ;						\
	 }							
#define FORWARD_IF_COPIED(delegate,counter)				\
      if (HashPtr::copyDone(status))					\
	 {								\
	 /* our bucket has been fully copied to the successor	*/	\
	 /*   table, so look for the key in that table instead	*/	\
	 /*   of the current one				*/	\
         Table* nexttab = next() ;					\
	 INCR_COUNT(counter) ;						\
	 nexttab->announceTable() ;					\
	 return nexttab->delegate ;					\
	 }
#define FORWARD_IF_STALE(delegate,counter)				\
      if (HashPtr::stale(status))					\
	 {								\
	 /* our bucket has at least started to be copied to the	*/	\
	 /*   successor table, so look for the key in that	*/	\
	 /*   table instead of the current one			*/ 	\
	 INCR_COUNT(counter) ;						\
	 resizeCopySegments(1) ;					\
	 waitUntilCopied(bucketnum) ;					\
	 Table* nexttab = next() ;					\
	 nexttab->announceTable() ;					\
	 return nexttab->delegate ;					\
	 }
#endif /* FrSINGLE_THREADED */

/************************************************************************/
/*	Methods for class Table						*/
/************************************************************************/

namespace Fr
{


template <typename KeyT, typename ValT>
inline void HashTable<KeyT,ValT>::Table::init(size_t size)
{
   m_size = size ;
   if (size < searchrange)
      m_fullsize = 2*size ;
   else if (size < 16*searchrange)
      m_fullsize = size + searchrange/2 ;
   else
      m_fullsize = size + searchrange ;
   m_next_table.store(nullptr) ;
   m_next_free.store(nullptr) ;
   m_entries = nullptr ;
   ifnot_INTERLEAVED(m_ptrs = nullptr ;)
   m_resizelock.store(false) ;
   m_resizedone.store(false) ;
   m_resizestarted.clear() ;
   m_resizepending.clear() ;
   size_t num_segs = (m_fullsize + FrHASHTABLE_SEGMENT_SIZE - 1) / FrHASHTABLE_SEGMENT_SIZE ;
   m_resizepending.init(num_segs) ;
   m_segments_assigned.store((size_t)0) ;
   m_segments_total.store(num_segs) ;
   m_first_incomplete.store(size) ;
   m_last_incomplete.store(0) ;
   remove_fn = nullptr ;
   if (m_fullsize > 0)
      {
      m_entries = new Entry[m_fullsize] ;
      ifnot_INTERLEAVED(m_ptrs = new HashPtr[m_fullsize]) ;
      if (!m_entries
	  ifnot_INTERLEAVED(|| !m_ptrs)
	 )
	 {
	 delete[] m_entries ;
	 m_entries = nullptr ;
	 ifnot_INTERLEAVED(delete[] m_ptrs) ;
	 ifnot_INTERLEAVED(m_ptrs = nullptr) ;
	 m_size = 0 ;
	 m_fullsize = 0 ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::clear()
{
   delete[] m_entries ;
   ifnot_INTERLEAVED(delete[] m_ptrs) ;
   m_entries = nullptr ;
   ifnot_INTERLEAVED(m_ptrs = nullptr) ;
   m_next_table.store(nullptr) ;
   m_next_free.store(nullptr) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t HashTable<KeyT,ValT>::Table::normalizeSize(size_t sz)
{
   if (sz < FrHASHTABLE_MIN_SIZE)
      sz = FrHASHTABLE_MIN_SIZE ;
   // bump up the size a little to avoid multiples of small
   //   primes like 2, 3, 5, 7, 11, 13, and 17
   if (sz % 2 == 0)
      ++sz ;
   size_t pre_bump ;
   do {
      pre_bump = sz ;
      for (size_t i = 0 ; i < FramepaC::num_small_primes ; ++i)
	 {
	 if (sz % FramepaC::small_primes[i] == 0)
	    sz += 2 ;
	 }
      } while (sz != pre_bump) ;
   return sz ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::autoResize()
{
   size_t newsize ;
   size_t sz = m_size ;
#ifdef FrSAVE_MEMORY
   if (sz < 100000)
      newsize = (size_t)(2.0*sz) ;
   else if (sz < 1000000)
      newsize = (size_t)(1.5*sz) ;
   else if (sz < 10*1000*1000)
      newsize = (size_t)(1.4*sz) ;
   else if (sz < 100*1000*1000)
      newsize = (size_t)(1.3*sz) ;
   else
      newsize = (size_t)(1.2*sz) ;
#else
   if (sz < 16*1000*1000)
      newsize = (size_t)(2.0*sz) ;
   else
      newsize = (size_t)(1.5*sz) ;
#endif /* FrSAVE_MEMORY */
   resize(newsize) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::copyChain(size_t bucketnum)
{
   Link offset ;
#ifndef FrSINGLE_THREADED
   HashPtr* bucket = bucketPtr(bucketnum) ;
   // atomically set the 'stale' bit and get the current status
   Link status = bucket->markStaleGetStatus() ;
   if (HashPtr::stale(status))
      {
      // someone else has already worked on this bucket, or a
      //   copy/recycle() is currently running, in which case that
      //   thread will do the copying for us
      return HashPtr::copyDone(status) ;
      }
   // since an add() could still be running in parallel, we need to
   //   start by making any deleted entries in the chain unusable by
   //   add(), so that it doesn't re-use them after we've skipped them
   //   during the copy loop
   offset = bucket->first() ;
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      bucketPtr(pos)->markReclaiming() ;
      }
   // the chain is now frozen -- add() can't push to the start or
   //   re-use anything on the chain
#endif /* !FrSINGLE_THREADED */
   // insert all elements of the current hash bucket into the next table
   offset = bucket->first() ;
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      // insert the element, ignoring it if it is deleted or a duplicate
      KeyT key = getKey(pos) ;
      if (key == Entry::DELETED())
	 {
	 continue ;
	 }
      size_t hashval = m_container->hashValFull(key) ;
      Entry* element = &m_entries[pos] ;
      if (next()->reAdd(hashval,key,element->getValue()))
	 {
	 // duplicate, apply removal function to the value
	 removeValue(*element) ;
	 }
      }
   // we're done copying the bucket, let anybody who is waiting on the
   //   copy proceed in the new table
   markCopyDone(bucketnum) ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::waitUntilCopied(size_t bucketnum)
{
   size_t loops = 0 ;
   while (!chainCopied(bucketnum))
      {
      thread_backoff(loops) ;
      }
   INCR_COUNT_if(loops,resize_wait) ;
   return  ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::copyChains(size_t bucketnum, size_t endpos)
{
   bool complete = true ;
   size_t first_incomplete = endpos ;
   size_t last_incomplete = bucketnum ;
   for (size_t i = bucketnum ; i <= endpos ; ++i)
      {
      if (!copyChain(i))
	 {
	 last_incomplete = i ;
	 if (complete)
	    {
	    complete = false ;
	    first_incomplete = i ;
	    }
	 }
      }
   // if we had to skip any chains due to a concurrent copy, tell the
   //   lead resizing thread that it needs to do a cleanup pass
   if (!complete)
      {
      // but if there was just one skipped chain and its copy has now
      //   completed, we don't need to say anything
      if (last_incomplete > first_incomplete || !bucketPtr(first_incomplete)->copyDone())
	 {
	 m_first_incomplete.decreaseTo(first_incomplete) ;
	 m_last_incomplete.increaseTo(last_incomplete) ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------
// add a key which is known not to be in the table (yet)

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::reAdd(size_t hashval, KeyT key, ValT value)
{
   INCR_COUNT(insert_resize) ;
   DECR_COUNT(insert_attempt) ; // don't count as a retry unless we need more than one attempt
   size_t bucketnum = hashval % m_size ;
   while (true)
      {
      FORWARD(reAdd(hashval,key,value),nexttab,insert_forwarded) ;
      Link firstoffset = chainHead(bucketnum) ;
      Link offset = firstoffset ;
      while (FramepaC::NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT key_at_pos = getKey(pos) ;
	 if (m_container->isEqualFull(key,key_at_pos))
	    {
	    INCR_COUNT(insert_dup) ;
	    return true ;		// item already exists
	    }
	 // advance to next item in chain
	 offset = chainNext(pos) ;
	 }
      if (insertKey(bucketnum,firstoffset,key,value))
	 break ;
      }
   return false ;			// item did not already exist
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
FramepaC::Link HashTable<KeyT,ValT>::Table::locateEmptySlot(size_t bucketnum, Link hint)
{
   if (hint == FramepaC::NULLPTR) hint = 0 ;
   size_t max = searchrange ;
   if (bucketnum + max > m_fullsize)
      max = m_fullsize - bucketnum ;
   for (size_t i = hint ; i < max ; ++i)
      {
      HashPtr* hp = bucketPtr(bucketnum + i) ;
      if (!hp->inUse() && hp->markUsed())
	 return i ;
      }
   // if we get here, there were no free slots available
   INCR_COUNT(neighborhood_full) ;
   return FramepaC::NULLPTR ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::insertKey(size_t bucketnum, Link firstptr, KeyT key, ValT value)
{
   INCR_COUNT(insert_attempt) ;
   Link offset = locateEmptySlot(bucketnum,firstptr) ;
   if (unlikely(FramepaC::NULLPTR == offset))
      {
      if (!superseded())
	 autoResize() ;
      return false ;
      }
   // fill in the slot we grabbed and link it to the head of
   //   the chain for the hash bucket
   size_t pos = bucketnum + offset ;
   bucketPtr(pos)->next(firstptr) ;
   setValue(pos,value) ;
   setKey(pos,key) ;
   HashPtr* headptr = bucketPtr(bucketnum) ;
   if (superseded())
      return false ;
#ifdef FrSINGLE_THREADED
   // life is much simpler in non-threaded mode: just point
   //   the chain head at the new node and increment the
   //   tally of items in the table
   headptr->first(offset) ;
#else
   // now that we've done all the preliminaries, try to get
   //   write access to actually insert the new entry
   // try to point the hash chain at the new entry
   Link status = headptr->status() ;
   if (unlikely(!headptr->first(offset,firstptr,status)))
      {
      // oops, someone else messed with the hash chain, which
      //   means there could have been a parallel insert of
      //   the same value, or a resize is in progress and
      //   copied the bucket while we were working
      // release the slot we grabbed and tell caller to retry
      setKey(pos,Entry::DELETED()) ;
      bucketPtr(pos)->markFree() ;
      INCR_COUNT(CAS_coll) ;
      debug_msg("insertKey: CAS collision\n") ;
      return false ;
      }
#endif /* FrSINGLE_THREADED */
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::resizeCopySegment(size_t segnum)
{
   size_t bucketnum = segnum * FrHASHTABLE_SEGMENT_SIZE ;
   size_t endpos = (segnum + 1) * FrHASHTABLE_SEGMENT_SIZE ;
   if (endpos > m_fullsize)
      endpos = m_fullsize ;
   copyChains(bucketnum,endpos-1) ;
   // record the fact that we copied a segment
   m_resizepending.consume() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::resizeCopySegments(size_t max_segs)
{
   // is there any work available to be stolen?
   if (!m_resizelock.load() || m_resizedone.load())
      return ;
   size_t total_segs = m_segments_total.load() ;
   // grab the current segment number and increment it so the next
   //   thread gets a different number; stop once all segments have
   //   been assigned
   while (max_segs > 0 && m_segments_assigned.load() <= total_segs)
      {
      size_t segnum ;
      if ((segnum = m_segments_assigned++) < total_segs)
	 {
	 resizeCopySegment(segnum) ;
	 --max_segs ;
	 }
      else if (segnum == total_segs)
	 {
	 // we've won the right to finalize the resizing!
	 // wait for any other threads that have grabbed segments to
	 //   complete them
	 m_resizepending.wait() ;
	 // if necessary, do a cleanup pass to copy any buckets which
	 //   had to be skipped due to concurrent reclaim() calls,
	 //   then finalize the resize
	 resizeCleanup() ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::clearDuplicates(size_t bucketnum)
{
   // scan down the chain for the given hash bucket, marking
   //   any duplicate entries for a key as deleted
   Link offset = chainHead(bucketnum) ;
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      KeyT currkey = getKey(pos) ;
      if (currkey == Entry::DELETED())
	 continue ;
      Link nxt = offset ;
      while (FramepaC::NULLPTR != nxt)
	 {
	 size_t nextpos = bucketnum + nxt ;
	 nxt = chainNext(nextpos) ;
	 KeyT nextkey = getKey(nextpos) ;
	 if (nextkey != Entry::DELETED() && m_container->isEqualFull(currkey,nextkey))
	    {
	    // duplicate, apply removal function to the value
	    removeValue(m_entries[nextpos]) ;
	    setKey(nextpos,Entry::DELETED()) ;
	    }
	 }
      }
   return ;
}

//----------------------------------------------------------------------------
//   must ONLY be called while the hash table is otherwise quiescent

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::reclaimChain(size_t bucketnum)
{
#ifdef FrSINGLE_THREADED
   (void)bucketnum ;
   // remove() chops out the deleted entries, so there's never anything to reclaim
   return false ;
#else
   HashPtr* headptr = bucketPtr(bucketnum) ;
   Link offset = headptr->first() ;
   bool reclaimed = false ;
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      Link nxt = bucketPtr(pos)->next() ;
      if (getKey(pos) != Entry::DELETED())
	 break ;
      headptr->first(nxt) ;
      bucketPtr(pos)->markFree() ;
      offset = nxt ;
      reclaimed = true ;
      }
   if (FramepaC::NULLPTR == offset)
      return reclaimed ;

   size_t prevpos = bucketnum + offset ;
   offset = bucketPtr(prevpos)->next() ;
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      HashPtr* nextptr = bucketPtr(pos) ;
      KeyT key = getKey(pos) ;
      Link nxt = nextptr->next() ;
      if (key == Entry::DELETED())
	 {
	 // we found a deleted entry; now unlink it
	 bucketPtr(prevpos)->next(nxt) ;
	 nextptr->markFree() ;
	 reclaimed = true ;
	 }
      else
	 {
	 prevpos = pos ;
	 }
      offset = nxt ;
      }
   return reclaimed;
#endif /* FrSINGLE_THREADED */
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::resizeCleanup()
{
   size_t first_incomplete = m_first_incomplete.load() ;
   size_t last_incomplete = m_last_incomplete.load() ;
   size_t loops = FrSPIN_COUNT ;  // jump right to yielding the CPU on backoff
   while (first_incomplete <= last_incomplete)
      {
      INCR_COUNT(resize_cleanup) ;
      bool complete = true ;
      size_t first = m_fullsize ;
      size_t last = 0 ;
      for (size_t i = first_incomplete ; i < last_incomplete && i < m_fullsize ; ++i)
	 {
	 if (!chainCopied(i) && !copyChain(i))
	    {
	    last = i ;
	    if (complete)
	       {
	       first = i ;
	       complete = false ;
	       }
	    }
	 }
      if (complete)
	 break ;
      first_incomplete = first ;
      last_incomplete = last ;
      thread_backoff(loops) ;
      }
   m_resizedone.store(true) ;
   // make the new table the current one for the containing HashTable
   m_container->updateTable() ;
   debug_msg(" resize done (thr %ld)\n",FramepaC::my_job_id) ;
   return  ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::resize(size_t newsize)
{
   if (superseded())
      {
      return false ;
      }
   if (unlikely(m_resizelock.exchange(true)))
      {
      // someone else has already claimed the right to
      //   resize, so wait until the resizing starts and
      //   then help them out
      INCR_COUNT(resize_assist) ;
      m_resizestarted.wait() ;
      resizeCopySegments() ;
      return true ;
      }
   // OK, we've won the right to run the resizing
   INCR_COUNT(resize) ;
   newsize = normalizeSize(newsize) ;
   debug_msg("resize to %ld from %ld (thr %ld)\n",newsize,m_size,FramepaC::my_job_id) ;
   Table* newtable = m_container->allocTable() ;
   newtable->init(newsize) ;
   newtable->m_container = m_container ;
   if (unlikely(!newtable->good()))
      {
      SystemMessage::warning("unable to resize hash table--will continue") ;
      m_container->releaseTable(newtable) ;
      return false ;
      }
   newtable->onRemove(m_container->onRemoveFunc()) ;
   // link in the new table; this also makes
   //   operations on the current table start to
   //   forward to the new one
   m_next_table.store(newtable) ;
   m_resizestarted.set() ;
   // enqueue ourself on the HashTableHelper thread
   m_container->startResize() ;
   // grab as many segments as we can and copy them
   resizeCopySegments() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::add(size_t hashval, KeyT key, ValT value)
{
   size_t bucketnum = hashval % m_size ;
   while (true)
      {
      FORWARD(add(hashval,key,value),nexttab,insert_forwarded) ;
      // scan the chain of items for this hash position
      if_THREADED(size_t deleted = NULLPOS ;)
      Link offset = chainHead(bucketnum) ;
      Link firstoffset = offset ;
      while (FramepaC::NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT key_at_pos = getKey(pos) ;
	 if (m_container->isEqual(key,key_at_pos))
	    {
	    // found existing entry!
	    INCR_COUNT(insert_dup) ;
	    return true ;		// item already exists
	    }
	 // advance to next item in chain
	 offset = chainNext(pos) ;
	 if_THREADED(if (key_at_pos == Entry::DELETED() && deleted == NULLPOS && !bucketPtr(pos)->reclaiming()) { deleted = pos ; })
	 }
      // when we get here, we know that the item is not yet in the
      //   hash table, so try to add it
#ifndef FrSINGLE_THREADED
      // first, did we encounter a deleted entry? (can only happen if multithreaded)
      // If so, try to reclaim it
      if (deleted != NULLPOS)
	 {
	 if (unlikely(!bucketPtr(deleted)->markReclaiming()))
	    {
	    INCR_COUNT(CAS_coll) ;
	    continue ;		// someone else reclaimed the entry, so restart
	    }
	 // we managed to grab the entry for reclamation
	 // fill in the value, then the key
	 setValue(deleted,value) ;
	 atomic_thread_fence(std::memory_order_release) ;
	 setKey(deleted,key) ;
	 bucketPtr(deleted)->markReclaimed() ;
	 INCR_COUNT(insert_attempt) ;
	 // Verify that we haven't been superseded while we
	 //   were working
	 FORWARD(add(hashval,key,value),nexttab2,insert_forwarded) ;
	 break ;
	 }
#endif /* !FrSINGLE_THREADED */
      // otherwise, try to insert a new key/value entry
      if (insertKey(bucketnum,firstoffset,key,value))
	 break ;
      }
   return false ;  // item did not already exist
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
ValT HashTable<KeyT,ValT>::Table::addCount(size_t hashval, KeyT key, size_t incr)
{
   size_t bucketnum = hashval % m_size ;
   while (true)
      {
      FORWARD(addCount(hashval,key,incr),nexttab,insert_forwarded) ;
      // scan the chain of items for this hash position
      if_THREADED(size_t deleted = NULLPOS ;)
      Link offset = chainHead(bucketnum) ;
      Link firstoffset = offset ;
      while (FramepaC::NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT key_at_pos = getKey(pos) ;
	 if (m_container->isEqual(key,key_at_pos))
	    {
	    // found existing entry!  Verify that we
	    //   haven't been superseded during the
	    //   search
	    if (superseded())
	       {
	       return addCount(hashval,key,incr) ;
	       }
	    ValT newcount = m_entries[pos].atomicIncrCount(incr) ;
	    INCR_COUNT(insert_dup) ;
	    return newcount ;
	    }
	 // advance to next item in chain
	 offset = chainNext(pos) ;
	 if_THREADED(if (key_at_pos == Entry::DELETED() && deleted == NULLPOS) { deleted = pos ; })
	 }
      // when we get here, we know that the item is not yet in the
      //   hash table, so try to add it
#ifndef FrSINGLE_THREADED
      // first, did we encounter a deleted entry? (can only happen if multithreaded)
      // If so, try to reclaim it
      if (deleted != NULLPOS)
	 {
	 if (unlikely(!bucketPtr(deleted)->markReclaiming()))
	    {
	    INCR_COUNT(CAS_coll) ;
	    continue ;		// someone else reclaimed the entry, so restart
	    }
	 // we managed to grab the entry for reclamation
	 // fill in the value, then the key
	 setValue(deleted,nullVal()+incr) ;
	 atomic_thread_fence(std::memory_order_release) ;
	 setKey(deleted,key) ;
	 bucketPtr(deleted)->markReclaimed() ;
	 INCR_COUNT(insert_attempt) ;
	 // Verify that we haven't been superseded while we
	 //   were working
	 FORWARD(addCount(hashval,key,incr),nexttab2,insert_forwarded) ;
	 break ;
	 }
#endif /* !FrSINGLE_THREADED */
      // otherwise, try to insert a new key/value entry
      if (insertKey(bucketnum,firstoffset,key,nullVal()+incr))
	 break ;
      }
   return nullVal()+incr ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::contains(size_t hashval, KeyT key) const
{
   size_t bucketnum = hashval % m_size ;
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(contains(hashval,key),contains_forwarded) ;
   INCR_COUNT(contains) ;
   // scan the chain of items for this hash position
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT key_at_pos = getKey(pos) ;
      if (m_container->isEqual(key,key_at_pos))
	 {
	 INCR_COUNT(contains_found) ;
	 return true ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   return false ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
ValT HashTable<KeyT,ValT>::Table::lookup(size_t hashval, KeyT key) const
{
   size_t bucketnum = hashval % m_size ;
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(lookup(hashval,key),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   // scan the chain of items for this hash position
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (m_container->isEqual(key,hkey))
	 {
	 ValT value = getValue(pos) ;
	 // double-check that a parallel remove() hasn't
	 //   deleted the entry while we were fetching the
	 //   value
	 if (getKey(pos) != hkey)
	    break ;
	 INCR_COUNT(lookup_found) ;
	 return value ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   return nullVal() ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::lookup(size_t hashval, KeyT key, ValT* value) const
{
   if (!value)
      return false ;
   size_t bucketnum = hashval % m_size ;
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(lookup(hashval,key,value),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   // scan the chain of items for this hash position
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (m_container->isEqual(key,hkey))
	 {
	 (*value) = getValue(pos) ;
	 INCR_COUNT(lookup_found) ;
	 // double-check that a parallel remove() hasn't deleted the
	 //   hash entry while we were working with it
	 return (getKey(pos) == hkey) ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   (*value) = nullVal() ;
   return false ;
}

//----------------------------------------------------------------------------

// NOTE: this lookup() is not entirely thread-safe if clear==true
template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::lookup(size_t hashval, KeyT key, ValT* value, bool clear_entry)
{
   if (!value)
      return false ;
   size_t bucketnum = hashval % m_size ;
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(lookup(hashval,key,value),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   // scan the chain of items for this hash position
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (m_container->isEqual(key,hkey))
	 {
	 (*value) = clear_entry ? m_entries[pos].swapValue(nullVal()) : getValue(pos) ;
	 INCR_COUNT(lookup_found) ;
	 // double-check that a parallel remove() hasn't deleted the
	 //   hash entry while we were working with it
	 return (getKey(pos) == hkey) ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   (*value) = nullVal() ;
   return false ;	// not found
}

//----------------------------------------------------------------------------

// note: lookupValuePtr is not safe in the presence of parallel
//   add() and remove() calls!  Use global synchronization if
//   you will be using both this function and add()/remove()
//   concurrently on the same hash table.
template <typename KeyT, typename ValT>
ValT* HashTable<KeyT,ValT>::Table::lookupValuePtr(size_t hashval, KeyT key) const
{
   size_t bucketnum = hashval % m_size ;
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(lookupValuePtr(hashval,key),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   // scan the chain of items for this hash position
   ValT* val = nullptr ; // assume "not found"
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT key_at_pos = getKey(pos) ;
      if (m_container->isEqual(key,key_at_pos))
	 {
	 val = getValuePtr(pos) ;
	 INCR_COUNT(lookup_found) ;
	 break ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   return val ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::remove(size_t hashval, KeyT key)
{
   size_t bucketnum = hashval % m_size ;
#ifdef FrSINGLE_THREADED
   // since nobody else can mess with the bucket chain while we're traversing it, we can
   //   just chop out the desired item and don't need to bother with marking entries as
   //   deleted and reclaiming them later
   INCR_COUNT(remove) ;
   Link* prevptr = chainHeadPtr(bucketnum) ;
   Link offset = Atomic<Link>::ref(*prevptr).load() ;
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT key_at_pos = getKey(pos) ;
      if (isEqual(key,key_at_pos))
	 {
	 // we found it!
	 // chop entry out of hash chain
	 *prevptr = chainNext(pos) ; //FIXME
	 // delete the item proper
	 bucketPtr(pos)->markFree() ;
	 replaceValue(pos,0) ;
	 INCR_COUNT(remove_found) ;
	 return true ;
	 }
      // advance to next item in chain
      prevptr = chainNextPtr(pos) ;
      offset = Atomic<Link>::ref(*prevptr).load() ;
      }
   // item not found
   return false ;
#else
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_STALE(remove(hashval,key),remove_forwarded) ;
   // deletions must be marked rather than removed since we
   //   can't CAS both the key field and the chain link at
   //   once
   // because add() could generate duplicates if there are
   //   concurrent insertions of the same item with multiple
   //   deleted records in the bucket's chain, we need to
   //   remove all occurrences of the key so that the delete
   //   doesn't make a replaced entry reappear
   bool success = false ;
   // scan the chain of items for this hash position
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT keyval = getKey(pos) ;
      if (m_container->isEqual(key,keyval))
	 {
	 ValT value = getValue(pos) ;
	 if (updateKey(pos,keyval,Entry::DELETED()))
	    {
	    INCR_COUNT(remove_found) ;
	    success = true ;
	    // we successfully marked the entry as deleted
	    //   before anybody else changed it, so now we
	    //   can free its value at our leisure by running
	    //   the removal hook if present
	    if (remove_fn && value)
	       {
	       remove_fn(keyval,value) ;
	       }
	    }
	 else
	    {
	    INCR_COUNT(CAS_coll) ;
	    }
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   return success ;
#endif /* FrSINGLE_THREADED */
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::reclaimDeletions(size_t totalfrags, size_t fragnum)
{
   if (superseded())
      return true ;
#ifdef FrSINGLE_THREADED
   (void)totalfrags ; (void)fragnum ;
   // when single-threaded, we chop out deletions
   //   immediately, so there is nothing to reclaim
   return false ;
#else
   if (totalfrags == 0) totalfrags = 1 ;
   size_t per_fragment = (m_fullsize / totalfrags) + 1 ;
   size_t start_bucket = fragnum * per_fragment ;
   size_t end_bucket = start_bucket + per_fragment ;
   if (end_bucket > m_fullsize) end_bucket = m_fullsize ;
   INCR_COUNT(reclaim) ;
   debug_msg("reclaimDeletions (thr %ld)\n",FramepaC::my_job_id) ;
   bool have_reclaimed = false ;
   for (size_t i = start_bucket ; i < end_bucket ; ++i)
      {
      // follow the chain for this hash bucket, chopping out any
      //   entries marked as deleted and resetting their status to
      //   Unused.
      bool reclaimed = reclaimChain(i) ;
      have_reclaimed |= reclaimed ;
      }
   debug_msg("reclaimDeletions done (thr %ld)\n",FramepaC::my_job_id) ;
   return have_reclaimed ;
#endif /* FrSINGLE_THREADED */
}

//----------------------------------------------------------------------------

// special support for Fr::SymHashSet
template <typename KeyT, typename ValT>
KeyT HashTable<KeyT,ValT>::Table::addKey(size_t hashval, const char* name, size_t namelen,
				    bool* already_existed)
{
   if (!already_existed)
      {
      // since only gensym() is interested in whether the key was already
      //   in the table and regular add() calls will usually be for
      //   symbols which already exist, try a fast-path lookup-only call
      KeyT key = lookupKey(hashval,name,namelen) ;
      if (key)
	 return key ;
      }
   size_t bucketnum = hashval % m_size ;
   while (true)
      {
      FORWARD(addKey(hashval,name,namelen,already_existed),nexttab,insert_forwarded) ;
      // scan the chain of items for this hash position
      Link offset = chainHead(bucketnum) ;
      Link firstoffset = offset ;
      while (FramepaC::NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT existing = getKey(pos) ;
	 if (isEqual(name,namelen,existing))
	    {
	    // found existing entry!
	    INCR_COUNT(insert_dup) ;
	    if (already_existed)
	       *already_existed = true ;
	    return existing ;
	    }
	 // advance to next item in chain
	 offset = chainNext(pos) ;
	 }
      // when we get here, we know that the item is not yet in the
      //   hash table, so try to add it
      KeyT key = m_container->createSymbol(name) ;
      if (key == nullKey()) return key ;
      // if the insertKey fails, someone else beat us to creating the
      //    symbol, so we delete the symbol we just created and then
      //    return the other one when we loop back to the top
      if (insertKey(bucketnum,firstoffset,key,nullVal()))
	 return key ;
      m_container->deleteSymbol(key) ;
      }
}

//----------------------------------------------------------------------------

// special support for Fr::SymHashSet
template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::contains(size_t hashval, const char* name, size_t namelen) const
{
   size_t bucketnum = hashval % m_size ;
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(contains(hashval,name,namelen),contains_forwarded) ;
   INCR_COUNT(contains) ;
   // scan the chain of items for this hash position
   bool success = false ;
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT key_at_pos = getKey(pos) ;
      if (isEqual(name,namelen,key_at_pos))
	 {
	 INCR_COUNT(contains_found) ;
	 success = true ;
	 break ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   return success ;
}

//----------------------------------------------------------------------------

// special support for Fr::SymHashSet
template <typename KeyT, typename ValT>
KeyT HashTable<KeyT,ValT>::Table::lookupKey(size_t hashval, const char* name, size_t namelen) const
{
   size_t bucketnum = hashval % m_size ;
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(lookupKey(hashval,name,namelen),contains_forwarded) ;
   INCR_COUNT(lookup) ;
   // scan the chain of items for this hash position
   while (FramepaC::NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (isEqual(name,namelen,hkey))
	 {
	 INCR_COUNT(lookup_found) ;
	 return hkey ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   return nullKey() ; // not found
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::Table::replaceValue(size_t pos, ValT new_value)
{
   ValT value = m_entries[pos].swapValue(new_value) ;
   if (remove_fn && value)
      {
      remove_fn(getKey(pos),value) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t HashTable<KeyT,ValT>::Table::countItems() const
{
   debug_msg("countItems\n") ;
   if (next())
      return next()->countItems() ;
   size_t count = 0 ;
   for (size_t i = 0 ; i < m_fullsize ; ++i)
      {
      if (activeEntry(i))
	 count++ ;
      }
   debug_msg("  -> %lu\n",count) ;
   return count ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t HashTable<KeyT,ValT>::Table::countItems(bool remove_dups)
{
   if (next())
      return next()->countItems(remove_dups) ;
   if (remove_dups)
      {
      for (size_t i = 0 ; i < m_fullsize ; ++i)
	 {
	 clearDuplicates(i) ;
	 }
      }
   return this->countItems() ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t HashTable<KeyT,ValT>::Table::countDeletedItems() const
{
   debug_msg("countDeletedItems\n") ;
   if (next())
      return next()->countDeletedItems() ;
   size_t count = 0 ;
   for (size_t i = 0 ; i < m_fullsize ; ++i)
      {
      if (bucketPtr(i)->inUse() && getKey(i) == Entry::DELETED())
	 count++ ;
      }
   debug_msg("  -> %lu\n",count) ;
   return count ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t HashTable<KeyT,ValT>::Table::bucket_size(size_t bucketnum) const
{
   Link status ;
   Link offset = chainHead(bucketnum,status) ;
   FORWARD_IF_COPIED(bucket_size(bucketnum),none) ;
   size_t len = 0 ;
   while (FramepaC::NULLPTR != offset)
      {
      len++ ;
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      }
   return len ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t* HashTable<KeyT,ValT>::Table::chainLengths(size_t &max_length) const
{
   if (next())
      return next()->chainLengths(max_length) ;
   size_t* lengths = new size_t[searchrange+2] ;
   for (size_t i = 0 ; i <= searchrange+1 ; i++)
      {
      lengths[i] = 0 ;
      }
   max_length = 0 ;
   for (size_t i = 0 ; i < m_fullsize ; ++i)
      {
      size_t len = bucket_size(i) ;
      if (len > max_length)
	 max_length = len ;
      lengths[len]++ ;
      }
   return lengths ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t* HashTable<KeyT,ValT>::Table::neighborhoodDensities(size_t &num_densities) const
{
   if (next())
      return next()->neighborhoodDensities(num_densities) ;
   size_t* densities = new size_t[searchrange+2] ;
   num_densities = searchrange+1 ;
   for (size_t i = 0 ; i <= searchrange+1 ; i++)
      {
      densities[i] = 0 ;
      }
   // count up the active neighbors of the left-most entry in the hash array
   size_t density = 0 ;
   for (size_t i = 0 ; i <= (size_t)searchrange && i < m_fullsize ; ++i)
      {
      if (activeEntry(i))
	 ++density ;
      }
   ++densities[density] ;
   for (size_t i = 1 ; i < m_fullsize ; ++i)
      {
      // keep a rolling count of the active neighbors around the current point
      // did an active entry come into range on the right?
      if (i + searchrange < m_fullsize && activeEntry(i+searchrange))
	 ++density ;
      // did an active entry go out of range on the left?
      if (activeEntry(i-1))
	 --density ;
      ++densities[density] ;
      }
   return densities ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::iterateVA(HashKeyValueFunc* func, std::va_list args) const
{
   bool success = true ;
   for (size_t i = 0 ; i < m_fullsize && success ; ++i)
      {
      if (!activeEntry(i))
	 continue ;
      ValT value = getValue(i) ;
      KeyT key = getKey(i) ;
      // if the item hasn't been removed while we were getting it,
      //   call the processing function
      if (key == Entry::DELETED())
	 continue ;
      std::va_list argcopy ;
      va_copy(argcopy,args) ;
      success = func(key,value,argcopy) ;
      va_end(argcopy) ;
      }
   return success ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
Object* HashTable<KeyT,ValT>::Table::makeObject(KeyT key)
{
   return (Object*)key ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
List* HashTable<KeyT,ValT>::Table::allKeys() const
{
   List* keys = nullptr ;
   for (size_t i = 0 ; i < m_fullsize ; ++i)
      {
      if (activeEntry(i))
	 {
	 KeyT name = m_entries[i].copyName() ;
	 pushlist(makeObject(name),keys) ;
	 }
      }
   return keys ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::Table::verify() const
{
   bool success = true ;
   for (size_t i = 0 ; i < m_fullsize ; ++i)
      {
      KeyT key = getKey(i) ;
      if (key != Entry::DELETED() && !contains(m_container->hashVal(key),key))
	 {
	 debug_msg("verify: missing @ %ld\n",i) ;
	 success = false ;
	 break ;
	 }
      }
   return success ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
ostream &HashTable<KeyT,ValT>::Table::printKeyValue(ostream &output, KeyT key) const
{
   return output << key ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
size_t HashTable<KeyT,ValT>::Table::keyDisplayLength(KeyT key) const
{
   return key ? key->cStringLength() + 1 : 3 ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
char* HashTable<KeyT,ValT>::Table::displayKeyValue(char* buffer, KeyT key) const
{
   if (key)
      {
      size_t len = key->cStringLength() ;
      if (key->toCstring(buffer,len))
	 buffer += len ;
      *buffer = '\0' ;
      return buffer ;
      }
   else
      {
      *buffer++ = 'N' ;
      *buffer++ = 'I' ;
      *buffer++ = 'L' ;
      *buffer = '\0' ;
      return  buffer ;
      }
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
ostream &HashTable<KeyT,ValT>::Table::printValue(ostream &output) const
{
   output << "#H(" ;
   size_t orig_indent = FramepaC::initial_indent ;
   FramepaC::initial_indent += 3 ; //strlen("#H(")
   size_t loc = FramepaC::initial_indent ;
   bool first = true ;
   for (size_t i = 0 ; i < m_fullsize ; ++i)
      {
      KeyT key = getKey(i) ;
      if (key == Entry::DELETED())
	 continue ;
      size_t len = keyDisplayLength(key) ;
      loc += len ;
      if (loc > FramepaC_display_width && !first)
	 {
	 output << '\n' << setw(FramepaC::initial_indent) << " " ;
	 loc = FramepaC::initial_indent + len ;
	 }
      output << key << ' ' ;
      first = false ;
      }
   output << ")" ;
   FramepaC::initial_indent = orig_indent ;
   return output ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
char* HashTable<KeyT,ValT>::Table::displayValue(char* buffer) const
{
   strcpy(buffer,"#H(") ;
   buffer += 3 ;
   for (size_t i = 0 ; i < m_fullsize ; ++i)
      {
      KeyT key = getKey(i) ;
      if (key == Entry::DELETED())
	 continue ;
      buffer = displayKeyValue(buffer,key) ;
      *buffer++ = ' ' ;
      }
   *buffer++ = ')' ;
   *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

// get size of buffer needed to display the string representation of the hash table
// NOTE: will not be valid if there are any add/remove/resize calls between calling
//   this function and using displayValue(); user must ensure locking if multithreaded
template <typename KeyT, typename ValT>
size_t HashTable<KeyT,ValT>::Table::cStringLength(size_t wrap_at, size_t indent) const
{
   if (wrap_at == 0) wrap_at = (size_t)~0 ;
   size_t dlength = indent + 4 ; // "#H(" prefix plus ")" trailer
   size_t currline = dlength ;
   for (size_t i = 0 ; i < m_fullsize ; i++)
      {
      KeyT key = getKey(i) ;
      if (key == Entry::DELETED())
	 continue ;
      size_t len = keyDisplayLength(key) + 1 ;
      dlength += len ;
      currline += len ;
      if (currline > wrap_at)
	 {
	 currline = 0 ;
	 dlength += indent ;
	 }
      }
   return dlength ;
}

//----------------------------------------------------------------------------

/************************************************************************/
/*	Methods for class HashTable					*/
/************************************************************************/

template <typename KeyT, typename ValT>
HashTable<KeyT,ValT>::HashTable(const HashTable &ht)
   : HashTableBase(), m_table(nullptr)
{
   if (&ht == nullptr)
      return ;
   init(ht.maxSize()) ;
   Table *table = m_table.load() ;
   Table *othertab = ht.m_table.load() ;
   for (size_t i = 0 ; i < table->m_fullsize ; i++)
      {
      table->copyEntry(i,othertab) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::init(size_t initial_size, Table* table)
{
   onRemove(nullptr) ;
   onDelete(nullptr) ;
   m_table.store(nullptr) ;
   m_oldtables.store(nullptr) ;
   m_freetables.store(nullptr) ;
   clearGlobalStats() ;
   initial_size = Table::normalizeSize(initial_size) ;
   for (size_t i = 0 ; i < FrHT_NUM_TABLES ; i++)
      {
      releaseTable(&m_tables[i]) ;
      }
   if (!table)
      {
      table = allocTable() ;
      }
   new (table) Table(initial_size) ;
   if (table->good())
      {
      table->m_container = this ;
      table->remove_fn = onRemoveFunc() ;
      m_table.store(table) ;
      m_oldtables.store(table) ;
      }
   else
      {
      SystemMessage::no_memory("creating HashTable") ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
HashTable<KeyT,ValT>::~HashTable()
{
   Table *table = m_table.load() ;
   if (table && table->good())
      {
      if (cleanup_fn)
	 {
	 for (size_t i = 0 ; i < table->m_fullsize ; ++i)
	    {
	    if (table->activeEntry(i))
	       {
	       cleanup_fn(this,table->getKey(i),table->getValue(i)) ;
	       }
	    }
	 cleanup_fn = nullptr ;
	 }
      if (remove_fn)
	 {
	 for (size_t i = 0 ; i < table->m_fullsize ; ++i)
	    {
	    if (table->activeEntry(i))
	       {
	       table->replaceValue(i,nullVal()) ;
	       }
	    }
	 }
      atomic_thread_fence(std::memory_order_seq_cst) ; 
      debug_msg("HashTable dtor\n") ;
      }
   table->clear() ;
   remove_fn = nullptr ;
   freeTables() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::stillLive(const Table* version)
{
#ifndef FrSINGLE_THREADED
   // scan the list of per-thread s_table variables
   for (const TablePtr *tables = Atomic<TablePtr*>::ref(s_thread_entries).load() ;
	tables ;
	tables = ANNOTATE_UNPROTECTED_READ(tables->m_next))
      {
      // check whether the hazard pointer is for the requested version
      if (tables->table() == version)
	 {
	 // yep, the requested version is still live
	 return true ;
	 }
      }
#endif /* !FrSINGLE_THREADED */
   (void)version ;
   // nobody else is using that version
   return false ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::thread_backoff(size_t &loops)
{
   ++loops ;
   // we expect the pending operation(s) to complete in
   //   well under one microsecond, so start by just spinning
   if (loops < FrSPIN_COUNT)
      {
      INCR_COUNT(spin) ;
      _mm_pause() ;
      _mm_pause() ;
      }
   else if (loops < FrSPIN_COUNT + FrYIELD_COUNT)
      {
      // it's taking a little bit longer, so now yield the
      //   CPU to any suspended but ready threads
      //debug_msg("yield\n") ;
      INCR_COUNT(yield) ;
      std::this_thread::yield() ;
      }
   else
      {
      // hmm, this is taking a while -- maybe an add() had
      // to do extra work, or we are oversubscribed on
      // threads and an operation got suspended
//      debug_msg("sleep\n") ;
      INCR_COUNT(sleep) ;
      size_t factor = loops - FrSPIN_COUNT - FrYIELD_COUNT + 1 ;
      std::this_thread::sleep_for(std::chrono::microseconds(factor * FrNAP_TIME)) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::Table* HashTable<KeyT,ValT>::allocTable()
{
   // pop a table record off the freelist, if available
   Table* tab = m_freetables.load() ;
   while (tab)
      {
      Table* nxt = tab->nextFree() ;
      if (m_freetables.compare_exchange_strong(tab,nxt))
	 return tab ;
      tab = m_freetables.load() ;
      }
   // no table records on the freelist, so create a new one
   return new Table ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::releaseTable(Table* t)
{
   t->clear() ;
   Table* freetab ;
   do
      {
      freetab = m_freetables.load() ;
      t->m_next_free.store(freetab) ;
      } while (!m_freetables.compare_exchange_weak(freetab,t)) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::freeTables()
{
   Table* tab ;
   while ((tab = m_freetables.load()) != nullptr)
      {
      Table* nxt = tab->nextFree() ;
      m_freetables.store(nxt) ;
      if (tab < &m_tables[0] || tab >= &m_tables[FrHT_NUM_TABLES])
	 {
	 // not one of the tables inside our own instance, so
	 //   send back to OS
	 delete tab ;
	 }
      }
   return  ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::updateTable()
{
   Table* table = m_table.load() ;
   bool updated = false ;
   while (table && table->resizingDone() && table->next())
      {
      table = table->next() ;
      updated = true ;
      }
   if (updated)
      {
      m_table.store(table) ;
      }
   return ;
}

//----------------------------------------------------------------------------
// set up the per-thread info needed for safe reclamation of
//   entries and hash arrays, as well as an on-exit callback
//   to clear that info

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::threadInit()
{
#ifndef FrSINGLE_THREADED
   // check whether we've initialized the thread-local data yet
#ifdef FrHASHTABLE_STATS
   if (!s_stats) s_stats = new HashTable_Stats ;
   s_stats->clear() ;
#endif /* FrHASHTABLE_STATS */
   if (!s_thread_record) s_thread_record = new TablePtr ;
   if (!s_thread_record->initialized())
      {
      // push our local-copy variable onto the list of all such variables
      //   for use by the resizer
      auto head = Atomic<TablePtr*>::ref(s_thread_entries).load() ;
      do {
         s_thread_record->init(&s_table,head) ;
         } while (!Atomic<TablePtr*>::ref(s_thread_entries).compare_exchange_weak(head,s_thread_record)) ;
      }
#endif /* FrSINGLE_THREADED */
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
void HashTable<KeyT,ValT>::threadCleanup()
{
#ifndef FrSINGLE_THREADED
   s_table = nullptr ;
   storeBarrier() ;
   if (s_thread_record->initialized())
      {
      // unlink from the list of all thread-local table pointers
      TablePtr* prev = nullptr ;
      TablePtr* curr = s_thread_entries ;
      while (curr && curr->m_table != &Atomic<Table*>::ref(s_table))
	 {
	 prev = curr ;
	 curr = curr->m_next ;
	 }
      if (curr)
	 {
	 // found a match, so unlink it
	 if (prev)
	    prev->m_next = curr->m_next ;
	 else
	    s_thread_entries = curr->m_next ;
	 }
      s_thread_record->clear() ;
      }
   delete s_thread_record ;
   s_thread_record = nullptr ;
#ifdef FrHASHTABLE_STATS
   delete s_stats ;
   s_stats = nullptr ;
#endif /* FrHASHTABLE_STATS */
#endif /* !FrSINGLE_THREADED */
   return ;
}

//----------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::assistResize()
{
   Table *tab = m_oldtables.load() ;
   while (tab && tab->next())
      {
      if (!tab->resizingDone())
	 {
	 tab->resizeCopySegments(4) ;
	 }
      tab = tab->next() ;
      }
   tab = m_oldtables.load() ;
   if (tab->resizingDone() && !stillLive(tab))
      {
      Table* nxt = tab->next() ;
      if (m_oldtables.compare_exchange_strong(tab,nxt))
	 {
	 releaseTable(tab) ;
	 }
      }
   return m_oldtables.load() != m_table.load() ;
}

/************************************************************************/
/*	Declarations for template class HashTable			*/
/************************************************************************/

//----------------------------------------------------------------------
// static members

#ifndef FrSINGLE_THREADED
template <typename KeyT, typename ValT>
typename HashTable<KeyT,ValT>::TablePtr* HashTable<KeyT,ValT>::s_thread_entries = nullptr ;
template <typename KeyT, typename ValT>
thread_local typename HashTable<KeyT,ValT>::Table* HashTable<KeyT,ValT>::s_table = nullptr ;
template <typename KeyT, typename ValT>
thread_local typename HashTable<KeyT,ValT>::TablePtr* HashTable<KeyT,ValT>::s_thread_record = nullptr ;
#endif /* !FrSINGLE_THREADED */

#if defined(FrHASHTABLE_STATS)
template <typename KeyT, typename ValT>
thread_local FramepaC::HashTable_Stats* HashTable<KeyT,ValT>::s_stats = nullptr ;
#endif /* FrHASHTABLE_STATS */

} // end namespace Fr


#undef INCR_COUNT
#undef FORWARD
#undef FORWARD_IF_COPIED
#undef FORWARD_IF_STALE

// end of file frhash.h //
