/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018,2019 Carnegie Mellon University		*/
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

#ifndef __Fr_NGRAMS_H_INCLUDED
#define __Fr_NGRAMS_H_INCLUDED

#include "framepac/byteorder.h"

namespace Fr
{

/*
   Format of NGrams file:
   	Header
	Array[1..maxrank] of record pointers
	Array[1..maxrank] of count-of-count records
   	Array[1..maxrank] of global discount records
   	CountID map
	Array of Unigram Records
   	Array of U32 "next" pointers for Unigrams
	N-2 times:
           Array of Ngram Records for each ngram rank
	   Array of U32 "next" pointers
	   Array of smoothing discount factors for previous rank
	Array of Ngram Records for N-grams
	Array of smoothing discount factors for (N-1)-grams
        Fr::Vocabulary
	optional FrBWTIndex
	NOTE: everything but header and record pointers can occur in arbitrary
		order, since there are pointers in the header and pointer
	   	records

   Format of Header:
	signature "Ngram Statistics File\0"
	U8	file format version number (2)
	U8	n-gram length (maximum rank)
	U8	bits used for word ID in ngram record (24)
	U8	bits used for count ID in ngram record (16)
	U8	maximum count-of-counts value (8)
	U8	flags
	   	bit 0: model is case-sensitive (has not been lowercased)
	        bit 1: model is character-based
		bit 2: character-based model includes whitespace
		bits 3-7: reserved (0)
	U32	number of count IDs
	U64	offset of count-ID array
	U64	offset of first count-of-counts record (for unigrams)
	U64	offset of first global discounts record (for unigrams)
	U64	total count of words used for training this model
   	U64	file offset of Fr::Vocabulary
	U64	size of optional Fr::BWTIndex
	U64	offset of optional Fr::BWTIndex or 0
	U64	file offset of optional secondary Fr::Vocabulary or 0
	        used to adjust predictions for members of a word class
		(maps words to additional IDs not actually present in
		 the ngram data)
	U64	file offset of optional stop-word Fr::Vocabulary or 0
	        used to ignore all but certain terms when computing
		   probabilities, e.g. a content-word LM
	U8	affix sizes (0 = keep entire word,
			  else bits 7-4 = prefix chars, bits 3-0 = suffix chars)
        U8[7]	reserved (0)
	U64[2]	reserved (0)

   Format of record pointers record:
	U32	number of N-gram records
	U32	number of extra Ngram-only records for use in adjusting
	   	  predictions for members of a word class (default 0)
	U64	file offset of array of N-gram records
	U64	file offset of array of "next" pointers for N-grams
		each "next" pointer is a U32; there is one more pointer than
		N-gram records to simplify lookup code
	U64	file offset of array of discount values (FLOATs) for N-grams
		{optional, as it can be computed (slowly) at runtime}
	NOTE: last two elements are reserved (0) for the highest-rank n-grams,
		since there are neither continuations nor a higher rank to
		interpolate with

   Format of count-of-counts record:
	U32[8]   counts of freq 1..7 and 8+
	U32[8]   counts of ngrams w/ 1..7 and 8+ distinct continuations

   Format of global discounts record:
	U32[8]   scaled discounts for count values 1..7 and 8+
		   (actual discount multiplied by 2**31)
		   (discount for count value 0 is zero by definition)

   Format of unigram record:
	U32	unigram freq
	U32	number of class members with wordID

   Format of Ngram Record:
	16 bits:  count ID 		{bytes 0-1}
        24 bits:  word ID 		{bytes 2-4}
*/

/************************************************************************/
/************************************************************************/

#define LmSIGNATURE		"Ngram Statistics File"

/************************************************************************/
/************************************************************************/

// forward declaration to avoid pulling in all of framepac/file.h
class CFile ;

class Vocabulary ; // old-FramepaC class roughly equiv to BidirIndex<CString,uint32_t>

class NgramFileHeader
   {
   public:
      NgramFileHeader(size_t rank = 3) ;
      bool read(CFile&) ;
      bool write(CFile&) const ;

      // accessors
      uint64_t trainingSize() const { return m_trainingsize ; }
      size_t countIDmax() const { return m_num_countIDs ; }
      off_t countOfCounts() const { return (off_t)m_countofcounts_offset ; }
      off_t countIDoffset() const { return (off_t)m_countID_offset ; }
      off_t globalDiscounts() const { return (off_t)m_discounts_offset ; }
      off_t vocabOffset() const { return (off_t)m_vocab_offset ; }
      off_t surfaceVocabOffset() const { return (off_t)m_surfvocab_offset ; }
      off_t stopWordOffset() const { return (off_t)m_stopword_offset ; }
      uint64_t suffArraySize() const { return m_sarray_size ; }
      off_t suffArrayOffset() const { return (off_t)m_sarray_offset ; }
      uint8_t affixSizes() const { return m_affix_sizes ; }
      bool isCaseSensitive() const { return m_flags & NGH_CASED ; }
      bool isCharBased() const { return m_flags & NGH_CHARBASED ; }
      bool includesSpaces() const { return m_flags & NGH_SPACES ; }

      // manipulators
      void setTrainingSize(uint64_t size) { m_trainingsize = size ; }
      void setCountIDmax(size_t size) { m_num_countIDs = (uint32_t)size ; }
      void setCountOfCounts(off_t off) { m_countofcounts_offset = (uint64_t)off ; }
      void setCountIDoffset(off_t off) { m_countID_offset = (uint64_t)off ; }
      void setGlobalDiscounts(off_t off) { m_discounts_offset = (uint64_t)off ; }
      void setVocabOffset(off_t off) { m_vocab_offset = (uint64_t)off ; }
      void setSurfVocabOffset(off_t off) { m_surfvocab_offset = (uint64_t)off ; }
      void setStopWordOffset(off_t off) { m_stopword_offset = (uint64_t)off ; }
      void setSuffArraySize(uint64_t size) { m_sarray_size = size ; }
      void setSuffArrayOffset(off_t off) { m_sarray_offset = (uint64_t)off ; }
      void setAffixSizes(uint8_t sizes) { m_affix_sizes = sizes ; }
      void setCaseSensitive(bool cased) { m_flags &= ~NGH_CASED ; if (cased) m_flags |= NGH_CASED ; }
      void setCharBased(bool cased) { m_flags &= ~NGH_CHARBASED ; if (cased) m_flags |= NGH_CHARBASED ; }
      void setIncludeSpaces(bool cased) { m_flags &= ~NGH_SPACES ; if (cased) m_flags |= NGH_SPACES ; }

   protected:
      char 	m_signature[sizeof(LmSIGNATURE)] ;
      uint8_t	m_version ;
      uint8_t	m_rank ;
      uint8_t	m_wordID_bits ;
      uint8_t	m_countID_bits ;
      uint8_t	m_countofcounts_max ;
      uint8_t	m_flags ;
      uint32_t	m_num_countIDs ;
      uint64_t	m_countID_offset ;
      uint64_t	m_countofcounts_offset ;
      uint64_t	m_discounts_offset ;
      uint64_t	m_trainingsize ;
      uint64_t	m_vocab_offset ;
      uint64_t  m_sarray_size ;
      uint64_t  m_sarray_offset ;
      uint64_t  m_surfvocab_offset ;
      uint64_t  m_stopword_offset ;
      uint8_t   m_affix_sizes ;
      uint8_t   m_reserved0[7] ;
      uint64_t	m_reserved[2] ;
} ;

/************************************************************************/
/************************************************************************/

class RecordPointerRecord
   {
   public:
      RecordPointerRecord() ;
      bool write(CFile&) const ;

      // accessors
      uint32_t count() const { return m_count ; }
      uint64_t records() const { return m_records ; }
      uint64_t pointers() const { return m_pointers ; }
      uint64_t discounts() const { return m_discounts ; }

      // modifiers
      void setCount(uint32_t count) { m_count = count ; }
      void setRecords(off_t offset) { m_records = offset ; }
      void setPointers(off_t offset) { m_pointers = offset) ; }
      void setDiscounts(off_t offset) { m_discounts = offset ; }
   private:
      uint32_t m_count ;
      uint32_t m_reserved ;
      uint64_t m_records ;
      uint64_t m_pointers ;
      uint64_t m_discounts ;
   } ;

/************************************************************************/
/************************************************************************/

class LmCountMap
   {
   public:
      LmCountMap(size_t numcounts) ;
      LmCountMap(size_t numcounts, char* counts) ;
      LmCountMap(size_t numcounts, CFile&) ;
      ~LmCountMap() ;

      bool write(CFile&) const ;

      uint32_t countID(size_t count) ;
      uint32_t count(size_t countID) const ;
      size_t numCounts() const { return m_numcounts ; }
      size_t numCountsUsed() const ;
      bool OK() const { return m_counts != 0 ; }

   private:
      size_t	m_numcounts ;
      uint32_t* m_counts ;
      bool	m_allocated ;
   } ;

/************************************************************************/
/************************************************************************/

class LmCountOfCounts
   {
   public:
      void* operator new(size_t, void* where) { return where ; }
      LmCountOfCounts() ;
      LmCountOfCounts(const char* data) ;
      ~LmCountOfCounts() {}

      bool write(CFile&) const ;

      // accessors
      uint32_t frequency(size_t N) const
	 { return (N < LmCOUNTOFCOUNTS_MAX) ? m_freqs[N] : m_freqs[LmCOUNTOFCOUNTS_MAX] ; }
      uint32_t continuations(size_t N) const
	 { return (N < LmCOUNTOFCOUNTS_MAX) ? m_conts[N] : m_conts[LmCOUNTOFCOUNTS_MAX] ; }
      static size_t dataSize()
	 { return 2*LmCOUNTOFCOUNTS_MAX*sizeof(uint32_t) ; }

      // modifiers
      void setFrequency(size_t N, uint32_t freq)
	 { if (N > LmCOUNTOFCOUNTS_MAX) N = LmCOUNTOFCOUNTS_MAX ;
	   m_freqs[N] = freq ; }
      void incrFrequency(size_t N, uint32_t amt = 1)
	 { if (N > LmCOUNTOFCOUNTS_MAX) N = LmCOUNTOFCOUNTS_MAX ;
	   m_freqs[N] += amt ; }
      void setContinuations(size_t N, uint32_t cont)
	 { if (N > LmCOUNTOFCOUNTS_MAX) N = LmCOUNTOFCOUNTS_MAX ;
	   m_conts[N] = cont ; }
      void incrContinuations(size_t N, uint32_t amt = 1)
	 { if (N > LmCOUNTOFCOUNTS_MAX) N = LmCOUNTOFCOUNTS_MAX ;
	   m_conts[N] += amt ; }
      void clearContinuations() ;

   private:
      uint32_t	m_freqs[LmCOUNTOFCOUNTS_MAX+1] ;
      uint32_t	m_conts[LmCOUNTOFCOUNTS_MAX+1] ;
   } ;

/************************************************************************/
/************************************************************************/

class LmNgramDiscounts
   {
   public:
      LmNgramDiscounts() ;

      bool read(CFile&) ;
      bool write(CFile&) const ;

      // accessors
      double discount(size_t N) const
	 { if (N == 0) return 0.0 ;
	   if (N > LmCOUNTOFCOUNTS_MAX) N = LmCOUNTOFCOUNTS_MAX ;
	   return (m_discounts[N-1] / (double)LmDISCOUNT_SCALE_FACTOR) ;
	 }

      // modifiers
      void discount(size_t N, double disc)
	 { if (N > 0 && N <= LmCOUNTOFCOUNTS_MAX)
	       m_discounts[N-1] = (uint32_t)(disc * LmDISCOUNT_SCALE_FACTOR + 0.5) ;
	 }

   private:
      uint32_t	m_discounts[LmCOUNTOFCOUNTS_MAX] ;
   } ;

/************************************************************************/
/************************************************************************/

class NgramRecord
   {
   public:
      NgramRecord() {}
      NgramRecord(size_t wordID, size_t countID = 1) : m_count(countID), m_wordID(wordID) {}
      NgramRecord(const NgramRecord& orig) = default ;
      ~NgramRecord() {}

      bool write(CFile&) const ;

      // accessors
      LmWordID_t wordID() const { return m_wordID.load() ; }
      LmWordCount_t wordCount() const { return m_count.load() ; }
      uint32_t wordCount(const LmCountMap* countmap) const
	 { LmWordCount_t count = wordCount() ; 
	   return countmap ? countmap->count(count) : count ; }

      // manipulators
      void setCount(size_t countID) { m_count = countID ; }

   private:
      Int16 m_count ;
      Int24 m_wordID ;
   } ;

/************************************************************************/
/************************************************************************/

class NGramHistory
   {
   public:
      NGramHistory() {}
      ~NGramHistory() {}

      // accessors
      uint32_t startLoc() const { return m_hist ; }
      uint32_t endLoc() const { return m_hist + m_count - 1 ; }
      uint32_t wordID() const { return m_hist ; }
      uint32_t count() const { return m_count ; }

      // manipulators
      void setID(uint32_t ID) { m_hist = ID ; }
      void setCount(size_t cnt) { m_count = (uint32_t)cnt ; }
      void setLoc(uint32_t first, uint32_t last)
	 { m_hist = first ; m_count = (last - first) + 1 ; }
//      void setLoc(BWTLocation loc) { m_hist = loc.first() ; m_count = loc.rangeSize() ; }

   private:
      uint32_t m_hist ;			// location in NGramsFile; raw word ID for unsmoothed/joint-prob model
      uint32_t m_count ;
   } ;

/************************************************************************/
/************************************************************************/

// to update from old FramepaC
class NGramsFile
   {
   public:
      NGramsFile(const char* filename, size_t max_rank_to_use,
		   bool allow_mmap = true,
		   bool preload = true) ;
      ~NGramsFile() ;
      static bool verifyFormat(const char* filename) ;

      static double smoothingFactor(const LmNgramDiscounts* discounts,
				    const LmCountOfCounts* counts, size_t total_count) ;

      // accessors
      bool OK() const { return m_OK ; }
      bool isCaseSensitive() const { return m_case_sensitive ; }
      bool isCharBased() const { return m_char_based ; }
      bool includesSpaces() const { return m_include_spaces ; }
      uint8_t affixSizes() const { return m_affix_sizes ; }
      FrLMSmoothing smoothing() const { return m_smoothing ; }
      Vocabulary* vocabulary() const { return m_vocab ; }
      Vocabulary* surfaceVocabulary() const { return m_surfvocab ; }
      Vocabulary* stopwordVocabulary() const { return m_stopwords ; }
      size_t maxNgramLength() const { return m_maxlength ; }
      uint64_t trainingSize() const { return m_numtokens ; }
      size_t vocabularySize() const { return m_vocabsize ; }
      size_t classSize(LmWordID_t ID) const
	 { return ID < m_vocabsize ? m_unigrams[ID].classSize() : 1 ; }
      uint32_t unigramFreq(size_t ID) const
	 { return ID < m_vocabsize ? m_unigrams[ID].frequency() : 0 ; }
      size_t ngramCount(size_t N) const
	 { return (N>=1 && N <= maxNgramLength()) ? m_record_counts[N] : 0 ; }
      size_t recordOffset(const LmWordID_t* IDs, size_t rank) const ;
      uint32_t getCount(size_t rank, uint32_t index) const ;
      uint32_t frequency(const LmWordID_t* IDs, size_t numIDs,
			 uint32_t* rec_index = 0) const ;
      uint32_t frequency(size_t prev_rank,LmWordID_t ID,uint32_t index) const ;
      uint32_t frequency(size_t prev_rank,LmWordID_t ID,
			 uint32_t* rec_index) const ;
      size_t longestMatch(const LmWordID_t* IDs, size_t numIDs) const ;
      double probability(const LmWordID_t* IDs, size_t numIDs,
			 size_t* max_exist = 0) const ;
      double probability(NGramHistory* history, size_t numIDs,
			 LmWordID_t ID, size_t* max_exist = 0) const ;
      double probability(const char* const* words, size_t numwords,
			 size_t* max_exist = 0) const ;
      double probability(const List* words) const ;

      double perplexity(size_t rank, bool use_entropy) const ;

      // manipulators
      //bool unmemmap() ;
      void smoothing(FrLMSmoothing sm) { m_smoothing = sm ; }

      // I/O
      static bool convert(const char* filename, const FrBWTIndex* index,
			    Vocabulary* vocab, size_t ngram_rank,
			    size_t precomp_rank = ~0, bool char_based = false,
			    bool include_spaces = false,
			    Vocabulary* surfvocab = 0, Vocabulary* stopwords = 0) ;
      static bool convert(const char* filename, const char* const* files,
			    size_t ngram_rank, size_t precomp_rank = ~0,
			    size_t scaling_factor = 1, bool char_based = false,
			    bool include_spaces = false) ;
      bool dump(CFile& out, const char* modelname,
		  bool run_verbosely = false) const ;

   private:
      bool allocPerRankData() ;
      void loadCountOfCounts(const char* data) ;
      void loadRecords(const char* recptrs, CFile&, const char* mappedmem, size_t max_touchmem_rank = 0) ;
      void computeSmoothingFactors(size_t rank) ;
      uint32_t locateNext(LmWordID_t ID, size_t rank, uint32_t index) const ;
      double discountFactor(size_t rank, size_t count) const ;
      double smoothingFactor(size_t rank, uint32_t index) const ;
      double rawCondProbability(const LmWordID_t* history, size_t histlen, LmWordID_t ID,
	 			  size_t class_size) const ;
      double rawCondProbability(NGramHistory* history, size_t histlen, LmWordID_t ID, size_t class_size) const ;
      double probabilityKN(const LmWordID_t* IDs, size_t numIDs, size_t* max_exist = 0) const ;
      double probabilityKN(const NGramHistory* history, NGramHistory* newhistory,
	 		     size_t numIDs, LmWordID_t ID, size_t* max_exist = 0) const ;
      double probabilityMax(const LmWordID_t* IDs, size_t numIDs, size_t* max_exist = 0) const ;
      double probabilityMax(NGramHistory* history, size_t numIDs, LmWordID_t ID, size_t* max_exist = 0) const ;
      double probabilityMean(const LmWordID_t* IDs, size_t numIDs, size_t* max_exist = 0) const ;
      double probabilityMean(NGramHistory* history, size_t numIDs, LmWordID_t ID, size_t* max_exist = 0) const ;
      double probabilityBackoff(const LmWordID_t* IDs, size_t numIDs, size_t* max_exist = 0) const ;
      double probabilityBackoff(NGramHistory* history, size_t numIDs, LmWordID_t ID,
	 			  size_t* max_exist = 0) const ;
      void initZerogram(NGramHistory* history) const ;

   private:
      Vocabulary* m_vocab ;			// map from word to ID
      Vocabulary* m_surfvocab ;			// map adjusting class members
      Vocabulary* m_stopwords ;			// which words to ignore
      LmUnigramRecord* m_unigrams ;		// array of unigram records
      LmNgramRecord** m_ngram_records ;		// arrays for 2..N-grams
      uint32_t** m_ngram_next ;			// arrays of "next" ptrs
      uint32_t** m_ngram_disc ;			// arrays of smoothing values
      size_t* m_record_counts ;			// array of #records per rank
      LmNgramDiscounts* m_discounts ;		// global discount factors
      size_t m_maxlength ;			// maximum ngram length in file
      size_t m_vocabsize ;			// number of distinct words
      size_t m_numcounts ;			// number of distinct counts
      uint64_t m_numtokens ;			// training data size
      LmCountMap* m_counts ;			// countID -> value mapping
      double m_uniform ;			// uniform 0-gram probability
      LmCountOfCounts* m_countofcounts ;	//
      LmCountOfCounts* m_cumcounts ;		// cumulative counts-of-counts
      FrFileMapping* m_fmap ;			// mmap()ing for file, if any
      FrLMSmoothing m_smoothing ;		// how to smooth probabilities
      uint8_t m_affix_sizes ;
      bool m_case_sensitive ;
      bool m_char_based ;			// is LM character-based?
      bool m_include_spaces ;			// char-based LM incl spaces?
      bool m_OK ;
   } ;
*/

/************************************************************************/
/************************************************************************/
//  this class provides a unified interface to the various different
//    supported language model file formats

// to update from old FramepaC
class NGramModel
   {
   private: // methods
      LmWordID_t findWordID_SRI(const char* word) const ;

   protected: // methods
      static bool verifyBWTSignature(const char* filename) ;
      static bool verifyBBOSignature(const char* filename) ;
      static bool verifyNGMSignature(const char* filename)
	 { return LmNGramsFile::verifyFormat(filename) ; }
      static bool verifySRISignature(const char* filename) ;
      void init(const char* filename, const char* sourcefile,
		double weight, double context, double discount,
		Symbol* smoothing, const char* ident = 0) ;
      double freq2prob(double freq) const ;
      double adjustProbability(const char* surf, LmWordID_t ID,
			       double prob) const ;
   protected:
      NGramModel() {}
   public:
      NGramModel(const class LModelConfig*) ;
      virtual ~NGramModel() ;
      static NGramModel* newModel(const char* filename, const char* sourcefile,
				    size_t max_ngrams, double weight,
				    double context_weight, double discount,
				    Symbol* smooth_name, const char* ident,
				    const List* sequences, const List* fallbacks, bool separate_feature) ;
      static NGramModel* newModel(const char* filename, size_t max_ngram,
				    const char* sourcefile, const char* ident, bool separate_feature) ;
      // note: above method sets smoothing to NONE
      static NGramModel* newModel(const class LModelConfig*, bool separate_feature) ;

      // utility functions
      static bool verifySignature(const char* filename)
	 { return (verifyBWTSignature(filename) || 
		   verifyBBOSignature(filename) ||
		   verifyNGMSignature(filename) ||
		   verifySRISignature(filename)) ; }

      static const char* remapWord(const char* word, SymHashTable* stem_ht,
				   bool& is_possessive, bool is_cased = true,
				   uint8_t affix_sizes = 0, bool* using_affix = 0) ;
      static double smoothedLog(double x) ;

      // accessors
      bool OK() const
	 { return (m_index != 0 && m_vocab != 0) ||
	      (m_ngrams != 0 && m_ngrams->OK() && m_vocab != 0) ||
	      (m_srilm && m_srivcb) ; }
      virtual bool isCaseSensitive() const ;
      virtual bool isCharBased() const ;
      virtual bool includesSpaces() const ;
      virtual uint8_t affixSizes() const ;
      virtual uint64_t trainingSize() const ;
      size_t modelNumber() const { return m_model_number ; }
      size_t maxNgramLength() const { return m_max_ngram ; }
      bool usingLogSpace() const { return m_logspace ; }
      bool usingJointProb() const { return m_jointprob ; }
      unsigned featureID() const { return m_featureID ; }
      LmWordID_t wordID_begsent() const { return m_begsent_id ; }
      Symbol* word_begsent() const { return m_begsent_sym ; }
      Symbol* word_endsent() const { return m_endsent_sym ; }
      FrLMSmoothing smoothing() const { return m_smoothing ; }
      Symbol* smoothingName() const { return m_smoothing_name ; }
      double discountMass() const { return m_discount_mass ; }
      LmWordID_t wordID_unknown() const { return m_unknown_id ; }
      Symbol* wordForID(LmWordID_t ID) const ;
      Vocabulary* vocabulary() const { return m_vocab ; }
      Vocabulary* surfaceVocabulary() const { return m_surfvocab ; }
      FrBWTIndex* ngramModel() const { return m_index ; }
      LmNGramsFile* ngramsFile() const { return m_ngrams ; }
      class Ngram* sriModel() const { return m_srilm ; }
      class Vocab* sriVocab() const { return m_srivcb ; }
      NGramModel* sourceModel() const { return m_sourcemodel ; }
      double weight() const { return  m_weight ; }
      double baseWeight() const { return m_baseweight ; }
      double contextWeight() const { return m_contextweight ; }
      const char* name() const { return m_name ; }
      const char* sectionName() const { return m_ident ; }
      const List* sequences() const { return m_sequences ; }
      const List* fallbacks() const { return m_fallbacks ; }
      const List* fallbacks(Symbol*) const ;
      virtual LmWordID_t getUnknownID() const ;

      virtual double rawProbability(size_t numwords, const LmWordID_t* IDs, size_t* max_exist) const ;
      virtual double rawProbability(size_t numwords, NGramHistory* hist,
				    LmWordID_t ID, size_t* max_exist) const ;

      static int string2words(const char* string, char**& wordlist) ;
      LmWordID_t findWordID(const char* word) const ;
      bool isStopWord(const char* word) const ;
      size_t longestMatch(const LmWordID_t* IDs, size_t numIDs) const ;

      double rawProbability(const LmWordID_t* IDs, size_t numIDs,
			    size_t* max_exist = 0,
			    const char* surf_word = 0) const ;
      double rawProbability(NGramHistory* history, size_t numIDs,
			    LmWordID_t ID, size_t* max_exist = 0, const char* surf_word = 0) const ;
      double probability(const LmWordID_t* IDs, size_t numwords,
			 size_t* max_exist = 0, const char* surf_word = 0) const ;
      double probability(NGramHistory* history, size_t numIDs,
			 LmWordID_t ID, size_t* max_exist, const char* surf_word = 0) const ;
      double probability(const List* ngram) const ;
      size_t frequency(const List* words) const ;
      virtual size_t frequency(const LmWordID_t* IDs, size_t numwords) const ;
      virtual size_t classSize(LmWordID_t ID) const ;
      virtual double perplexity(size_t rank) const ;
      double scaledProbability(double prob) const ;

      // manipulators
      void useLogSpace(bool logsp = true) ;
      void useJointProb(bool joint = true) { m_jointprob = joint ; }
      void maxNgramLength(size_t len) ;
      void setFeatureID(unsigned ID) { m_featureID = ID ; }
      void setSentMarkers() ;
      void smoothing(Symbol* smoothing_name) ;
      void smoothing(FrLMSmoothing sm) ;
      void modelNumber(size_t N) { m_model_number = N ; }
      void weight(double wt)
	 { m_baseweight = (wt >= 0.0 ? wt : 0.0) ;
	   m_weight = m_baseweight * contextWeight() ; }
      void contextWeight(double wt)
	 { m_contextweight = (wt >= 0.0 ? wt : 0.0) ;
	   m_weight = baseWeight() * m_contextweight ; }
      void setSequences(const List* sequences, const List* fallbacks) ;
      bool convertBWTtoNGM(const char* destfile, size_t rank = 4,
			     size_t precomp_rank = 4) const ;

      // source-cover computation
      bool accumulateCoverBegin() ;
      bool accumulateCover(const List* words) ;
      double accumulatedCoverage() const { return m_sourcecover ; }
      bool accumulateCoverDone(double total_cover, size_t total_models) ;

      // I/O and debugging
      bool dump(CFile& out, const char* modelname,
		  bool run_verbosely = false) const ;
      void dumpConfig(ostream& out) const ;

   protected:
      Vocabulary* m_vocab ;		// mapping from word to ID
      Vocabulary* m_surfvocab ;		// map from surface to adj within class
      Vocabulary* m_stopwords ;		//
      LmNGramsFile* m_ngrams ;		// data for .ngm-format model file
      FrBWTIndex*   m_index ;		// data for .bwt-format model file
      class Ngram*  m_srilm ;		// SRI-style language model
      class Vocab*  m_srivcb ;		// vocabulary for SRI-style model
      NGramModel*   m_sourcemodel ;	// corresponding source-language model
      Symbol*       m_begsent_sym ;
      Symbol*       m_endsent_sym ;
      Symbol*       m_smoothing_name ;
      char*         m_name ;	 	// friendly name (usu. base filename)
      char*         m_ident ;		// identifier, used for section name
      List*         m_sequences ;
      List*         m_fallbacks ;
      double        m_baseweight ;     	// default weight of this model
      double        m_contextweight ;  	// context-dependent weight
      double        m_weight ;	       	// effective weight
      double        m_sourcecover ;    	// accumulated source coverage
      double        m_discount_mass ;
      size_t        m_max_ngram ;
      size_t        m_model_number ;
      LmWordID_t    m_begsent_id ;
      LmWordID_t    m_unknown_id ;
      unsigned      m_featureID ;
      FrLMSmoothing m_smoothing ;
      bool          m_logspace ;
      bool          m_jointprob ;
   } ;
*/

/************************************************************************/
/************************************************************************/

class NGramModelNGM : public NGramModel
   {
   public:
      NGramModelNGM(const char* filename, const char* sourcemodelfile,
		      size_t max_ngram, double weight, double context_weight,
		      double discount, Symbol* smooth_name, const char* ident) ;
      virtual LmWordID_t getUnknownID() const ;
      virtual double perplexity(size_t rank) const ;
      virtual size_t classSize(LmWordID_t ID) const ;
      virtual size_t frequency(const LmWordID_t* IDs, size_t numwords) const ;
      virtual size_t longestMatch(const LmWordID_t* IDs, size_t numwords) const ;
      virtual double rawProbability(size_t numwords, const LmWordID_t* IDs, size_t* max_exist) const ;
      virtual double rawProbability(size_t numwords, NGramHistory* hist,
				    LmWordID_t ID, size_t* max_exist) const ;
      virtual bool isCaseSensitive() const ;
      virtual bool isCharBased() const ;
      virtual bool includesSpaces() const ;
      virtual uint8_t affixSizes() const ;
      virtual uint64_t trainingSize() const ;
      virtual bool dump(CFile& out, const char* modelname, bool verb) const ;

   private:
      void initNGM(const char* filename, size_t max_ngram) ;
   } ;

/************************************************************************/
/************************************************************************/

} // end namespace Fr

#endif /* !__Fr_NGRAMS_H_INCLUDED */

// end of file ngrams.h //

