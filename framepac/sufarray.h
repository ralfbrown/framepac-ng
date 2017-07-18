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

#ifndef _Fr_SUFARRAY_H_INCLUDED
#define _Fr_SUFARRAY_H_INCLUDED

#include "framepac/file.h"

namespace Fr {

// forward declaration
class BitVector ;

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
class SuffixArray
   {
   public:
      SuffixArray() {}
      SuffixArray(const SuffixArray&) = delete ;
      SuffixArray(const IdT* ids, IdxT num_ids, IdT num_types, IdT newline, const IdxT* freqs = nullptr)
	 { generate(ids,num_ids, num_types, newline, freqs) ; }
      ~SuffixArray() ;
      SuffixArray& operator= (const SuffixArray&) = delete ;

      bool generate(const IdT* ids, IdxT num_ids, IdT num_types,
		    IdT mapped_newline, const IdxT* freqs = nullptr) ;
      bool load(const char* filename) ;
      bool load(CFile& file) ; // load from open file starting at current file position
      bool load(void*& mmap) ; // load starting from specified position in mmap'ed file

      bool lookup(const IdT* key, unsigned keylen, IdxT& first_match, IdxT& last_match) const ;

      void clear() ;

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

   protected:
      const IdT* m_ids { nullptr } ;
      IdxT*      m_index { nullptr } ;
      IdxT       m_numids { 0 } ;
      IdT        m_newline { IdT(-1) } ;
      IdxT       m_last_linenum { IdxT(-1) } ;
   } ;

//----------------------------------------------------------------------------

// end of namespace Fr
} ;

#endif /* !_Fr_SUFARRAY_H_INCLUDED */

// end of file sufarray.h //
