/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.02, last edit 2017-07-27					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

namespace Fr
{

//----------------------------------------------------------------------------

class CognateAlignment
   {
   public:
      CognateAlignment() : m_srclen(0), m_trglen(0) {}

      size_t sourceLength() const { return m_srclen ; }
      size_t targetLength() const { return m_trglen ; }

   protected:
      size_t m_srclen ;
      size_t m_trglen ;
   } ;

//----------------------------------------------------------------------------

class CognateData
   {
   public:
      CognateData() ;
      CognateData(const char* cognates, size_t fuzzy_match_score = 0) ; // 0 = don't change
      CognateData(const List* cognates, size_t fuzzy_match_score = 0) ; // 0 = don't change
      ~CognateData() ;

      static CognateData* defaultInstance() ;

      static CognateData* load(const char* filename, size_t fuzzy_match_score = 0) ; // 0 = don't change
      bool save(const char* filename) ;

      // configuration
      void casefold(bool fold) { m_casefold = fold ; }
      bool casefold() const { return m_casefold ; }
      bool setCognateScoring(const char* cognates) ;
      bool setOne2ManyScore(char source, const char* targets) ;
      bool setMany2OneScore(const char* sources, char target) ;
      
      void relativeToShorter(bool rel) { m_rel_to_shorter = rel ; m_rel_to_average = false ; }
      void relativeToAverage(bool rel) { m_rel_to_average = rel ; m_rel_to_shorter = false ; }
      void defaultScoring() { m_rel_to_average = false ; m_rel_to_shorter = false ; }

      bool areCognate(char letter1, char letter2, bool casefold = true) const ;
      size_t cognateLetters(const char* str1, const char* str2, size_t& len1,size_t& len2) const ;
      double score(const char* word1, const char* word2, bool exact_letter_match_only = false,
	 CognateAlignment** align = nullptr) const ;
      double score(const Object* word1, const Object* word2, bool exact_letter_match_only = false,
	 CognateAlignment** align = nullptr) const ;

   protected:
      uint8_t m_one2one[256][256] ;	// similarity scores for all single-char X->Y mappings
      char* m_one2many[256] ;		// pointers to X->YY mappings for each X
      char* m_many2one[256] ;		// pointers to XX->Y mappings for each Y
      char** m_many2many ;		// info about XX->YY mappings
      size_t m_many2many_count ;	// how many XX->YY mappings we have
      size_t m_longest_source ;		// longest many-to-X mapping, determines how much buffer we need
      char* m_fwdbuffer ;
      char* m_revbuffer ;
      bool  m_casefold ;
      bool  m_rel_to_shorter ;
      bool  m_rel_to_average ;
   } ;

//----------------------------------------------------------------------------

class LetterConfusionMatrix
   {
   public:
      LetterConfusionMatrix() ;
      ~LetterConfusionMatrix() ;

      static LetterConfusionMatrix* load(const char* filename) ;
      bool save(const char* filename) const ;

      double score(char letter1, char letter2) const ;
      double score(const char* seq1,const char* seq2) const ;
   protected:

   } ;

//----------------------------------------------------------------------------

class SpellCorrectionData
   {
   public:
      SpellCorrectionData(const ObjHashTable* gw = nullptr, const SymCountHashTable* wc = nullptr,
	 LetterConfusionMatrix* cm = nullptr, ObjHashTable* subst = nullptr, size_t maxsubst = 0) ;
      ~SpellCorrectionData() ;

      // modifiers
      void setWordCounts(const SymCountHashTable* wc) { delete m_wordcounts ; m_wordcounts = wc ; }
      void setGoodWords(const ObjHashTable* gw) { delete m_good_words ; m_good_words = gw ; }

      // accessors
      size_t longestSubstitution() const { return m_maxsubst ; }
      const ObjHashTable* goodWords() const { return m_good_words ; }
      const SymCountHashTable* wordCounts() const { return m_wordcounts ; }
      const ObjHashTable* substitutions() const { return m_substitutions ; }
      const LetterConfusionMatrix* confusionMatrix() const { return m_confmatrix ; }

      bool knownPhrase(const char* term, bool allow_norm = true, char split_char = ' ') const ;
      // get a spelling suggestion; 'suggestion' must point at a buffer at least twice as long as 'term' !!
      bool spellingSuggestion(const char* term, char* suggestion, const char* typo_letters = nullptr,
	 bool allow_norm = true) const ;
      // get a list of suggestions, each consisting of a list of the suggestion and the corresponding frequency
      List* spellingSuggestions(const char* term, const char* typo_letters = nullptr, bool allow_norm = true,
	 bool allow_self = true) const ;

   protected:
      const ObjHashTable* m_good_words ;
      ObjHashTable* m_substitutions ;
      const SymCountHashTable* m_wordcounts ;
      LetterConfusionMatrix* m_confmatrix ;
      size_t m_maxsubst ;
   } ;


} // end namespace Fr

#endif /* !__Fr_SPELLING_H_INCLUDED */

// end of file spelling.h //
