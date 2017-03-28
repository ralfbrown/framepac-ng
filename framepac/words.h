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

#ifndef _Fr_WORDS_H_INCLUDED
#define _Fr_WORDS_H_INCLUDED

#include "framepac/bidindex.h"
#include "framepac/file.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

typedef uint32_t WordID ;

typedef BidirIndex<Symbol,WordID> BidirWordIndex ;
extern template class BidirIndex<Symbol,WordID> ;

/************************************************************************/
/************************************************************************/

class WordSplitter
   {
   public:
      WordSplitter(class CharGetter&) ;
      WordSplitter(const WordSplitter&) = default ;
      ~WordSplitter() ;
      WordSplitter& operator= (const WordSplitter&) = default ;

      StringPtr nextWord() ;
      operator bool () const ;  // did we successfully init?
      bool eof() const ;
   public:
      enum boundary { no_boundary = 0, word_start = 1, word_end = 2 } ;
   protected:
      CharGetter &m_getter ;
      unsigned    m_maxlookahead ;
      unsigned    m_maxlookback ;

   protected:
      virtual boundary boundaryType(const char* window_start, const char* currpos,
				    const char* window_end) const ;
   } ;

//----------------------------------------------------------------------------

class WordSplitterWhitespace : public WordSplitter
   {
   public:
      WordSplitterWhitespace(class CharGetter& getter) : WordSplitter(getter) {}
      WordSplitterWhitespace(const WordSplitterWhitespace&) = default ;
      virtual ~WordSplitterWhitespace() {}
      WordSplitterWhitespace& operator= (const WordSplitterWhitespace&) = default ;

   protected:
      virtual boundary boundaryType(const char* window_start, const char* currpos,
				    const char* window_end) const ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename ID>
class WordCorpus
   {
   public:
      WordCorpus() ;
      WordCorpus(const WordCorpus&) = delete ;
      WordCorpus(const char* filename, bool readonly) ;
      WordCorpus(CFile& fp, bool readonly) ;
      ~WordCorpus() ;
      void operator= (const WordCorpus&) = delete ;

      const char* word(size_t id) { return m_wordstrings[id] ; }
      ID& operator [] (size_t N) { return m_wordIDs[N] ; }
      const ID& operator [] (size_t N) const { return m_wordIDs[N] ; }

      bool save(CFile& fp) ;
      bool save(const char* filename) ;
   private:
      char **m_wordstrings ;
      ID    *m_wordIDs ;
      size_t m_numwords ;
      size_t m_numIDs ;
      size_t m_allocwords ;
      size_t m_allocIDs ;
   } ;

//----------------------------------------------------------------------------

extern template class WordCorpus<WordID> ;

//----------------------------------------------------------------------------

} ; // end namespace Fr

#endif /* !_Fr_WORDS_H_INCLUDED */

// end of file words.h //

