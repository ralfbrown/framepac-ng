/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-18					*/
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

#include <cstring>
#include <vector>
#include "framepac/bitvector.h"
#include "framepac/sufarray.h"
#include "framepac/threadpool.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

// Suffix array construction code adapted/optimized from
// Ge Nong, Sen Zhang, and Wai Hong Chan.  Two Efficient Algorithms for Linear Time Suffix Array Construction

// see also
//   Anish Man Singh Shrestha, Martin C. Frith, and Paul Horton.  A
//      bioinformatician's guide to the forefront of suffix array
//      construction algorithms.  Briefings in Bioinformatics (2014) 15(2):138-154.
// ( http://bib.oxfordjournals.org/content/15/2/138.full )

//----------------------------------------------------------------------------

// special case for EOL markers, since they can encode line numbers
template <typename IdT, typename IdxT>
template <typename I>
IdT SuffixArray<IdT,IdxT>::convertEOL(I id, IdT num_types) const
{
   return (id >= num_types) ? I(m_newline) : id ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
template <typename I>
IdxT* SuffixArray<IdT,IdxT>::bucketBoundaries(const I* ids, IdxT num_ids, IdT num_types, const IdxT* freqs)
{
   // allocate and clear buckets
   IdxT* buckets = new IdxT[num_types+2] ;
   if (buckets)
      {
      std::fill(buckets,buckets+num_types+2,0) ;
      if (!freqs)
	 {
	 // accumulate bucket sizes
	 for (IdxT i = 0 ; i < num_ids ; ++i)
	    {
	    IdT id = convertEOL(ids[i],num_types) ;
	    ++buckets[id] ;
	    }
	 freqs = buckets ;
	 }
      // convert per-bucket counts into running totals such that
      //   buckets[i] is the start of the ith bucket, and buckets[i+1]
      //   points just past the bucket's end
      IdxT total = 0 ;
      for (IdxT i = 0 ; i <= num_types ; ++i)
	 {
	 IdxT bcount = freqs[i] ;
	 buckets[i] = total ;
	 total += bcount ;
	 }
      buckets[num_types+1] = total ;
      }
   return buckets ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT* SuffixArray<IdT,IdxT>::copyBucketBoundaries(const IdxT* bounds, IdT num_types)
{
   // make a copy of the bucket positions
   IdxT* copy = new IdxT[num_types+1] ;
   if (copy)
      {
      std::copy(bounds,bounds+num_types+1,copy) ;
      }
   return copy ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
template <typename I>
void SuffixArray<IdT,IdxT>::induce(const I* ids, IdxT* SA, IdxT num_ids, IdT num_types, IdxT* buckets,
	    const Fr::BitVector& ls_types)
{
   LocalAlloc<IdxT,1> bucket_ends { copyBucketBoundaries(buckets+1,num_types) } ;
   // induce on bucket starts (SAl in original paper)
   for (IdxT i = 0 ; i < num_ids ; ++i)
      {
      IdxT j = SA[i] ;
      if (j == (IdxT)-1 || j == (IdxT)0)
	 continue ;
      --j ;
      if (!ls_types.getBit(j))
	 {
	 IdxT bck = convertEOL(ids[j],num_types) ;
	 SA[buckets[bck]] = j ;
	 ++buckets[bck] ;
	 }
      }
   delete[] buckets ;
   // induce on bucket ends (SAs in original paper)
   for (IdxT i = num_ids ; i > 0 ; --i)
      {
      IdxT j = SA[i-1] ;
      if (j == (IdxT)-1 || j == (IdxT)0)
	 continue ;
      --j ;
      if (ls_types.getBit(j))
	 {
	 IdxT bck = convertEOL(ids[j],num_types) ;
	 SA[--bucket_ends[bck]] = j ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
template <typename I>
void SuffixArray<IdT,IdxT>::classifyLS(Fr::BitVector& ls_types, const I* ids, IdxT num_ids, IdT num_types)
{
   // classify elements of 'ids' array
   ls_types.setBit(num_ids+1,true) ;
   ls_types.setBit(num_ids,false) ;
   ls_types.setBit(num_ids-1,true) ;
   bool bit = true ;
   if (num_ids >= 2)
      {
      IdxT id2 = convertEOL(ids[num_ids-1],num_types) ;
      for (IdxT i = num_ids-1 ; i > 0 ; --i)
	 {
	 IdxT id1 = convertEOL(ids[i-1],num_types) ;
	 bit = (id1 < id2 || (id1 == id2 && bit)) ;
	 ls_types.setBit(i-1,bit) ;
	 id2 = id1 ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
template <typename I>
bool SuffixArray<IdT,IdxT>::Create(const I* ids, IdxT* index, IdxT num_ids, IdT num_types, const IdxT* freqs)
{
   if (num_ids == IdxT(0))
      return false ;
   ScopedObject<BitVector> ls_types(num_ids+2) ;
   if (!ls_types)
      return false ;
   classifyLS(*ls_types,ids,num_ids,num_types) ;
   for (IdxT i = 0U ; i < num_ids ; ++i)
      {
      index[i] = (IdxT)-1 ;
      }
   IdxT* buckets = bucketBoundaries(ids, num_ids, num_types, freqs) ;
   IdxT* bucket_ends = copyBucketBoundaries(buckets+1,num_types) ;
   bool prev_type = ls_types->getBit(0) ;
   for (IdxT i = 1 ; i < num_ids ; ++i)
      {
      // find left-most S positions and place them in the appropriate buckets
      bool curr_type = ls_types->getBit(i) ;
      if (!prev_type && curr_type)
	 {
	 IdT bck = convertEOL(ids[i],num_types) ;
	 index[--bucket_ends[bck]] = i ;
	 }
      prev_type = curr_type ;
      }
   delete[] bucket_ends ;
   induce(ids, index, num_ids, num_types, buckets, *ls_types) ;
   // compact all of the sorted substrings into the start of 'suffix_index'
   IdxT subsize { 0 } ;
   for (IdxT i = 0 ; i < num_ids ; ++i)
      {
      IdxT idx = index[i] ;
      if (idx > 0 && !ls_types->getBit(idx-1) && ls_types->getBit(idx))
	 {
	 index[subsize] = idx ;
	 ++subsize ;
	 }
      }
   //!assert(subsize > 0) ;
   for (IdxT i = subsize ; i < num_ids ; ++i)
      {
      // init as-yet-unused part of buffer
      index[i] = -1 ;
      }
   IdxT *s1 = index + subsize ;
   // find the lexicographic names of the substrings, storing them in
   //   the now-spare part of the suffix array
   IdxT name { 0 } ;
   IdxT prev { IdxT(-1) } ;
   for (IdxT i = 0 ; i < subsize ; ++i)
      {
      IdxT pos = index[i] ;
      bool diff = false ;
      if (prev == (IdxT)-1)
	 {
	 diff = true ;
	 }
      else
	 {
	 bool last_pos_bit = true ;
	 bool last_prev_bit = true ;
	 for (IdxT d = 0; d < num_ids ; ++d)
	    {
	    if (ids[pos+d] != ids[prev+d])
	       {
	       diff = true ;
	       break ;
	       }
	    bool pos_bit = ls_types->getBit(pos+d) ;
	    bool prev_bit = ls_types->getBit(prev+d) ;
	    if (pos_bit != prev_bit)
	       {
	       diff = true ;
	       break ;
	       }
	    else if ((pos_bit && !last_pos_bit) || (prev_bit && !last_prev_bit))
	       {
	       break ;
	       }
	    last_pos_bit = pos_bit ;
	    last_prev_bit = prev_bit ;
	    }
	 }
      if (diff)
	 {
	 ++name ;
	 prev = pos ;
	 }
      s1[pos/2]=name-1;
      }
   // compact used elements to the beginning of the spare part of the suffix array
   for (IdxT i = subsize, j = subsize ; i < num_ids ; ++i)
      {
      IdxT idx = index[i] ;
      if (idx != (IdxT)-1)
	 {
	 index[j] = idx ;
	 ++j ;
	 }
      }
   // stage 2: solve the reduced problem
   if (name < subsize)
      {
      // names are not yet unique, so recurse
      Create(s1, index, subsize, name) ;
      }
   else
      {
      // names are unique, so we can directly generate the reduced
      //   suffix array now
      for (IdxT i = 0 ; i < subsize ; ++i)
	 {
	 index[s1[i]] = i ;
	 }
      }
   // stage 3: induce the solution for the original problem from the
   //   results of the subproblem
   bool prev_bit = ls_types->getBit(0) ;
   for (IdxT i = 1, j = 0 ; i < num_ids ; ++i)
      {
      bool curr_bit = ls_types->getBit(i) ;
      if (!prev_bit && curr_bit)
	 {
	 s1[j] = i ;
	 ++j ;
	 }
      prev_bit = curr_bit ;
      }
   // retrieve original index in 'ids'
   for (IdxT i = 0 ; i < subsize ; ++i)
      {
      index[i] = s1[index[i]] ;
      }
   for (IdxT i = subsize ; i < num_ids ; ++i)
      {
      index[i] = -1 ;		// init remainder of suffix_index
      }
   buckets = bucketBoundaries(ids, num_ids, num_types, freqs) ;
   bucket_ends = copyBucketBoundaries(buckets+1, num_types) ;
   for (IdxT i = subsize ; i > 0 ; --i)
      {
      IdxT j = index[i-1] ;
      index[i-1] = -1 ;
      IdT bck = convertEOL(ids[j],num_types) ;
      index[--bucket_ends[bck]] = j ;
      }
   delete[] bucket_ends ;
   induce(ids, index, num_ids, num_types, buckets, *ls_types) ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::generate(const IdT *ids, IdxT num_ids, IdT num_types,
				     IdT mapped_newline, const IdxT *freqs)
{
   m_ids = const_cast<IdT*>(ids) ;
   m_numids = num_ids ;
   m_types = num_types ;
   m_newline = mapped_newline ;
   if (!ids || !num_ids)
      return false ;
   m_index = new IdxT[num_ids] ;
   if (!m_index)
      {
      m_numids = 0 ;
      return false ;
      }
   return Create(ids,m_index,num_ids,num_types,freqs) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
int SuffixArray<IdT,IdxT>::compare(IdT id1, IdT id2)
{
   return (id1 < id2) ? -1 : ((id1 > id2) ? +1 : 0) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
int SuffixArray<IdT,IdxT>::compare(const IdT* key1, const IdT* key2, unsigned keylen) const
{
   int comp = 0 ;
   unsigned pos = 0 ;
   while (pos < keylen && (comp = compare(key1[pos],key2[pos]) == 0))
      {
      if (key1[pos] >= m_last_linenum || key2[pos] >= m_last_linenum)
	 break ;
      ++pos ;
      }
   return comp ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
int SuffixArray<IdT,IdxT>::compareAt(IdxT idx1, IdxT idx2, unsigned keylen) const
{
   int comp = 0 ;
   unsigned pos = 0 ;
   while (pos < keylen && (comp = compare(m_ids[idx1],m_ids[idx2]) == 0))
      {
      if (m_ids[idx1] >= m_last_linenum || m_ids[idx2] >= m_last_linenum)
	 break ;
      idx1 = (idx1 + 1) % m_numids ;
      idx2 = (idx2 + 1) % m_numids ;
      ++pos ;
      }
   return comp ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
int SuffixArray<IdT,IdxT>::compareAt(IdxT idx1, const IdT* key2, unsigned keylen) const
{
   int comp = 0 ;
   unsigned pos = 0 ;
   while (pos < keylen && (comp = compare(m_ids[idx1],key2[pos])) == 0)
      {
      if (m_ids[idx1] >= m_last_linenum || key2[pos] >= m_last_linenum)
	 break ;
      idx1 = (idx1 + 1) % m_numids ;
      ++pos ;
      }
   return comp ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::lookup(const IdT* key, unsigned keylen, IdxT& first_match, IdxT& last_match) const
{
   if (!m_index)			// do we actually have an index?
      return false ;
   // binary search for the first match of the key
   IdxT lo(0), hi(m_numids) ;
   while (lo < hi)
      {
      IdxT mid = lo + (hi-lo)/2 ;
      int cmp = compareAt(m_index[mid],key,keylen) ;
      if (cmp < 0)
         {
         lo = mid + 1 ;
         }
      else // if (cmp >= 0)
         {
         hi = mid ;
         }
      }
   // if there was no match, we can bail out now
   if (lo >= m_numids || compareAt(m_index[lo],key,keylen) != 0)
      {
      return false ;
      }
   first_match = lo ;
   // binary search for the last match of the key
   lo = 0 ;
   hi = m_numids ;
   while (lo < hi)
      {
      IdxT mid = lo + (hi-lo)/2 ;
      int cmp = compareAt(m_index[mid],key,keylen) ;
      if (cmp <= 0)
         {
         lo = mid + 1 ;
         }
      else // if (cmp > 0)
         {
         hi = mid ;
         }
      }
   last_match = lo ;
   if (lo > 0) --last_match ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::enumerateSegment(IdxT startpos, IdT firstID, IdT lastID,
   unsigned minlen, unsigned maxlen, const std::function<EnumFunc>& fn,
   const std::function<FilterFunc>& filter) const
{
   if (minlen < 1)
      minlen = 1 ;
   if (maxlen < minlen)
      maxlen = minlen ;
   // use the unigram frequency table to quickly skip over all phrases starting with a word
   //   that fails to pass the filter function
   for (IdT i = firstID ; i < lastID ; ++i)
      {
      IdxT freq = getFreq(i) ;
      if (!filter || filter(this,&firstID,1,freq,maxlen>1))
         {
	 if (maxlen == 1)
	    {
	    // go right to calling the enumeration function, as there is no further checking needed
	    fn(this,&firstID,1,freq,startpos) ;
	    }
         else if (freq > 1)
            {
            if (!enumerate(startpos,startpos+freq,minlen,maxlen,fn,filter))
               return false ;
            }
         else
            {
            // special-case singletons; go from long to short to match the user function
            //   invocations elsewhere
            for (size_t len = maxlen ; len >= minlen ; --len)
               {
	       if (!filter || filter(this,m_ids+m_index[startpos],len,1,false))
		  fn(this,m_ids + m_index[startpos],len,1,startpos) ;
               }
            }
         }
      startpos += freq ;
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::enumerate(IdxT startpos, IdxT endpos, unsigned minlen, unsigned maxlen,
   const std::function<EnumFunc>& fn, const std::function<FilterFunc>& filter) const
{
   IdxT keystart[maxlen+1] ;
   IdT keyval[maxlen+1] ;
   for (unsigned i = 0 ; i < maxlen ; i++)
      {
      keystart[i+1] = startpos ;
      keyval[i] = idAt(m_index[startpos] + i) ;
      }
   for (IdxT idx = startpos+1 ; idx < endpos ; ++idx)
      {
      // compare the key for the current position in the index to that
      //   of the previous position, and figure out the length of the
      //   common prefix
      unsigned common = 0 ;
      for ( ; common < maxlen ; ++common)
         {
         IdxT pos = m_index[idx] + common ;
         if (keyval[common] != idAt(pos))
            break ;
         }
      // if the common prefix is less than 'maxlen', invoke the
      //   caller's function for each length from prefix-len to
      //   'maxlen', provided that the phrase passes the filter
      //   function
      for (size_t len = maxlen ; len > common ; --len)
	 {
	 if (len >= minlen)
	    {
	    size_t freq = idx - keystart[len] ;
	    if (!filter || filter(this,keyval,len,freq,false))
	       {
	       fn(this,keyval, len, freq, keystart[len]) ;
	       }
	    }
	 // update the first occurrence of a key of the current length
	 keystart[len] = idx ;
	 keyval[len-1] = idAt(m_index[idx]+len-1) ;
	 }
      }
   // process the final key at each length
   for (size_t len = maxlen ; len >= minlen ; --len)
      {
      size_t freq = endpos - keystart[len] ;
      if (!filter | filter(this,keyval,len,freq,false))
         {
         fn(this, keyval, len, freq, keystart[len]) ;
         }
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void SuffixArray<IdT,IdxT>::enumerate_segment(const void* in, void*)
{
   const Job* info = reinterpret_cast<const Job*>(in) ;
   info->index->enumerateSegment(info->startpos,info->startID,info->stopID,info->minlen,info->maxlen,
      info->fn,info->filter) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::enumerateParallel(unsigned minlen, unsigned maxlen,
   const std::function<EnumFunc>& fn, const std::function<FilterFunc>& filter) const
{
   // split the suffix array into segments on first-word boundaries; we
   //   use 32 times the number of threads to keep overhead reasonably
   //   low while avoiding long waits for stragglers at the end
   ThreadPool *tpool = ThreadPool::defaultPool() ;
   size_t num_segments(tpool->numThreads() * 32) ;
   if (num_segments == 0)
      return enumerateSegment(getFreq(m_sentinel),1,vocabSize(),minlen,maxlen,fn,filter) ;
   size_t segment_size((m_numids - getFreq(m_sentinel) + num_segments-1) / num_segments) ;
   size_t prev_start(getFreq(m_sentinel)) ;
   size_t count(0) ;
   size_t prev_id(1) ;
   bool success(true) ;
   std::vector<Job> orders(num_segments, Job(this,minlen,maxlen,fn,filter)) ;
   size_t jobnum(0) ;
   for (size_t i = 1 ; i < vocabSize() ; ++i)
      {
      count += getFreq(i) ;
      if (count >= segment_size)
         {
         orders[jobnum].startpos = prev_start ;
         orders[jobnum].startID = prev_id ;
         orders[jobnum].stopID = i+1 ;
         tpool->dispatch(&enumerate_segment,&orders[jobnum],nullptr) ;
         jobnum++ ;
         prev_start += count ;
         prev_id = i+1 ;
         count = 0 ;
         }
      }
   // handle the leftover at the end
   orders[jobnum].startpos = prev_start ;
   orders[jobnum].startID = prev_id ;
   orders[jobnum].stopID = vocabSize() ;
   tpool->dispatch(&enumerate_segment,&orders[jobnum],nullptr) ;
   tpool->waitUntilIdle() ;
   return success ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void SuffixArray<IdT,IdxT>::clear()
{
   //TODO
   return ;
}

//----------------------------------------------------------------------------

// end of namespace Fr
} ;


// end of file sufarray.cc //
