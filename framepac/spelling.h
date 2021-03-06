/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-12					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018,2019 Carnegie Mellon University		*/
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

#ifndef __Fr_SPELLING_H_INCLUDED
#define __Fr_SPELLING_H_INCLUDED

#include "framepac/hashtable.h"
#include "framepac/texttransforms.h"
#include "framepac/trie.h"

namespace Fr
{

//----------------------------------------------------------------------------

class CognateCorrespondence
   {
   public:
      CognateCorrespondence(const char* src, const char* trg, uint8_t sc)
	 {
	    m_srclen = 0 ;
	    m_trglen = 0 ;
	    init(src,trg,sc) ;
	 }
      ~CognateCorrespondence() { freeStrings() ; }

      void init(const char* src, const char* trg, uint8_t sc)
	 {
	    freeStrings() ;
	    if (!src) src = "" ;
	    if (!trg) trg = "" ;
	    m_score = sc ;
	    m_srclen = strlen(src) ;
	    m_trglen = strlen(trg) ;
	    if (m_srclen < sizeof(m_source))
	       strcpy((char*)&m_source,src) ;
	    else
	       m_source = dup_string(src).move() ;
	    if (m_trglen < sizeof(m_target))
	       strcpy((char*)&m_target,trg) ;
	    else
	       m_target = dup_string(trg).move() ;
	    return ;
	 }
      void freeStrings()
	 {
	    if (m_srclen >= sizeof(m_source))
	       delete[] m_source ;
	    if (m_trglen >= sizeof(m_target))
	       delete[] m_target ;
	 }

      const char* source() const { return m_srclen < sizeof(m_source) ? (char*)&m_source : m_source ; }
      const char* target() const { return m_trglen < sizeof(m_target) ? (char*)&m_target : m_target ; }
   protected:
      uint8_t m_score ;
      uint8_t m_srclen ;
      uint8_t m_trglen ;
      char*   m_source ;
      char*   m_target ;
   } ;

//----------------------------------------------------------------------------

class CognateAlignment
   {
   public:
      CognateAlignment() : m_srclen(0), m_trglen(0) {}

      void init(size_t srclen, size_t trglen) { m_srclen = (uint16_t)srclen ; m_trglen = (uint16_t)trglen ; }

      size_t sourceLength() const { return m_srclen ; }
      size_t targetLength() const { return m_trglen ; }

   protected:
      uint16_t m_srclen ;
      uint16_t m_trglen ;
   } ;

//----------------------------------------------------------------------------

struct CogScoreInfo ;  // opaque except inside CognateData implementation

class CognateData
   {
   public:
      CognateData() ;
      CognateData(const char* cognates, size_t fuzzy_match_score = 0) ; // 0 = don't change
      CognateData(const List* cognates, size_t fuzzy_match_score = 0) ; // 0 = don't change
      ~CognateData() ;

      static CognateData* defaultInstance() ;

      void reset() ;
      static CognateData* load(const char* filename, size_t fuzzy_match_score = 0) ; // 0 = don't change
      bool save(const char* filename) ;

      // configuration
      void casefold(bool fold) { m_casefold = fold ; }
      bool casefold() const { return m_casefold ; }
      bool setDoublings(const char* allowable_letters, double score, bool reverse = false) ;
      bool setTranspositions(const char* allowable_letters, double score) ;
      bool setCognateScoring(const char* src, const char* trg, double score) ;
      bool setCognateScoring(const char* cognates) ;
      bool setCognateScoring(const List* cognates) ;
      bool setAdjacentKeys(const char* layout, double nearscore, double farscore = 0.0) ;

      void relativeToShorter(bool rel) { m_rel_to_shorter = rel ; m_rel_to_average = false ; }
      void relativeToAverage(bool rel) { m_rel_to_average = rel ; m_rel_to_shorter = false ; }
      void defaultScoring() { m_rel_to_average = false ; m_rel_to_shorter = false ; }

      bool areCognate(char letter1, char letter2, bool casefold = true) const ;
      size_t cognateLetters(const char* str1, const char* str2, size_t& len1,size_t& len2) const ;
      double score(const char* word1, const char* word2, bool exact_letter_match_only = false,
	 CognateAlignment** align = nullptr) const ;
      double score(const Object* word1, const Object* word2, bool exact_letter_match_only = false,
	 CognateAlignment** align = nullptr) const ;

      // access to internal state
      size_t longestSource() const { return m_mappings ? m_mappings->longestKey() : 1 ; }

   protected:
      float scale_match(size_t srcmatch, size_t trgmatch, size_t srclen, size_t trglen) const ;
      float best_match(const char* word1, size_t len1, size_t index1, const char* word2, size_t len2, size_t index2,
	 size_t& srclen, size_t& trglen) const ;
      float score_single_byte(const char* word1, size_t len1, const char* word2, size_t len2,
	 CogScoreInfo** score_buf, size_t rows) const ;
      float score_general(const char* word1, size_t len1, const char* word2, size_t len2,
	 CogScoreInfo** score_buf, size_t rows) const ;
      double scaledScore(double rawscore, const char* word1, const char* word2) const ;
      
   protected:
      float m_one2one[256][256] ;	// similarity scores for all single-char X->Y mappings
      float m_insertion[256] ;		// similarity scores for a single-char insertion
      float m_deletion[256] ;		// similarity scores for a single-char deletion
      Trie<List*>* m_mappings ;		// info about multi-char X->Y mappings
      bool  m_casefold ;
      bool  m_rel_to_shorter ;
      bool  m_rel_to_average ;
   } ;

//----------------------------------------------------------------------------

class SpellCorrectionData
   {
   public:
      SpellCorrectionData(const SymHashTable* gw = nullptr, const SymCountHashTable* wc = nullptr,
	 ObjHashTable* subst = nullptr, size_t maxsubst = 0) ;
      ~SpellCorrectionData() ;

      // modifiers
      void setWordCounts(const SymCountHashTable* wc)
	 { if (m_wordcounts) const_cast<SymCountHashTable*>(m_wordcounts)->free() ; m_wordcounts = wc ; }
      void setGoodWords(const SymHashTable* gw)
	 { if (m_good_words) const_cast<SymHashTable*>(m_good_words)->free() ; m_good_words = gw ; }

      // accessors
      size_t longestSubstitution() const { return m_maxsubst ; }
      const SymHashTable* goodWords() const { return m_good_words ; }
      const SymCountHashTable* wordCounts() const { return m_wordcounts ; }
      const ObjHashTable* substitutions() const { return m_substitutions ; }

      bool knownPhrase(const char* term, bool allow_norm = true, char split_char = ' ') const ;
      // get a spelling suggestion; 'suggestion' must point at a buffer at least twice as long as 'term' !!
      bool spellingSuggestion(const char* term, char* suggestion, const char* typo_letters = nullptr,
	 bool allow_norm = true) const ;
      // get a list of suggestions, each consisting of a list of the suggestion and the corresponding frequency
      List* spellingSuggestions(const char* term, const char* typo_letters = nullptr, bool allow_norm = true,
	 bool allow_self = true) const ;

   protected:
      const SymHashTable* m_good_words ;
      const SymCountHashTable* m_wordcounts ;
      ObjHashTable* m_substitutions ;
      size_t m_maxsubst ;
   } ;


} // end namespace Fr

#endif /* !__Fr_SPELLING_H_INCLUDED */

// end of file spelling.h //


