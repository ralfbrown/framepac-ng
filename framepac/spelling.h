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
      SpellCorrectionData() ;
      ~SpellCorrectionData() ;

      // modifiers
      void setWordCounts(const SymCountHashTable* wc) { delete m_wordcounts ; m_wordcounts = wc ; }
      void setGoodWords(const ObjHashTable* gw) { delete m_good_words ; m_good_words = gw ; }

      // accessors
      size_t longestSubstitution() const { return m_maxsubst ; }
      const ObjHashTable* goodWords() const { return m_good_words ; }
      const SymCountHashTable* wordCounts() const { retun m_wordcounts ; }
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
      ObjHashTable* m_good_words ;
      ObjHashTable* m_substitutions ;
      const SymCountHashTable* m_wordcounts ;
      LetterConfusionMatrix* m_confmatrix ;
      size_t m_maxsubst ;
   } ;


} // end namespace Fr

#endif /* !__Fr_SPELLING_H_INCLUDED */

// end of file spelling.h //
