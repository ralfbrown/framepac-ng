/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-08-27					*/
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

#ifndef _Fr_WORDCORPUS_H_INCLUDED
#define _Fr_WORDCORPUS_H_INCLUDED

#include <cstdio>

#include "framepac/bidindex.h"
#include "framepac/builder.h"
#include "framepac/byteorder.h"
#include "framepac/cstring.h"
#include "framepac/file.h"
#include "framepac/mmapfile.h"
#include "framepac/sufarray.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

/************************************************************************/
/************************************************************************/

template <> template <>
inline bool Fr::HashTable<Fr::CString, uint32_t>::isEqual(Fr::CString s1, Fr::CString s2) const
{
   const char *str1 = s1.str() ;
   const char *str2 = s2.str() ;
   return str1 == str2 || (str1 && str2 && strcmp(str1,str2) == 0) ;
}

/************************************************************************/
/************************************************************************/

class WordCorpusHeader
   {
   public:
      uint64_t m_numwords ;		// number of tokens in the corpus
      uint64_t m_vocabsize ;		// number of types in the corpus
      uint64_t m_last_linenum ;		// highest word ID before hitting EOL records		
      uint64_t m_rare_ID ;		// ID of the <rare> token class
      uint64_t m_rare_threshold ;	// frequency below which to substitute <rare> class for token
      uint64_t m_wordmap ;		// offset of vocabulary
      uint64_t m_wordbuf ;		// offset of text
      uint64_t m_contextmap ;		// offset of context map
      uint64_t m_fwdindex ;		// offset of suffix-array index in forward direction
      uint64_t m_revindex ;		// offset of suffix-array index in reverse direction
      uint64_t m_freq { 0 } ;		// offset of word frequencies
      uint64_t m_attributes { 0 } ;	// offset of optional attributes for each token in the text
      uint64_t m_pad[16] ;		// padding for future expansion of header
      uint8_t  m_idsize ;		// sizeof(IdT)
      uint8_t  m_idxsize ;		// sizeof(IdxT)
      bool     m_keep_linenumbers ;
      char     m_pad2[13] ;		// padding for future expansion of header
   } ;

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
class WordCorpusT
   {
   public: // types
      typedef IdT ID ;
      typedef IdxT Index ;
      typedef ParallelBufferBuilder<IdT,1> IDBufferBuilder ;
      typedef Fr::SuffixArray<IdT,IdxT> SufArr ;
      typedef Fr::HashTable<Fr::CString,IdT> Map ;
      typedef Fr::BidirIndex<Fr::CString,IdT> BiMap ;
      typedef bool SAEnumFunc(const IdT* key, unsigned keylen, size_t freq,
	 		      const SufArr*, IdxT first_match,
			      void *user_arg) ;
      typedef bool AttrCheckFunc(const char *word) ;

   public: // constants
      static const char signature[] ;
      static constexpr ID ErrorID { ID(~0) } ;
      static constexpr int file_format { 2 } ;
      static constexpr int min_file_format { 2 } ;

   public: // methods
      WordCorpusT() ;
      WordCorpusT(const WordCorpusT&) = delete ;
      WordCorpusT(const char* filename, bool readonly = false) ;
      WordCorpusT(CFile& fp, const char* filename, bool readonly = false) ;
      void operator= (const WordCorpusT&) = delete ;
      ~WordCorpusT() ;

      static bool isCorpusFile(const char *filename) ;

      bool load(const char* filename, bool allow_mmap = true) ;
      bool load(CFile&, const char* filename, bool allow_mmap = true) ;
      bool loadContextEquivs(const char* filename, bool force_lowercase = true) ;
      size_t loadAttribute(const char* filename, unsigned attr_bit, bool add_words = false) ;
      bool save(const char* filename) const ;
      bool save(CFile&) const ;
      bool discardText() ;
      bool discardAttributes() ;
      bool discardContextEquivs() ;

      IdxT corpusSize() const { return m_wordbuf.size() ; }
      IdT vocabSize() const { return m_wordmap->indexSize() ; }

      IdT findID(const char* word) const ;
      IdT findOrAddID(const char* word) ;
      IdT addWord(const char* word) ;
      bool addWord(IdT word) ;
      bool addNewline() ;

      bool rareWordThreshold(IdxT thresh, const char* token) ;

      bool setAttributeIf(unsigned attr_bit, AttrCheckFunc* fn) ;
      void setAttributes(IdT word, uint8_t mask) const ;
      void setAttribute(IdT word, unsigned bit) const { setAttributes(word,1<<bit) ; }
      void clearAttributes(uint8_t mask) const ;
      void clearAttributes(IdT word, uint8_t mask) const
	 { if (word < m_attributes_alloc) m_attributes[word] &= ~mask ; }
      void clearAttribute(IdT word, unsigned bit) const { clearAttributes(word,1<<bit) ; }
      void clearAttribute(unsigned bit) const { clearAttribute(1<<bit) ; }

      bool createIndex(bool bidirectional = false) ;
      void freeTermFrequencies() ;
      bool freeIndices() ;
      bool lookup(const ID *key, unsigned keylen, Index &first_match, Index &last_match) const ;
      bool enumerateForward(unsigned minlen, unsigned maxlen, size_t minfreq, SAEnumFunc *fn, void *user_arg) ;
      bool enumerateForwardParallel(unsigned minlen, unsigned maxlen, size_t minfreq,
				    SAEnumFunc *fn, void *user_arg) ;
      bool enumerateReverse(unsigned minlen, unsigned maxlen, size_t minfreq, SAEnumFunc *fn, void *user_arg) ;
      bool enumerateReverseParallel(unsigned minlen, unsigned maxlen, size_t minfreq,
				    SAEnumFunc *fn, void *user_arg) ;

      IdxT rareWordThreshold() const { return  m_rare_thresh ; }
      size_t numContextEquivs() const { return m_contextmap->size() ; }

      IdT getID(IdxT N) const ;
      IdT getContextID(IdxT N) const ;
      IdT getContextID(const char* word) const ;
      IdT newlineID() const { return m_newline ; }
      IdT rareID() const { return m_rare ; }

      bool haveTermFrequencies() const { return m_freq != nullptr ; }
      IdxT getFreq(IdT N) const ;
      IdxT getFreq(const char* word) const ;

      const char* getWord(IdT N) const ;
      const char* getNormalizedWord(IdT N) const ;
      const char* newlineWord() const ;
      const char* rareWord() const ;
      const char* getWordForLoc(IdxT N) const ;

      IdxT getForwardPosition(IdxT N) const ;
      IdxT getReversePosition(IdxT N) const ;

      uint8_t attributes(IdT word) const
	 { return word < m_attributes_alloc ? m_attributes[word] : 0 ; }
      bool hasAttribute(IdT word, unsigned bit) const
	 { return (attributes(word) & (1<<bit)) != 0 ; }

      void setContextSizes(unsigned lcontext, unsigned rcontext) ;
      unsigned leftContextSize() const { return m_left_context ; }
      unsigned totalContextSize() const { return m_total_context ; }
      IdT positionalID(IdT word, int offset) const ;
      int offsetOfPosition(IdT position) const ;
      static int offsetOfPosition(IdT position, unsigned left_context, unsigned total_context) ;
      IdT wordForPositionalID(IdT position) const ;

      size_t longestContextEquiv() const { return m_max_context ; }

      bool printVocab(CFile&) const ;
      bool printWords(CFile&, size_t max = ~0) const ;
      bool printSuffixes(CFile&, bool by_ID = false, size_t max = 6) const ;

      // support functions for parallelized conversion of text into WordCorpusT
      IdxT reserveIDs(IdxT count, IdT *newline) ;
      void setID(IdxT N, IdT id) ;

   protected:
      bool loadMapped(const char* filename, off_t base_offset = 0) ;
      bool loadFromMmap(const char* mmap_base, size_t mmap_len) ;
      void computeTermFrequencies() ;
      bool createForwardIndex() ;
      bool createReverseIndex() ;
      bool enumerateForward(IdxT start, IdxT stop, unsigned minlen, unsigned maxlen, size_t minfreq,
			    SAEnumFunc *fn, void *user_arg) const ;
      bool enumerateReverse(IdxT start, IdxT stop, unsigned minlen, unsigned maxlen, size_t minfreq,
			    SAEnumFunc *fn, void *user_arg) const ;

   protected:
      MemMappedFile      m_mmap ;
      BiMap*		 m_wordmap ;
      IDBufferBuilder    m_wordbuf ;	// contains array of word IDs
      Map*		 m_contextmap ;
      SufArr   		 m_fwdindex ;
      SufArr   		 m_revindex ;
      IdxT*		 m_freq { nullptr } ;
      mutable uint8_t*   m_attributes { nullptr } ;
      IdT		 m_rare ;
      IdT		 m_newline ;
      IdT		 m_sentinel ;
      IdxT		 m_rare_thresh { 0 } ;
      IdxT		 m_last_linenum { (IdxT)~0 } ;
      mutable IdxT	 m_attributes_alloc { 0 } ;
      unsigned		 m_max_context { 0 } ;
      unsigned		 m_left_context { 0 } ;
      unsigned		 m_right_context { 0 } ;
      unsigned		 m_total_context { 1 } ;
      bool		 m_mapped { false } ;     // data is memory-mapped; don't free memory!
      bool		 m_readonly { false } ;
      bool		 m_keep_linenumbers { false } ;
   } ;

template <typename IdT, typename IdxT>
const char WordCorpusT<IdT,IdxT>::signature[] = "\x7FWordCorp" ;

//----------------------------------------------------------------------------

// the standard version, for corpora with up to 4 billion types and up to 4 billion tokens
extern template class WordCorpusT<uint32_t,uint32_t> ;
typedef WordCorpusT<uint32_t,uint32_t> WordCorpus ;

// an extra-large version for corpora with more than 4 billion tokens
extern template class WordCorpusT<uint32_t,UInt40> ;
typedef WordCorpusT<uint32_t,UInt40> WordCorpusXL ;

// save some space if the corpus vocabulary is less than 16 million types, at the
//   cost of a little slower processing
extern template class WordCorpusT<UInt24,uint32_t> ;
typedef WordCorpusT<UInt24,uint32_t> WordCorpusCompact ;

//----------------------------------------------------------------------------

// end of namespace Fr
} ;

#endif /* !_Fr_WORDCORPUS_H_INCLUDED */

// end of file wordcorpus.h //
