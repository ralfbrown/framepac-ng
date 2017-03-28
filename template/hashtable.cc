/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
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

#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include "framepac/hashtable.h"

#if DYNAMIC_ANNOTATIONS_ENABLED != 0
#  include "dynamic_annotations.h"
#endif /* DYNAMIC_ANNOTATIONS_ENABLED */

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#if defined(FrSINGLE_THREADED)
#define FORWARD(delegate,tab,counter)		\
      Table *tab = next() ;			\
      if (tab /*&& chainIsStale(bucketnum)*/)	\
	 {					\
	 return tab->delegate ;			\
	 }							
#define FORWARD_IF_COPIED(delegate,counter)
#define FORWARD_IF_STALE(delegate,counter)
#else
#define FORWARD(delegate,tab,counter)					\
      Table *tab = next() ;						\
      if (tab /*&& chainIsStale(bucketnum)*/)				\
	 {								\
	 /* ensure that our bucket has been copied to 	*/		\
	 /*   the successor table, then add the key to	*/		\
	 /*   that table					*/	\
	 resizeCopySegments() ;						\
	 waitUntilCopied(bucketnum) ;					\
	 INCR_COUNT(counter) ;						\
	 tab->announceTable() ;						\
	 return tab->delegate ;						\
	 }							
#define FORWARD_IF_COPIED(delegate,counter)				\
      Table *nexttab = next() ;						\
      if (nexttab)							\
	 {								\
	 /* if our bucket has been fully copied to the	*/ 		\
	 /*   successor table, look for the key in that	*/		\
	 /*   table instead of the current one		*/ 		\
	 /* FIXME: do we want to do any copying to help out the resize?*/ \
	 if (chainCopied(bucketnum))					\
	    {								\
	    INCR_COUNT(counter) ;					\
	    nexttab->announceTable() ;					\
	    return nexttab->delegate ;					\
	    }								\
	 }
#define FORWARD_IF_STALE(delegate,counter)				\
      Table *nexttab = next() ;						\
      if (nexttab)							\
	 {								\
	 /* if our bucket has at least started to be copied to	*/	\
	 /*   the successor table, look for the key in that	*/	\
	 /*   table instead of the current one			*/ 	\
	 /* FIXME: do we want to do any copying to help out the resize?*/ \
	 if (chainIsStale(bucketnum))					\
	    {								\
	    INCR_COUNT(counter) ;					\
	    waitUntilCopied(bucketnum) ;				\
	    nexttab->announceTable() ;					\
	    return nexttab->delegate ;					\
	    }								\
	 }
#endif /* FrSINGLE_THREADED */

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

void Table::init(size_t size, double max_fill)
{
   m_size = size ;
   m_currsize.store(0) ;
   m_next_table.store(0) ;
   m_next_free.store(0) ;
   m_entries = 0 ;
   ifnot_INTERLEAVED(m_ptrs = 0 ;)
      m_resizelock.store(false) ;
   m_resizedone.store(false) ;
   m_resizestarted.clear() ;
   m_resizepending.clear() ;
   m_resizethresh = (size_t)(size * max_fill + 0.5) ;
   size_t num_segs = (m_size + FrHASHTABLE_SEGMENT_SIZE - 1) / FrHASHTABLE_SEGMENT_SIZE ;
   m_resizepending.init(num_segs) ;
   m_segments_assigned.store((size_t)0) ;
   m_segments_total.store(num_segs) ;
   m_first_incomplete.store(size) ;
   m_last_incomplete.store(0) ;
   remove_fn = 0 ;
   if (size > 0)
      {
      m_entries = FrNewN(Entry,size) ;
      ifnot_INTERLEAVED(m_ptrs = FrNewN(HashPtr,size) ;)
	 if (m_entries
	     ifnot_INTERLEAVED(&& m_ptrs)
	    )
	    {
	    for (size_t i = 0 ; i < size ; i++)
	       {
	       m_entries[i].init() ;
	       ifnot_INTERLEAVED(m_ptrs[i].init() ;)
		  }
	    }
	 else
	    {
	    FrFree(m_entries) ;
	    ifnot_INTERLEAVED(FrFree(m_ptrs) ;)
	       m_entries = 0 ;
	    ifnot_INTERLEAVED(m_ptrs = 0 ;)
	       m_size = 0 ;
	    }
      }
   return ;
}

//----------------------------------------------------------------------------

void Table::clear()
{
   FrFree(m_entries) ;
   ifnot_INTERLEAVED(FrFree(m_ptrs) ;)
      m_entries = 0 ;
   ifnot_INTERLEAVED(m_ptrs = 0 ;)
      m_currsize.store(0) ;
   m_next_table.store(0) ;
   m_next_free.store(0) ;
   return ;
}

//----------------------------------------------------------------------------

size_t Table::normalizeSize(size_t sz)
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
   for (size_t i = 0 ; FramepaC_small_primes[i] ; ++i)
      {
      if (sz % FramepaC_small_primes[i] == 0)
	 sz += 2 ;
      }
   } while (sz != pre_bump) ;
   return sz ;
}

//----------------------------------------------------------------------------

void Table::autoResize()
{
   size_t currsize = m_currsize.load() ;
   if (currsize <= m_resizethresh)
      {
      // force an increase if we ran out of free slots
      //   within range of a bucket
      size_t sz = m_size ;
#ifdef FrSAVE_MEMORY
      if (sz < 100000)
	 currsize = 2.0*sz ;
      else if (sz < 1000000)
	 currsize = 1.5*sz ;
      else if (sz < 10*1000*1000)
	 currsize = 1.4*sz ;
      else if (sz < 100*1000*1000)
	 currsize = 1.3*sz ;
      else
	 currsize = 1.2*sz ;
#else
      if (sz < 16*1000*1000)
	 currsize = 2.0*sz ;
      else
	 currsize = 1.5*sz ;
#endif /* FrSAVE_MEMORY */
      }
   else
      {
      currsize *= 2 ;
      }
   resize(currsize,true) ;
   return ;
}

//----------------------------------------------------------------------------

bool Table::bucketsInUse(size_t startbucket, size_t endbucket) const
{
#ifndef FrSINGLE_THREADED
   // can only have concurrent use when multi-threaded
   // scan the list of per-thread s_table variables
   for (const TablePtr *tables = cs::load(s_thread_entries) ; tables ; tables = tables->m_next)
      {
      // check whether the hazard pointer is for ourself
      const Table *ht = tables->table() ;
      if (this != ht)
	 continue ;
      // check whether it's for a bucket of interest
      size_t bucket = tables->bucket() ;
      if (bucket >= startbucket && bucket < endbucket)
	 return true ;
      }
#endif
   return false ; 
}

//----------------------------------------------------------------------------

void Table::awaitIdle(size_t bucketnum, size_t endpos)
{
   // wait until nobody is announcing that they are
   //   using one of our hash buckets
#ifndef FrSINGLE_THREADED
   debug_msg("await idle %ld\n",my_job_id);
   size_t loops = 0 ;
   while (bucketsInUse(bucketnum,endpos))
      {
      thread_backoff(loops) ;
      if (superseded())
	 {
	 debug_msg(" idle wait canceled %ld\n",my_job_id) ;
	 return ;
	 }
      }
   debug_msg("is idle %ld\n",my_job_id);
#endif /* !FrSINGLE_THREADED */
   (void)bucketnum ;
   (void)endpos ;
   return ;
}

//----------------------------------------------------------------------------

bool Table::copyChainLocked(size_t bucketnum) _fnattr_hot
{
   Link offset ;
#ifndef FrSINGLE_THREADED
   // since add() can still run in parallel, we need to
   //   start by making any deleted entries in the
   //   chain unusable by add(), so that it doesn't
   //   re-use them after we've skipped them during the
   //   copy loop
   offset = chainHead(bucketnum) ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      (void)updateKey(pos,Entry::DELETED(),Entry::RECLAIMING()) ;
      }
   // the chain is now frozen -- add() can't push to
   //   the start or re-use anything on the chain
#endif /* !FrSINGLE_THREADED */
   // insert all elements of the current hash bucket into the next table
   offset = chainHead(bucketnum) ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      // insert the element, ignoring it if it is deleted or a duplicate
      if (!activeEntry(pos))
	 {
	 continue ;
	 }
      KeyT key = getKey(pos) ;
      size_t hashval = hashValFull(key) ;
      Entry *element = &m_entries[pos] ;
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

bool Table::copyChain(size_t bucketnum) _fnattr_hot
{
#ifndef FrSINGLE_THREADED
   HashPtr *bucket = bucketPtr(bucketnum) ;
   // atomically set the 'stale' bit and get the current status
   uint8_t status = bucket->markStaleGetStatus() ;
   if ((status & (HashPtr::stale_mask | HashPtr::lock_mask)) != 0)
      {
      if (status & HashPtr::lock_mask) { INCR_COUNTstat(chain_lock_coll) ; }
      // someone else has already worked on this
      //   bucket, or a copy/recycle()/hopscotch() is
      //   currently running, in which case that thread
      //   will do the copying for us
      return bucket->copyDone() ;
      }
#endif /* !FrSINGLE_THREADED */
   return copyChainLocked(bucketnum) ;
}

//----------------------------------------------------------------------------

void Table::waitUntilCopied(size_t bucketnum)
{
   size_t loops = 0 ;
   while (!chainCopied(bucketnum))
      {
      thread_backoff(loops) ;
      }
   return  ;
}

//----------------------------------------------------------------------------

void Table::copyChains(size_t bucketnum, size_t endpos) _fnattr_hot
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
   // if we had to skip copying any chains due to a
   //    concurrent recycle() or hopscotch(), tell the
   //    lead resizing thread that it need to do a
   //    cleanup pass
   if (!complete)
      {
      size_t old_first ;
      do 
	 {
	 old_first = m_first_incomplete.load() ;
	 if (first_incomplete >= old_first)
	    break ;
	 } while (!m_first_incomplete.compare_exchange_strong(old_first,first_incomplete)) ;
      size_t old_last ;
      do
	 {
	 old_last = m_last_incomplete.load() ;
	 if (last_incomplete <= old_last)
	    break ;
	 } while (!m_last_incomplete.compare_exchange_strong(old_last,last_incomplete)) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool Table<KeyT,ValT>::reAdd(size_t hashval, KeyT key, ValT value = 0)
{
   INCR_COUNT(insert_resize) ;
   DECR_COUNT(insert_attempt) ; // don't count as a retry unless we need more than one attempt
   size_t bucketnum = hashval % m_size ;
   while (true)
      {
      FORWARD(reAdd(hashval,key,value),nexttab,insert_forwarded) ;
      // tell others we're using the bucket
      announceBucketNumber(bucketnum) ;
      // scan the chain of items for this hash position
      Link offset = chainHead(bucketnum) ;
      Link firstoffset = offset ;
      while (NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT key_at_pos = getKey(pos) ;
	 if (isEqualFull(key,key_at_pos))
	    {
#if FrHASHTABLE_VERBOSITY > 1
	    cerr << "reAdd skipping duplicate " << key << " [" << hex << (size_t)key
		 << "] - " << key_at_pos << " [" << ((size_t)key_at_pos)
		 << "]" << dec << endl ;
#endif /* FrHASHTABLE_VERBOSITY > 1 */
	    // found existing entry!
	    INCR_COUNT(insert_dup) ;
	    unannounceBucketNumber() ;
	    return true ;		// item already exists
	    }
	 // advance to next item in chain
	 offset = chainNext(pos) ;
	 }
      // when we get here, we know that the item is not yet in the
      //   hash table, so try to add it
      if (insertKey(bucketnum,firstoffset,key,value))
	 break ;
      }
   unannounceBucketNumber() ;
   return false ;		// item did not already exist
}

//----------------------------------------------------------------------------

bool Table::claimEmptySlot(size_t pos, KeyT key) _fnattr_hot
{
#ifdef FrSINGLE_THREADED
   if (unusedEntry(pos))
      {
      setKey(pos,key) ;
      return true ;
      }
#else
   if (getKeyNonatomic(pos) == Entry::UNUSED())
      {
      // check if someone stole the free slot out from under us
      return updateKey(pos,Entry::UNUSED(),key) ;
      }
#endif /* FrSINGLE_THREADED */
   return false ;
}

//----------------------------------------------------------------------------

Link Table::locateEmptySlot(size_t bucketnum, KeyT key, bool &got_resized) _fnattr_hot
{
   if (superseded())
      {
      // a resize snuck in, so caller MUST retry
      got_resized = true ;
      return NULLPTR ;
      }
   // is the given position free?
   if (expected(claimEmptySlot(bucketnum,key)))
      return 0 ;
   size_t sz = m_size ;
   // compute the extent of the cache line containing
   //   the offset-0 entry for the bucket
   uintptr_t CLstart_addr = ((uintptr_t)(bucketPtr(bucketnum))) & ~(Fr_cacheline_size-1) ;
   if (CLstart_addr < (uintptr_t)bucketPtr(0))
      CLstart_addr = (uintptr_t)bucketPtr(0) ;
   size_t itemsize = (uintptr_t)bucketPtr(1) - (uintptr_t)bucketPtr(0) ;
   size_t CL_start = (CLstart_addr - (uintptr_t)bucketPtr(0)) / itemsize ;
   size_t CL_end = (CLstart_addr - (uintptr_t)bucketPtr(0) + Fr_cacheline_size) / itemsize ;
   // search for a free slot on the same cache line as the bucket header
   for (size_t i = CL_start ; i < CL_end && i < sz ; ++i)
      {
      if (i == bucketnum)
	 continue ;
      if (claimEmptySlot(i,key))
	 return (i - bucketnum) ;
      }
   // extended search for a free spot following the given position
   size_t max = bucketnum + searchrange ;
   if (max >= sz)
      {
      max = sz - 1 ;
      }
   for (size_t i = max ; i >= CL_end  ; --i)
      {
      if (claimEmptySlot(i,key))
	 return (i - bucketnum) ;
      }
   // search for a free spot preceding the given position
   size_t min = bucketnum >= (size_t)searchrange ? bucketnum - searchrange : 0 ;
   for (size_t i = min ; i < CL_start ; ++i)
      {
      if (claimEmptySlot(i,key))
	 return (i - bucketnum) ;
      }
   if (superseded())
      {
      // a resize snuck in, so caller MUST retry
      got_resized = true ;
      return NULLPTR ;
      }
   INCR_COUNT(neighborhood_full) ;
   // if we get here, we didn't get a free slot, so
   //    try recycling a deleted slot in the search
   //    window
   size_t recycled = recycleDeletedEntry(bucketnum,key) ;
   if (NULLPOS != recycled)
      {
      return recycled - bucketnum ;
      }
   // that still didn't get us a slot, so try moving
   //    an entry on someone else's chain to a free
   //    slot outside our reach
   size_t hopped = hopscotch(bucketnum) ;
   if (NULLPOS != hopped)
      {
      setKey(hopped,key) ;
      return hopped - bucketnum ;
      }
   // if we get here, there were no free slots available, and
   //   none could be made
   return NULLPTR ;
}

//----------------------------------------------------------------------------

bool Table::insertKey(size_t bucketnum, Link firstptr, KeyT key, ValT value) _fnattr_hot
{
   if (superseded())
      {
      // a resize snuck in, so retry
      return false ;
      }
   if (unlikely(m_currsize.load() > m_resizethresh))
      {
      autoResize() ;
      return false ;
      }
   INCR_COUNT(insert_attempt) ;
   bool got_resized = false ;
   Link offset = locateEmptySlot(bucketnum,key,got_resized) ;
   if (unlikely(NULLPTR == offset))
      {
      if (!got_resized)
	 autoResize() ;
      return false ;
      }
   // fill in the slot we grabbed and link it to the head of
   //   the chain for the hash bucket
   size_t pos = bucketnum + offset ;
   setValue(pos,value) ;
   setChainOwner(pos,offset) ;
#ifdef FrSINGLE_THREADED
   *chainNextPtr(pos) = firstptr ;
   // life is much simpler in non-threaded mode: just point
   //   the chain head at the new node and increment the
   //   tally of items in the table
   *chainHeadPtr(bucketnum) = offset ;
#else
   setChainNext(pos,firstptr) ;
   // now that we've done all the preliminaries, try to get
   //   write access to actually insert the new entry
   // try to point the hash chain at the new entry
   HashPtrHead expected_head ;
   HashPtrHead new_head ;
   expected_head.first = firstptr ;
   expected_head.status = 0 ;
   new_head.first = offset ;
   new_head.status = 0 ;
   // we need to "cast" a bit of black magic here so that we
   //   can CAS both the .first and .status fields in the
   //   HashPtr, since we require not only that the link
   //   be unchanged, but that the chain be neither stale
   //   nor locked
   HashPtrInt *headptr = (HashPtrInt*)&bucketPtr(bucketnum)->head ;
   if (unlikely(!cs::compareAndSwap(&headptr->head_int,
				    *((uint16_t*)&expected_head),
				    *((uint16_t*)&new_head))))
      {
      // oops, someone else messed with the hash chain, which
      //   means there could have been a parallel insert of
      //   the same value, or a resize is in progress and
      //   copied the bucket while we were working
      // release the slot we grabbed and tell caller to retry
      setKey(pos,Entry::UNUSED()) ;
      INCR_COUNT(CAS_coll) ;
      debug_msg("insertKey: CAS collision\n") ;
      return false ;
      }
#endif /* FrSINGLE_THREADED */
   m_currsize++ ;
   return true ;
}

//----------------------------------------------------------------------------

void Table::resizeCopySegment(size_t segnum) _fnattr_hot
{
   size_t bucketnum = segnum * FrHASHTABLE_SEGMENT_SIZE ;
   size_t endpos = (segnum + 1) * FrHASHTABLE_SEGMENT_SIZE ;
   if (endpos > m_size)
      endpos = m_size ;
   copyChains(bucketnum,endpos-1) ;
   // record the fact that we copied a segment
   m_resizepending.consume() ;
   return ;
}

//----------------------------------------------------------------------------

void Table::resizeCopySegments(size_t max_segs = ~0UL) _fnattr_hot
{
   // is there any work available to be stolen?
   if (!m_resizelock.load() || m_resizedone.load())
      return ;
   // grab the current segment number and increment it
   //   so the next thread gets a different number; stop
   //   once all segments have been assigned
   while (max_segs > 0 && m_segments_assigned.load() < m_segments_total.load())
      {
      size_t segnum ;
      if ((segnum = m_segments_assigned++) < m_segments_total.load())
	 {
	 resizeCopySegment(segnum) ;
	 --max_segs ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void Table::clearDuplicates(size_t bucketnum)
{
   // scan down the chain for the given hash bucket, marking
   //   any duplicate entries for a key as deleted
   announceBucketNumber(bucketnum) ;
   Link offset = chainHead(bucketnum) ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      KeyT currkey = getKey(pos) ;
      if (!isActive(currkey))
	 continue ;
      Link nxt = offset ;
      while (NULLPTR != nxt)
	 {
	 size_t nextpos = bucketnum + nxt ;
	 nxt = chainNext(nextpos) ;
	 KeyT nextkey = getKey(nextpos) ;
	 if (isActive(nextkey) && isEqualFull(currkey,nextkey))
	    {
	    // duplicate, apply removal function to the value
	    removeValue(m_entries[nextpos]) ;
	    setKey(nextpos,Entry::DELETED()) ;
	    }
	 }
      }
   unannounceBucketNumber() ;
   return ;
}

//----------------------------------------------------------------------------

bool Table::relocateEntry(size_t from, size_t to, size_t bucketnum)
{
   announceBucketNumber(bucketnum) ;
   // copy the contents of the 'from' entry to the 'to' entry
   setKey(to,getKey(from)) ;
   setValue(to,getValue(from)) ;
   setChainNext(to,chainNext(from)) ;
//FIXME
   INCR_COUNT(move) ;
   // try to swap the predecessor link for 'from' over to 'to'

   unannounceBucketNumber() ;
   return false ; 
}

//----------------------------------------------------------------------------

size_t Table::hopscotchChain(size_t bucketnum, size_t entrynum)
{
   // try to move an entry within the search range of
   //   'bucketnum' which is contained in another
   //   bucket's chain to a position outside the search
   //   range
#if !defined(FrSINGLE_THREADED) && 0
   // the following lock serializes to allow only one
   //   recycle() or hopscotch() on the chain at a time;
   //   concurrent add() and remove() are OK
   ScopedChainLock lock(this,bucketnum) ;
   if (lock.busy())
      {
      // someone else is currently reclaiming the chain or
      //   moving elements to make room, so we can just wait
      //   for them to finish and then claim success ourselves
      INCR_COUNT(chain_lock_coll) ;
      return NULLPOS ;
      }
#endif /* !FrSINGLE_THREADED */
   // find an entry within the search range of
   //   'bucketnum' which is in the chain for a bucket
   //   that can reach 'entrynum' and move it there
   if (entrynum > bucketnum)
      {
      size_t max = bucketnum + searchrange ;
      if (max >= m_size)
	 max = m_size - 1 ;
      for (size_t i = max ; i >= bucketnum ; --i)
	 {
	 size_t buck = bucketContaining(i) ;
	 if (NULLPOS == buck || buck + searchrange < entrynum || entrynum + searchrange < buck )
	    continue ;
	 if (relocateEntry(i,entrynum,buck))
	    return i ;
	 }
      }
   else
      {
      size_t min = bucketnum > (size_t)searchrange ? bucketnum-searchrange : 0 ;
      for (size_t i = min ; i <= bucketnum ; ++i)
	 {
	 size_t buck = bucketContaining(i) ;
	 if (NULLPOS == buck || buck + searchrange < entrynum || entrynum + searchrange < buck )
	    continue ;
	 if (relocateEntry(i,entrynum,buck))
	    return i ;
	 }
      }
   return NULLPOS ;
}

//----------------------------------------------------------------------------

size_t Table::hopscotch(size_t bucketnum)
{
   return NULLPOS;//FIXME
   size_t max = bucketnum + 2 * searchrange ;
   if (max > m_size)
      max = m_size ;
   for (size_t i = bucketnum + searchrange ; i < max ; ++i)
      {
      if (claimEmptySlot(i,Entry::RECLAIMING()))
	 {
	 size_t slot = hopscotchChain(bucketnum,i) ;
	 if (slot != NULLPOS)
	    {
	    return slot ;
	    }
	 else
	    {
	    // release the slot we grabbed, since we
	    //   were unable to use it
	    setKey(i,Entry::UNUSED()) ;
	    }
	 }
      }
   size_t min = 0 ;
   if (bucketnum > 2 * searchrange)
      {
      min = bucketnum - 2 * searchrange ;
      }
   for (size_t i = min ; i + searchrange < bucketnum ; ++i)
      {
      if (claimEmptySlot(i,Entry::RECLAIMING()))
	 {
	 size_t slot = hopscotchChain(bucketnum,i) ;
	 if (slot != NULLPOS)
	    {
	    return slot ;
	    }
	 else
	    {
	    // release the slot we grabbed, since we
	    //   were unable to use it
	    setKey(i,Entry::UNUSED()) ;
	    }
	 }
      }
   return NULLPOS ;
}

//----------------------------------------------------------------------------

bool Table::unlinkEntry(size_t entrynum)
{
#ifdef FrSINGLE_THREADED
   (void)entrynum ;
#else
   // the following lock serializes to allow only one
   //   recycle() or hopscotch() on the chain at a time;
   //   concurrent add() and remove() are OK
   size_t bucketnum = bucketContaining(entrynum) ;
   if (NULLPOS == bucketnum)
      return true ;
   ScopedChainLock lock(this,bucketnum) ;
   if (lock.busy())
      {
      return false ; // need to retry
      }
   announceBucketNumber(bucketnum) ;
   Link *prevptr = chainHeadPtr(bucketnum) ;
   Link offset = cs::load(*prevptr) ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      Link *nextptr = chainNextPtr(pos) ;
      if (pos == entrynum)
	 {
	 // we found the entry, now try to unlink it
	 Link nxt = cs::load(*nextptr) ;
	 if (unlikely(!cs::compareAndSwap(prevptr,offset,nxt)))
	    {
	    // uh oh, someone else messed with the chain!
	    // restart from the beginning of the chain
	    INCR_COUNT(CAS_coll) ;
	    prevptr = chainHeadPtr(bucketnum) ;
	    offset = cs::load(*prevptr) ;
	    continue ;
	    }
	 // mark the entry as not belonging to any chain anymore
	 setChainOwner(pos,NULLPTR) ;
	 unannounceBucketNumber() ;
	 return true ;
	 }
      else
	 {
	 prevptr = nextptr ;
	 }
      offset = cs::load(*nextptr) ;
      }
   unannounceBucketNumber() ;
#endif /* FrSINGLE_THREADED */
   return true ;
}

//----------------------------------------------------------------------------

size_t Table::recycleDeletedEntry(size_t bucketnum, KeyT new_key)
{
#ifdef FrSINGLE_THREADED
   // when single-threaded, we chop out deletions
   //   immediately, so there is nothing to reclaim
   (void)bucketnum ;
   (void)new_key ;
#else
   if (superseded())
      return NULLPOS ;
   debug_msg("recycledDeletedEntry (thr %ld)\n",my_job_id) ;
   // figure out the range of hash buckets to process
   size_t endpos = bucketnum + searchrange ;
   if (endpos > m_size)
      endpos = m_size ;
   bucketnum = bucketnum >= (size_t)searchrange ? bucketnum - searchrange : 0 ;
   size_t claimed = NULLPOS ;
   for (size_t i = bucketnum ; i < endpos ; ++i)
      {
      if (deletedEntry(i))
	 {
	 // try to grab the entry
	 if (updateKey(i,Entry::DELETED(),Entry::RECLAIMING()))
	    {
	    claimed = i ;
	    break ;
	    }
	 }
      }
   if (claimed == NULLPOS)
      return NULLPOS ;	// unable to find an entry to recycle
   INCR_COUNT(reclaim) ;
   // we successfully grabbed the entry, so
   //   now we need to chop it out of the
   //   chain currently containing it
   size_t claimed_bucketnum = bucketContaining(claimed) ;
   size_t loops = 0 ;
   while (!unlinkEntry(claimed))
      {
      thread_backoff(loops) ;
      }
   if (!superseded())
      {
      // ensure that any concurrent accesses complete
      //   before we actually recycle the entry
      awaitIdle(claimed_bucketnum,claimed_bucketnum+1) ;
      if (!superseded())
	 {
	 setKey(claimed,new_key) ;
	 return claimed ;
	 }
      }
#endif /* FrSINGLE_THREADED */
   return NULLPOS ;
}

//----------------------------------------------------------------------------

bool Table::reclaimChain(size_t bucketnum, size_t &min_reclaimed, size_t &max_reclaimed)
{
#ifdef FrSINGLE_THREADED
   (void)bucketnum ;
   // remove() chops out the deleted entries, so there's never anything to reclaim
   return false ;
#else
   // the following lock serializes to allow only one
   //   recycle()/reclaim() or hopscotch() on the chain at a time;
   //   concurrent add() and remove() are OK
   ScopedChainLock lock(this,bucketnum) ;
   if (lock.busy())
      {
      // someone else is currently reclaiming the chain,
      //   moving elements to make room, or copying the
      //   bucket to a new hash table; so we can just
      //   claim success ourselves
      return true ;
      }
   Link *prevptr = chainHeadPtr(bucketnum) ;
   Link offset = cs::load(*prevptr) ;
   bool reclaimed = false ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      Link *nextptr = chainNextPtr(pos) ;
      // while the additional check of the key just below is not strictly necessary, it
      //   permits us to avoid the expensive CAS except when an entry is actually deleted
      KeyT key = getKey(pos) ;
      Link nxt = cs::load(*nextptr) ;
      if (key == Entry::DELETED() && updateKey(pos,Entry::DELETED(),Entry::RECLAIMING()))
	 {
	 // we grabbed a deleted entry; now try to unlink it
	 if (unlikely(!cs::compareAndSwap(prevptr,offset,nxt)))
	    {
	    // uh oh, someone else messed with the chain!
	    setKey(pos,Entry::DELETED()) ;
	    // restart from the beginning of the chain
	    INCR_COUNT(CAS_coll) ;
	    prevptr = chainHeadPtr(bucketnum) ;
	    offset = cs::load(*prevptr) ;
	    continue ;
	    }
	 // mark the entry as not belonging to any chain anymore
	 setChainOwner(pos,NULLPTR) ;
	 reclaimed = true ;
	 if (pos < min_reclaimed)
	    min_reclaimed = pos ;
	 if (pos > max_reclaimed)
	    max_reclaimed = pos ;
	 }
      else
	 {
	 prevptr = nextptr ;
	 }
      offset = cs::load(*nextptr) ;
      }
   return reclaimed;
#endif /* FrSINGLE_THREADED */
}

//----------------------------------------------------------------------------

bool Table::assistResize()
{
   // someone else has already claimed the right to
   //   resize, so wait until the resizing starts and
   //   then help them out
   INCR_COUNT(resize_assist) ;
   m_resizestarted.wait() ;
   resizeCopySegments() ;
   // there's no need to synchronize with the
   //   completion of the resizing, since we'll
   //   transparently forward from the older version
   //   as required
   return true ;
}

//----------------------------------------------------------------------------

bool Table::resize(size_t newsize, bool enlarge_only = false)
{
   if (superseded())
      return false ;
   size_t currsize = m_currsize.load() ;
   newsize = normalizeSize(newsize) ;
   if (resizeThreshold(newsize) < currsize)
      newsize = normalizeSize(sizeForCapacity(currsize+1)) ;
   if (newsize == m_size || (enlarge_only && newsize < 1.1*m_size))
      {
      debug_msg("resize canceled (thr %ld)\n",my_job_id) ;
      return false ;
      }
   if (unlikely(m_resizelock.exchange(true)))
      {
      return assistResize() ;
      }
   // OK, we've won the right to run the resizing
   INCR_COUNT(resize) ;
   if (enlarge_only && currsize <= m_resizethresh)
      {
      warn_msg("contention-resize to %ld (currently %ld/%ld - %4.1f%%), thr %ld\n",
	       newsize,currsize,m_size,100.0*currsize/m_size,my_job_id) ;
      }
   else
      {
      debug_msg("resize to %ld from %ld/%ld (%4.1f%%) (thr %ld)\n",
		newsize,currsize,m_size,100.0*currsize/m_size,my_job_id) ;
      }
   Table *newtable = m_container->allocTable() ;
   newtable->init(newsize,m_container->m_maxfill) ;
   newtable->onRemove(m_container->onRemoveFunc()) ;
   cs::store(newtable->m_container,m_container) ;
   bool success = true ;
   if (expected(newtable->good()))
      {
      // link in the new table; this also makes
      //   operations on the current table start to
      //   forward to the new one
      m_next_table.store(newtable) ;
      m_resizestarted.set() ;
      // grab as many segments as we can and copy them
      resizeCopySegments() ;
      // wait for any other threads that have grabbed
      //   segments to complete them
      m_resizepending.wait() ;
      // if necessary, do a cleanup pass to copy any
      //   buckets which had to be skipped due to
      //   concurrent reclaim/hopscotch/remove() calls
      size_t first_incomplete = m_first_incomplete.load() ;
      size_t last_incomplete = m_last_incomplete.load() ;
      size_t loops = FrSPIN_COUNT ;
      while (first_incomplete <= last_incomplete)
	 {
	 INCR_COUNT(resize_cleanup) ;
	 bool complete = true ;
	 size_t first = m_size ;
	 size_t last = 0 ;
	 for (size_t i = first_incomplete ; i < last_incomplete && i < m_size ; ++i)
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
      // make the new table the current one for the containing FrHashTable
      m_container->updateTable() ;
      debug_msg(" resize done (thr %ld)\n",my_job_id) ;
      }
   else
      {
      FrWarning("unable to resize hash table--will continue") ;
      // bump up the resize threshold so that we can
      //   (hopefully) store a few more before
      //   (hopefully successfully) retrying the resize
      m_resizethresh += (m_size - m_resizethresh)/2 ;
      m_container->releaseTable(newtable) ;
      success = false ;
      }
   return success ;
}

//----------------------------------------------------------------------------

bool Table::add(size_t hashval, KeyT key, ValT value = 0) _fnattr_hot
{
   INCR_COUNT(insert) ;
   size_t bucketnum = hashval % m_size ;
   while (true)
      {
      FORWARD(add(hashval,key,value),nexttab,insert_forwarded) ;
      // scan the chain of items for this hash position
      if_MULTITHREAD(size_t deleted = NULLPOS ;)
	 // tell others we're using the bucket
	 announceBucketNumber(bucketnum) ;
      Link offset = chainHead(bucketnum) ;
      Link firstoffset = offset ;
      while (NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT key_at_pos = getKey(pos) ;
	 if (isEqual(key,key_at_pos))
	    {
	    // found existing entry!
	    INCR_COUNT(insert_dup) ;
	    unannounceBucketNumber() ;
	    return true ;		// item already exists
	    }
	 if_MULTITHREAD(else if (deletedEntry(pos) && deleted == NULLPOS) { deleted = pos ; })
	    // advance to next item in chain
	    offset = chainNext(pos) ;
	 }
      unannounceBucketNumber() ;
      // when we get here, we know that the item is not yet in the
      //   hash table, so try to add it
#ifndef FrSINGLE_THREADED
      // first, did we encounter a deleted entry? (can only happen if multithreaded)
      // If so, try to reclaim it
      if (deleted != NULLPOS)
	 {
	 if (unlikely(!updateKey(deleted,Entry::DELETED(),Entry::RECLAIMING())))
	    {
	    INCR_COUNT(CAS_coll) ;
	    continue ;		// someone else reclaimed the entry, so restart
	    }
	 // we managed to grab the entry for reclamation
	 m_currsize++ ;
	 // fill in the value, then the key
	 setValue(deleted,value) ;
	 cs::storeBarrier();
	 setKey(deleted,key) ;
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

size_t Table::addCount(size_t hashval, KeyT key, ValT incr) _fnattr_hot
{
   INCR_COUNT(insert) ;
   size_t bucketnum = hashval % m_size ;
   while (true)
      {
      FORWARD(addCount(hashval,key,incr),nexttab,insert_forwarded) ;
      // scan the chain of items for this hash position
      if_MULTITHREAD(size_t deleted = NULLPOS ;)
	 // tell others we're using the bucket
	 announceBucketNumber(bucketnum) ;
      Link offset = chainHead(bucketnum) ;
      Link firstoffset = offset ;
      while (NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT key_at_pos = getKey(pos) ;
	 if (isEqual(key,key_at_pos))
	    {
	    // found existing entry!  Verify that we
	    //   haven't been superseded during the
	    //   search
	    if (superseded())
	       {
	       unannounceBucketNumber() ;
	       return addCount(hashval,key,incr) ;
	       }
	    ValT newcount = m_entries[pos].atomicIncrCount(incr) ;
	    INCR_COUNT(insert_dup) ;
	    return newcount ;
	    }
	 // advance to next item in chain
	 offset = chainNext(pos) ;
	 }
      unannounceBucketNumber() ;
      // when we get here, we know that the item is not yet in the
      //   hash table, so try to add it
#ifndef FrSINGLE_THREADED
      // first, did we encounter a deleted entry? (can only happen if multithreaded)
      // If so, try to reclaim it
      if (deleted != NULLPOS)
	 {
	 if (unlikely(!updateKey(deleted,Entry::DELETED(),Entry::RECLAIMING())))
	    {
	    INCR_COUNT(CAS_coll) ;
	    continue ;		// someone else reclaimed the entry, so restart
	    }
	 // we managed to grab the entry for reclamation
	 m_currsize++ ;
	 // fill in the value, then the key
	 setValue(deleted,incr) ;
	 cs::storeBarrier();
	 setKey(deleted,key) ;
	 INCR_COUNT(insert_attempt) ;
	 // Verify that we haven't been superseded while we
	 //   were working
	 FORWARD(add(hashval,key,incr),nexttab2,insert_forwarded) ;
	 break ;
	 }
#endif /* !FrSINGLE_THREADED */
      // otherwise, try to insert a new key/value entry
      if (insertKey(bucketnum,firstoffset,key,incr))
	 break ;
      }
   return incr ;
}

//----------------------------------------------------------------------------

bool Table::contains(size_t hashval, KeyT key) const _fnattr_hot
{
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_COPIED(contains(hashval,key),contains_forwarded) ;
   INCR_COUNT(contains) ;
   // tell others we're using the bucket
   announceBucketNumber(bucketnum) ;
   // scan the chain of items for this hash position
   Link offset = chainHead(bucketnum) ;
   bool success = false ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT key_at_pos = getKey(pos) ;
      if (isEqual(key,key_at_pos))
	 {
	 INCR_COUNT(contains_found) ;
	 success = true ;
	 break ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   unannounceBucketNumber() ;
   return success ;
}

//----------------------------------------------------------------------------

ValT Table::lookup(size_t hashval, KeyT key) const _fnattr_hot
{
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_COPIED(lookup(hashval,key),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   // tell others we're using the bucket
   announceBucketNumber(bucketnum) ;
   // scan the chain of items for this hash position
   Link offset = chainHead(bucketnum) ;
   ValT value = 0 ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (isEqual(key,hkey))
	 {
	 value = getValue(pos) ;
	 // double-check that a parallel remove() hasn't
	 //   deleted the entry while we were fetching the
	 //   value
	 if (getKey(pos) != hkey)
	    value = 0 ;
	 INCR_COUNT(lookup_found) ;
	 break ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   unannounceBucketNumber() ;
   return value ;
}

//----------------------------------------------------------------------------

bool Table::lookup(size_t hashval, KeyT key, ValT *value) const _fnattr_hot
{
   if (!value)
      return false ;
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_COPIED(lookup(hashval,key,value),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   (*value) = 0 ;
   // tell others we're using the bucket
   announceBucketNumber(bucketnum) ;
   // scan the chain of items for this hash position
   Link offset = chainHead(bucketnum) ;
   bool found = false ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (isEqual(key,hkey))
	 {
	 (*value) = getValue(pos) ;
	 // double-check that a parallel remove() hasn't deleted the
	 //   hash entry while we were working with it
	 found = (getKey(pos) == hkey) ;
	 INCR_COUNT(lookup_found) ;
	 break ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   unannounceBucketNumber() ;
   return found ;
}

//----------------------------------------------------------------------------

// NOTE: this lookup() is not entirely thread-safe if clear==true
bool Table::lookup(size_t hashval, KeyT key, ValT *value, bool clear_entry) _fnattr_hot
{
   if (!value)
      return false ;
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_COPIED(lookup(hashval,key,value),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   (*value) = 0 ;
   // tell others we're using the bucket
   announceBucketNumber(bucketnum) ;
   // scan the chain of items for this hash position
   Link offset = chainHead(bucketnum) ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (isEqual(key,hkey))
	 {
	 if (clear_entry)
	    {
	    (*value) = m_entries[pos].swapValue(0) ;
	    }
	 else
	    {
	    (*value) = getValue(pos) ;
	    }
	 // double-check that a parallel remove() hasn't deleted the
	 //   hash entry while we were working with it
	 bool found = (getKey(pos) == hkey) ;
	 unannounceBucketNumber() ;
	 INCR_COUNT(lookup_found) ;
	 return found ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   unannounceBucketNumber() ;
   return false ;	// not found
}

//----------------------------------------------------------------------------

// note: lookupValuePtr is not safe in the presence of parallel
//   add() and remove() calls!  Use global synchronization if
//   you will be using both this function and add()/remove()
//   concurrently on the same hash table.
ValT *Table::lookupValuePtr(size_t hashval, KeyT key) const
{
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_COPIED(lookupValuePtr(hashval,key),lookup_forwarded) ;
   INCR_COUNT(lookup) ;
   // tell others we're using the bucket
   announceBucketNumber(bucketnum) ;
   // scan the chain of items for this hash position
   Link offset = chainHead(bucketnum) ;
   ValT *val = nullptr ; // assume "not found"
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT key_at_pos = getKey(pos) ;
      if (isEqual(key,key_at_pos))
	 {
	 val = getValuePtr(pos) ;
	 INCR_COUNT(lookup_found) ;
	 break ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   unannounceBucketNumber() ;
   return val ;
}

//----------------------------------------------------------------------------

bool Table::remove(size_t hashval, KeyT key)
{
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_STALE(remove(hashval,key),remove_forwarded) ;
   INCR_COUNT(remove) ;
#ifdef FrSINGLE_THREADED
   // since nobody else can mess with the bucket chain while we're traversing it, we can
   //   just chop out the desired item and don't need to bother with marking entries as
   //   deleted and reclaiming them later
   Link *prevptr = chainHeadPtr(bucketnum) ;
   Link offset = *prevptr ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT key_at_pos = getKey(pos) ;
      if (isEqual(key,key_at_pos))
	 {
	 // we found it!
	 // chop entry out of hash chain
	 *prevptr = chainNext(pos) ;
	 // delete the item proper
	 m_entries[pos].markUnused() ;
	 replaceValue(pos,0) ;
	 m_currsize-- ; // update count of active items
	 INCR_COUNT(remove_found) ;
	 return true ;
	 }
      // advance to next item in chain
      prevptr = chainNextPtr(pos) ;
      offset = *prevptr ;
      }
   // item not found
   return false ;
#else
   // deletions must be marked rather than removed since we
   //   can't CAS both the key field and the chain link at
   //   once
   // because add() could generate duplicates if there are
   //   concurrent insertions of the same item with multiple
   //   deleted records in the bucket's chain, we need to
   //   remove all occurrences of the key so that the delete
   //   doesn't make a replaced entry reappear
   announceBucketNumber(bucketnum) ;
   Link offset = chainHead(bucketnum) ;
   bool success = false ;
   // scan the chain of items for this hash position
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT keyval = getKey(pos) ;
      if (isEqual(key,keyval))
	 {
	 ValT value = getValue(pos) ;
	 if (expected(updateKey(pos,keyval,Entry::DELETED())))
	    {
	    // update count of items in table
	    m_currsize-- ;
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
   unannounceBucketNumber() ;
   return success ;
#endif /* FrSINGLE_THREADED */
}

//----------------------------------------------------------------------------

bool Table::reclaimDeletions()
{
   if (superseded())
      return true ;
#ifdef FrSINGLE_THREADED
   // when single-threaded, we chop out deletions
   //   immediately, so there is nothing to reclaim
   return false ;
#else
   INCR_COUNT(reclaim) ;
   debug_msg("reclaimDeletions (thr %ld)\n",my_job_id) ;
   bool have_reclaimed = false ;
   size_t min_reclaimed = ~0UL ;
   size_t max_reclaimed = 0 ;
   size_t min_bucket = ~0UL ;
   size_t max_bucket = 0 ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (deletedEntry(i))
	 {
	 // follow the chain containing the deleted item,
	 //   chopping out any marked as deleted and
	 //   resetting their status to Reclaimed.
	 Link offset = chainOwner(i) ;
	 if (NULLPTR != offset)
	    {
	    size_t bucket = i - offset ;
	    bool reclaimed = reclaimChain(bucket,min_reclaimed,max_reclaimed) ;
	    have_reclaimed |= reclaimed ;
	    if (reclaimed && bucket < min_bucket)
	       min_bucket = bucket ;
	    if (reclaimed && bucket > max_bucket)
	       max_bucket = bucket ;
	    }
	 // abandon reclaim if a resize has run since we started
	 if (superseded())
	    break ;
	 }
      }
   if (have_reclaimed && !superseded()) // (if resized, the reclamation was moot)
      {
      // ensure that any existing concurrent reads complete
      //   before we allow writes to the reclaimed entries
      awaitIdle(min_bucket,max_bucket+1) ;
      // at this point, nobody is traversing nodes we've chopped
      //   out of hash chains, so we can go ahead and actually
      //   reclaim
      if (!superseded())
	 {
	 // reclaim entries marked as Reclaimed by switching their
	 //   keys from Reclaimed to Unused
	 // assumes no active writers and pending writers will be
	 //   blocked
	 for (size_t i = min_reclaimed ; i <= max_reclaimed ; ++i)
	    {
	    // any entries which no longer belong to a chain get marked as free
	    if (NULLPTR == chainOwner(i))
	       {
	       setKey(i,Entry::UNUSED()) ;
	       }
	    }
	 }
      }
   else if (have_reclaimed)
      {
      debug_msg("reclaimDeletions mooted (thr %ld)\n",my_job_id) ;
      }
   debug_msg("reclaimDeletions done (thr %ld)\n",my_job_id) ;
   return have_reclaimed ;
#endif /* FrSINGLE_THREADED */
}

//----------------------------------------------------------------------------

// special support for FrSymbolTableX
KeyT Table::addKey(size_t hashval, const char *name, size_t namelen,
		   bool *already_existed = 0) _fnattr_hot
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
   INCR_COUNT(insert) ;
   size_t bucketnum = hashval % m_size ;
   KeyT key ;
   while (true)
      {
      FORWARD(addKey(hashval,name,namelen,already_existed),nexttab,insert_forwarded) ;
      // scan the chain of items for this hash position
      announceBucketNumber(bucketnum) ;
      Link offset = chainHead(bucketnum) ;
      Link firstoffset = offset ;
      while (NULLPTR != offset)
	 {
	 size_t pos = bucketnum + offset ;
	 KeyT existing = getKey(pos) ;
	 if (isEqual(name,namelen,existing))
	    {
	    // found existing entry!
	    INCR_COUNT(insert_dup) ;
	    if (already_existed)
	       *already_existed = true ;
	    unannounceBucketNumber() ;
	    return existing ;
	    }
	 // advance to next item in chain
	 offset = chainNext(pos) ;
	 }
      // when we get here, we know that the item is not yet in the
      //   hash table, so try to add it
      key = Fr_allocate_symbol(static_cast<FrSymbolTable*>(m_container),name,namelen) ;
      // if the insertKey fails, someone else beat us to
      //    creating the symbol, so abandon this copy
      //    (temporarily leaks at bit of memory until the
      //    hash table is destroyed) and return the other one
      //    when we loop back to the top
      if (insertKey(bucketnum,firstoffset,key,(ValT)0))
	 break ;
      unannounceBucketNumber() ;
      }
   return key ;
}

//----------------------------------------------------------------------------

// special support for FrSymbolTable
bool Table::contains(size_t hashval, const char *name, size_t namelen) const _fnattr_hot
{
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_COPIED(contains(hashval,name,namelen),contains_forwarded)
      INCR_COUNT(contains) ;
   // scan the chain of items for this hash position
   announceBucketNumber(bucketnum) ;
   Link offset = chainHead(bucketnum) ;
   bool success = false ;
   while (NULLPTR != offset)
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
   unannounceBucketNumber() ;
   return success ;
}

//----------------------------------------------------------------------------

// special support for FrSymbolTableX
KeyT Table::lookupKey(size_t hashval, const char *name, size_t namelen) const _fnattr_hot
{
   size_t bucketnum = hashval % m_size ;
   FORWARD_IF_COPIED(lookupKey(hashval,name,namelen),contains_forwarded)
      INCR_COUNT(lookup) ;
   // scan the chain of items for this hash position
   announceBucketNumber(bucketnum) ;
   Link offset = chainHead(bucketnum) ;
   while (NULLPTR != offset)
      {
      size_t pos = bucketnum + offset ;
      KeyT hkey = getKey(pos) ;
      if (isEqual(name,namelen,hkey))
	 {
	 INCR_COUNT(lookup_found) ;
	 unannounceBucketNumber() ;
	 return hkey ;
	 }
      // advance to next item in chain
      offset = chainNext(pos) ;
      }
   unannounceBucketNumber() ;
   return 0 ;	// not found
}

//----------------------------------------------------------------------------

void Table::replaceValue(size_t pos, ValT new_value)
{
   ValT value = m_entries[pos].swapValue(new_value) ;
   if (remove_fn && value)
      {
      remove_fn(getKey(pos),value) ;
      }
   return ;
}

//----------------------------------------------------------------------------

size_t Table::countItems() const _fnattr_cold
{
   if (next())
      return next()->countItems() ;
   size_t count = 0 ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (activeEntry(i))
	 count++ ;
      }
   return count ;
}

//----------------------------------------------------------------------------

size_t Table::countItems(bool remove_dups) _fnattr_cold
{
   if (next())
      return next()->countItems(remove_dups) ;
   if (remove_dups)
      {
      for (size_t i = 0 ; i < m_size ; ++i)
	 {
	 clearDuplicates(i) ;
	 }
      }
   size_t count = 0 ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (activeEntry(i))
	 count++ ;
      }
   return count ;
}

//----------------------------------------------------------------------------

size_t Table::countDeletedItems() const _fnattr_cold
{
   if (next())
      return next()->countDeletedItems() ;
   size_t count = 0 ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (deletedEntry(i))
	 count++ ;
      }
   return count ;
}

//----------------------------------------------------------------------------

size_t Table::chainLength(size_t bucketnum) const _fnattr_cold
{
   FORWARD_IF_COPIED(chainLength(bucketnum),none) ;
   size_t len = 0 ;
   announceBucketNumber(bucketnum) ;
   Link offset = chainHead(bucketnum) ;
   while (NULLPTR != offset)
      {
      len++ ;
      size_t pos = bucketnum + offset ;
      offset = chainNext(pos) ;
      }
   unannounceBucketNumber() ;
   return len ;
}

//----------------------------------------------------------------------------

size_t *Table::chainLengths(size_t &max_length) const _fnattr_cold
{
   if (next())
      return next()->chainLengths(max_length) ;
   size_t *lengths = new size_t[2*searchrange+2] ;
   for (size_t i = 0 ; i <= 2*searchrange+1 ; i++)
      {
      lengths[i] = 0 ;
      }
   max_length = 0 ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      size_t len = chainLength(i) ;
      if (len > max_length)
	 max_length = len ;
      lengths[len]++ ;
      }
   return lengths ;
}

//----------------------------------------------------------------------------

size_t *Table::neighborhoodDensities(size_t &num_densities) const _fnattr_cold
{
   if (next())
      return next()->neighborhoodDensities(num_densities) ;
   size_t *densities = new size_t[2*searchrange+2] ;
   num_densities = 2*searchrange+1 ;
   for (size_t i = 0 ; i <= 2*searchrange+1 ; i++)
      {
      densities[i] = 0 ;
      }
   // count up the active neighbors of the left-most entry in the hash array
   size_t density = 0 ;
   for (size_t i = 0 ; i <= (size_t)searchrange && i < m_size ; ++i)
      {
      if (activeEntry(i))
	 ++density ;
      }
   ++densities[density] ;
   for (size_t i = 1 ; i < m_size ; ++i)
      {
      // keep a rolling count of the active neighbors around the current point
      // did an active entry come into range on the right?
      if (i + searchrange < m_size && activeEntry(i+searchrange))
	 ++density ;
      // did an active entry go out of range on the left?
      if (i > (size_t)searchrange && activeEntry(i-searchrange-1))
	 --density ;
      ++densities[density] ;
      }
   return densities ;
}

//----------------------------------------------------------------------------

bool Table::iterateVA(HashKeyValueFunc *func, va_list args) const
{
   bool success = true ;
   for (size_t i = 0 ; i < m_size && success ; ++i)
      {
      if (!activeEntry(i))
	 continue ;
      KeyT key = getKey(i) ;
      ValT value = getValue(i) ;
      // if the item hasn't been removed while we were getting it,
      //   call the processing function
      if (!activeEntry(i))
	 continue ;
      FrSafeVAList(args) ;
      success = func(key,value,FrSafeVarArgs(args)) ;
      FrSafeVAListEnd(args) ;
      }
   return success ;
}

//----------------------------------------------------------------------------

bool iterateAndClearVA(HashKeyValueFunc *func, va_list args) const
{
   bool success = true ;
   for (size_t i = 0 ; i < m_size && success ; ++i)
      {
      if (!activeEntry(i))
	 continue ;
      KeyT key = getKey(i) ;
      ValT value = getValue(i) ;
      // if the item hasn't been removed while we were getting it,
      //   call the processing function
      if (!activeEntry(i))
	 continue ;
      FrSafeVAList(args) ;
      success = func(key,value,FrSafeVarArgs(args)) ;
      if (success)
	 m_entries[i].setValue(0) ;
      FrSafeVAListEnd(args) ;
      }
   return success ;
}

//----------------------------------------------------------------------------

bool Table::iterateAndModifyVA(HashKeyPtrFunc *func, va_list args) const
{
   bool success = true ;
   for (size_t i = 0 ; i < m_size && success ; ++i)
      {
      if (!activeEntry(i))
	 continue ;
      KeyT key = getKey(i) ;
      ValT* valptr = getValuePtr(i) ;
      // if the item hasn't been removed while we were getting it,
      //   call the processing function
      if (!activeEntry(i))
	 continue ;
      FrSafeVAList(args) ;
      success = func(key,valptr,FrSafeVarArgs(args)) ;
      FrSafeVAListEnd(args) ;
      }
   return success ;
}

//----------------------------------------------------------------------------

FrList *Table::allKeys() const
{
   FrList *keys = 0 ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (activeEntry(i))
	 {
	 KeyT name = m_entries[i].copyName() ;
	 pushlist(name,keys) ;
	 }
      }
   return keys ;
}

//----------------------------------------------------------------------------

bool Table::verify() const _fnattr_cold
{
   bool success = true ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (activeEntry(i) && !contains(getKey(i)))
	 {
	 debug_msg("verify: missing @ %ld\n",i) ;
	 success = false ;
	 break ;
	 }
      }
   return success ;
}

//----------------------------------------------------------------------------

ostream &Table::printKeyValue(ostream &output, KeyT key) const
{
   return output << key ;
}

//----------------------------------------------------------------------------

size_t Table::keyDisplayLength(KeyT key) const
{
   return key ? key->displayLength() + 1 : 3 ;
}

//----------------------------------------------------------------------------

char *Table::displayKeyValue(char *buffer, KeyT key) const
{
   if (key)
      return key->displayValue(buffer) ;
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

ostream &Table::printValue(ostream &output) const
{
   output << "#H(" ;
   size_t orig_indent = FramepaC_initial_indent ;
   FramepaC_initial_indent += 3 ; //strlen("#H(")
   size_t loc = FramepaC_initial_indent ;
   bool first = true ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (!activeEntry(i))
	 continue ;
      KeyT key = getKey(i) ;
      size_t len = keyDisplayLength(key) ;
      loc += len ;
      if (loc > FramepaC_display_width && !first)
	 {
	 output << '\n' << setw(FramepaC_initial_indent) << " " ;
	 loc = FramepaC_initial_indent + len ;
	 }
      output << key << ' ' ;
      first = false ;
      }
   output << ")" ;
   FramepaC_initial_indent = orig_indent ;
   return output ;
}

//----------------------------------------------------------------------------

char *Table::displayValue(char *buffer) const
{
   strcpy(buffer,"#H(") ;
   buffer += 3 ;
   for (size_t i = 0 ; i < m_size ; ++i)
      {
      if (!activeEntry(i))
	 continue ;
      KeyT key = getKey(i) ;
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
size_t Table::displayLength() const
{
   size_t dlength = 4 ; // "#H(" prefix plus ")" trailer
   for (size_t i = 0 ; i < m_size ; i++)
      {
      if (!activeEntry(i))
	 continue ;
      KeyT key = getKey(i) ;
      dlength += keyDisplayLength(key) + 1 ;
      }
   return dlength ;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

/************************************************************************/
/*	Declarations for template class FrHashTable			*/
/************************************************************************/

//----------------------------------------------------------------------
// static members

#ifndef FrSINGLE_THREADED
template <typename KeyT, typename ValT>
thread_local typename FrHashTable<KeyT,ValT>::Table *FrHashTable<KeyT,ValT>::s_table = nullptr ;
template <typename KeyT, typename ValT>
thread_local size_t FrHashTable<KeyT,ValT>::s_bucket = ~0UL ;
template <typename KeyT, typename ValT>
FrMutex FrHashTable<KeyT,ValT>::s_mutex ;
template <typename KeyT, typename ValT>
FrThreadOnce FrHashTable<KeyT,ValT>::s_once ;
template <typename KeyT, typename ValT>
FrThreadKey FrHashTable<KeyT,ValT>::s_threadkey ;
template <typename KeyT, typename ValT>
typename FrHashTable<KeyT,ValT>::TablePtr *FrHashTable<KeyT,ValT>::s_thread_entries = nullptr ;
template <typename KeyT, typename ValT>
size_t FrHashTable<KeyT,ValT>::s_registered_threads = 0 ;
template <typename KeyT, typename ValT>
thread_local typename FrHashTable<KeyT,ValT>::TablePtr FrHashTable<KeyT,ValT>::s_thread_record ;
#endif /* !FrSINGLE_THREADED */

#if defined(FrHASHTABLE_STATS)
template <typename KeyT, typename ValT>
thread_local HashTable_Stats HashTable<KeyT,ValT>::s_stats ;
#endif /* FrHASHTABLE_STATS */

template <> FrReader *FrHashTable<FrObject,FrObject*>::s_reader ;
template <typename KeyT, typename ValT>
FrReader *FrHashTable<KeyT,ValT>::s_reader = 0 ;

//----------------------------------------------------------------------
// specializations: integer keys

#define FrMAKE_INTEGER_HASHTABLE_CLASS(NAME,K,V) \
\
template <> \
inline size_t HashTable<K,V>::hashVal(const K key) { return (size_t)key ; } \
\
template <> \
inline bool HashTable<K,V>::isEqual(const K key1, const K key2) \
{ return key1 == key2 ; } \
\
template <> \
inline K HashTable<K,V>::Entry::copy(const K obj) { return obj ; } \
\
template <> \
inline size_t HashTable<K,V>::Table::keyDisplayLength(const K key) const	\
{ \
   char buffer[200] ; \
   ultoa((size_t)key,buffer,10) ; \
   return strlen(buffer) ; \
} \
\
template <> \
inline char *HashTable<K,V>::Table::displayKeyValue(char *buffer,const K key) const \
{ ultoa((size_t)key,buffer,10) ; return strchr(buffer,'\0') ; }	\
\
typedef HashTable<K,V> NAME ;

//----------------------------------------------------------------------
// specializations: FrSymbol* keys

#define FrMAKE_SYMBOL_HASHTABLE_CLASS(NAME,V) \
\
template <> \
inline size_t HashTable<const Symbol *,V>::hashVal(const Symbol *key) { return (size_t)key ; } \
\
template <> \
inline bool HashTable<const Symbol *,V>::isEqual(const Symbol *key1, const FrSymbol *key2) \
{ return (size_t)key1 == (size_t)key2 ; }			  \
\
template <> \
inline const Symbol *FrHashTable<const Symbol *,V>::Entry::copy(const Symbol *obj) { return obj ; } \
\
typedef HashTable<const Symbol *,V> NAME ;

//----------------------------------------------------------------------
// specializations for FrSymbolTableX not included in above macro

size_t Fr_symboltable_hashvalue(const char *symname) ;
template <>
inline size_t HashTable<const Symbol *,NullObject>::hashValFull(const Symbol *key)
{ 
   return key ? Fr_symboltable_hashvalue(key->symbolName()) : 0 ;
}

template <>
inline bool HashTable<const Symbol *,NullObject>::isEqualFull(const Symbol *key1, const Symbol *key2)
{ 
   if (!HashTable::isActive(key2))
      return false ;
   if (key1 == key2)
      return true ;
   return (key1 && key2 && strcmp(key1->symbolName(),key2->symbolName()) == 0) ;
}

/************************************************************************/

typedef HashTable<Object*,Object*> ObjHashTable ;
typedef HashTable<Object*,size_t> ObjCountHashTable ;

FrMAKE_SYMBOL_HASHTABLE_CLASS(SymHashTable,Object *) ;
FrMAKE_SYMBOL_HASHTABLE_CLASS(SymCountHashTable,size_t) ;
FrMAKE_SYMBOL_HASHTABLE_CLASS(SymbolTableX,NullObject) ;

#undef NUM_TABLES
#undef INCR_COUNT
#undef FORWARD
#undef FORWARD_IF_COPIED
#undef FORWARD_IF_STALE
#undef DELEGATE
#undef DELEGATE_HASH
#undef DELEGATE_HASH_RECLAIM

#endif /* !__FRHASH_H_INCLUDED */

// end of file frhash.h //
