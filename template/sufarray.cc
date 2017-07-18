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

#include <cstring>
#include "framepac/bitvector.h"
#include "framepac/sufarray.h"

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
      std::memset(buckets,'\0',sizeof(IdxT)*(num_types+2)) ;
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
      memcpy(copy,bounds,sizeof(IdxT)*(num_types+1)) ;
      }
   return copy ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
template <typename I>
void SuffixArray<IdT,IdxT>::induce(const I* ids, IdxT* SA, IdxT num_ids, IdT num_types, IdxT* buckets,
	    const Fr::BitVector& ls_types)
{
   IdxT* bucket_ends = copyBucketBoundaries(buckets+1,num_types) ;
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
   delete [] buckets ;
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
   delete [] bucket_ends ;
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
   Fr::BitVector *ls_types = Fr::BitVector::create(num_ids+2) ;
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
   delete [] bucket_ends ;
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
   delete [] bucket_ends ;
   induce(ids, index, num_ids, num_types, buckets, *ls_types) ;
   ls_types->free() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::generate(const IdT *ids, IdxT num_ids, IdT num_types,
				     IdT mapped_newline, const IdxT *freqs)
{
   m_ids = ids ;
   m_numids = num_ids ;
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
   while (pos < keylen && (comp = compare(m_ids[idx1],key2[pos]) == 0))
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

// end of namespace Fr
} ;


// end of file sufarray.cc //
