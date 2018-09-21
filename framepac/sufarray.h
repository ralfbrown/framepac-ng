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

#ifndef _Fr_SUFARRAY_H_INCLUDED
#define _Fr_SUFARRAY_H_INCLUDED

#include <functional>
#include "framepac/file.h"
#include "framepac/mmapfile.h"
#include "framepac/range.h"

namespace Fr {

// forward declaration
class BitVector ;

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
class SuffixArray
   {
   public:
      typedef bool FilterFunc(const SuffixArray*, const IdT* key, unsigned keylen, size_t freq, bool all) ;
      // all==true means eliminate all phrases starting with this key; all==false means eliminate only
      //   this exact key
      typedef bool EnumFunc(const SuffixArray*, const IdT* key, unsigned keylen, size_t freq, IdxT first_match) ;
      static constexpr IdT ErrorID { IdT(~0) } ;
   public:
      SuffixArray() {}
      SuffixArray(const SuffixArray&) = delete ;
      SuffixArray(const IdT* ids, IdxT num_ids, IdT num_types, IdT newline, const IdxT* freqs = nullptr)
	 { generate(ids,num_ids, num_types, newline, freqs) ; }
      ~SuffixArray()
	 {
	 if (!m_mmap && !m_readonly)	// if memory-mapped, we didn't allocate any arrays
	    {
	    delete[] m_index ;
	    if (!m_external_freq)
	       delete[] m_freq ;
	    if (!m_external_ids)
	       delete[] m_ids ;
	    }
	 m_ids = nullptr ;
	 m_index = nullptr ;
	 m_freq = nullptr ;
	 m_numids = 0 ;
	 m_types = 0 ;
	 }
      SuffixArray& operator= (const SuffixArray&) = delete ;

      bool load(const char* filename, bool allow_mmap = true, const IdT* using_ids = nullptr) ;
      // load from open file starting at current file position
      bool load(CFile&, const char* filename, bool allow_mmap = true, const IdT* using_ids = nullptr) ;
      bool loadMapped(const char*filename, off_t base_offset = 0, const IdT* using_ids = nullptr) ;
      // load starting from specified position in mmap'ed file
      bool loadFromMmap(const char* mmap_base, size_t mmap_len, const IdT* using_ids = nullptr) ;
      bool save(CFile&, bool include_ids = true) const ;

      bool generate(const IdT* ids, IdxT num_ids, IdT num_types,
		    IdT mapped_newline, const IdxT* freqs = nullptr) ;

      IdxT indexSize() const { return m_numids ; }
      IdT vocabSize() const { return  m_types ; }
      IdT idAt(IdxT N) const { return (N < m_numids) ? m_ids[N] : ErrorID ; }
      IdxT indexAt(IdxT N) const { return m_index[N] ; }
      IdxT getFreq(IdT N) const { return (m_freq && N < m_types) ? m_freq[N] : (IdxT)0 ; }
      bool lookup(const IdT* key, unsigned keylen, IdxT& first_match, IdxT& last_match) const ;

      bool enumerate(Range<IdxT> positions, Range<unsigned> lengths,
	 const std::function<EnumFunc>& fn, const std::function<FilterFunc>& filter) const ;
      bool enumerateSegment(Range<IdxT> positions, size_t offset, Range<IdT> IDs, Range<unsigned> lengths,
	 const std::function<EnumFunc>& fn, const std::function<FilterFunc>& filter) const ;
      bool enumerateParallel(Range<unsigned> lengths, const std::function<EnumFunc>& fn,
	 const std::function<FilterFunc>& filter, bool async = false) const ;
      void finishParallel() const ; // wait until the preceding enumerateParallel(...,true) finishes
      void clear() ;

      operator bool () const { return m_index != nullptr ; }

      void setSentinel(IdT sent) { m_sentinel = sent ; }
      void setFreqTable(IdxT* freq, bool external = true) { m_freq = freq ; m_external_freq = external ; }

   protected:
      struct Job
	 {
	    const SuffixArray* index ;
#if defined(__GCC__) && __GCC__ < 5
	    // GCC 4.8 breaks async version of enumerateParallel unless we make a copy of the closures,
	    //   even though the original closure we are referencing lives until after the following
	    //   finishParallel
	    const std::function<EnumFunc> fn ;
	    const std::function<FilterFunc> filter ;
#else
	    const std::function<EnumFunc>& fn ;
	    const std::function<FilterFunc>& filter ;
#endif /* GCC < 5 */
	    Range<IdxT> positions ;
	    Range<IdT> IDs ;
	    Range<unsigned> lengths ;
	    size_t offset ;
	 public:
	    Job(const SuffixArray* sa, Range<unsigned> lens, const std::function<EnumFunc>& enum_fn,
	       const std::function<FilterFunc>& filter_fn)
	       : index(sa), fn(enum_fn), filter(filter_fn), lengths(lens)
	       {}
	 } ;

   protected:
      template <typename I>
      bool Create(const I* ids, IdxT* index, IdxT num_ids, IdT num_types, const IdxT* freqs = nullptr) ;
      template <typename I>
      IdT convertEOL(I id, IdT num_types) const ;
      template <typename I>
      IdxT* bucketBoundaries(const I* ids, IdxT num_ids, IdT num_types, const IdxT* freqs) ;
      static IdxT* copyBucketBoundaries(const IdxT* bounds, IdT num_types) ;
      template <typename I>
      void classifyLS(BitVector& ls_types, const I* ids, IdxT num_ids, IdT num_types) ;
      template <typename I>
      void induce(const I* ids, IdxT* SA, IdxT num_ids, IdT num_types, IdxT* buckets,
	          const BitVector& ls_types) ;

      static int compare(IdT, IdT) ;
      int compare(const IdT*, const IdT*, unsigned keylen) const ;
      int compareAt(IdxT, IdxT, unsigned keylen) const ;
      int compareAt(IdxT, const IdT*, unsigned keylen) const ;

      static void enumerate_segment(const void* in, void* out) ;

   protected:
      MemMappedFile m_mmap ;
      IdT*       m_ids { nullptr } ;
      IdxT*      m_index { nullptr } ;
      IdxT*      m_freq { nullptr } ;
      IdxT       m_numids { 0 } ;
      IdT        m_types { 0 } ;
      IdT        m_sentinel { 0 } ;
      IdT        m_newline { IdT(-1) } ;
      IdxT       m_last_linenum { IdxT(-1) } ;
      bool       m_external_ids { true } ;
      bool       m_external_freq { false } ;
      bool	 m_readonly { false } ;

      // magic values for serializing
      static constexpr auto signature = "\x7FSufArray" ;
      static constexpr unsigned file_format = 1 ;
      static constexpr unsigned min_file_format = 1 ;
   } ;

//----------------------------------------------------------------------------

extern template class SuffixArray<uint32_t,uint32_t> ;

// end of namespace Fr
} ;

#endif /* !_Fr_SUFARRAY_H_INCLUDED */

// end of file sufarray.h //
