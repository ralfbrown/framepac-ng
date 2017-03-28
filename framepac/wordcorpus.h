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

#ifndef _Fr_WORDCORPUS_H_INCLUDED
#define _Fr_WORDCORPUS_H_INCLUDED

#include "framepac/bidindex.h"
#include "framepac/builder.h"
#include "framepac/byteorder.h"
#include "framepac/cstring.h"
#include "framepac/file.h"
#include "framepac/sufarray.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

template <typename IdT, typename IdxT>
class WordCorpusT
   {
   public: // types
      typedef IdT ID ;
      typedef IdxT Index ;
      typedef bool AttrCheckFunc(const char *word) ;
   public:
      WordCorpusT() ;
      WordCorpusT(const WordCorpusT&) = delete ;
      WordCorpusT(const char* filename, bool readonly) ;
      WordCorpusT(CFile& fp, bool readonly) ;
      void operator= (const WordCorpusT&) = delete ;
      ~WordCorpusT() ;

      bool load(const char* filename) ;
      bool load(CFile&) ;
      bool loadContextEquivs(const char* filename, bool force_lowercase = true) ;
      size_t loadAttribute(const char* filename, unsigned attr_bit, bool add_words = false) ;
      bool save(const char* filename) const ;
      bool save(CFile&) const ;
      bool discardText() ;
      bool discardAttributes() ;
      bool discardContextEquivs() ;

      IdxT corpusSize() const ; //FIXME
      IdT vocabSize() const ; //FIXME

      IdT findID(const char* word) const ;
      IdT findOrAddID(const char* word) ;
      IdT addWord(const char* word) ;
      bool addWord(IdT word) ;
      bool addNewline() ;

      bool setAttributeIf(unsigned attr_bit, AttrCheckFunc* fn) ;
      void setAttributes(IdT word, uint8_t mask) const ;
      void setAttribute(IdT word, unsigned bit) const { setAttributes(word,1<<bit) ; }
      void clearAttributes(uint8_t mask) const ;
      void clearAttributes(IdT word, uint8_t mask) const
	 { if (m_attributes && word < m_numwords) m_attributes[word] &= ~mask ; } //FIXME
      void clearAttribute(IdT word, unsigned bit) const { clearAttributes(word,1<<bit) ; }
      void clearAttribute(unsigned bit) const { clearAttribute(1<<bit) ; }

      IdT getID(IdxT N) const ; //FIXME
      IdT getContextID(IdxT N) const ;
      IdT getContextID(const char* word) const ;
      IdT newlineID() const ;
      IdT rareID() const ;

      IdxT getFreq(IdT N) ; //FIXME
      IdxT getFreq(const char* word) ;

      const char* getWord(IdT N) const ;//FIXME
      const char* getNormalizedWord(IdT N) const ;
      const char* newlineWord() const ;
      const char* rareWord() const ;
      const char* getWordForLoc(IdxT N) const ;

      IdxT getForwardPosition(IdxT N) const ;
      IdxT getReversePosition(IdxT N) const ;

      uint8_t attributes(IdT word) const ;
      bool hasAttribute(IdT word, unsigned bit) const ;

      bool printVocab(CFile&) const ;
      bool printWords(CFile&, size_t max = ~0) const ;
      bool printSuffixes(CFile&, bool by_ID = false, size_t max = 6) const ;

      // support functions for parallelized conversion of text into WordCorpusT
      IdxT reserveIDs(IdxT count, IdT *newline) ;
      void setID(IdxT N, IdT id) ;

   protected:
      BidirIndex<CString,IdT> m_wordmap ;
      BufferBuilder<IdT,1>    m_wordbuf ;
      SuffixArray<IdT,IdxT>   m_fwdindex ;
      SuffixArray<IdT,IdxT>   m_revindex ;
//      IdT*                    m_wordIDs ;
      uint8_t*                m_attributes ;
      size_t		      m_numwords ; //FIXME

   } ;

//----------------------------------------------------------------------------

extern template class WordCorpusT<uint32_t,uint32_t> ;
typedef WordCorpusT<uint32_t,uint32_t> WordCorpus ;

extern template class WordCorpusT<uint32_t,UInt40> ;
typedef WordCorpusT<uint32_t,UInt40> WordCorpusXL ;

//----------------------------------------------------------------------------

// end of namespace Fr
} ;

#endif /* !_Fr_WORDCORPUS_H_INCLUDED */

// end of file wordcorpus.h //

