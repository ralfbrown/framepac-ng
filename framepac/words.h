/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#ifndef _Fr_WORDS_H_INCLUDED
#define _Fr_WORDS_H_INCLUDED

#include "framepac/bidindex.h"
#include "framepac/file.h"
#include "framepac/string.h"

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
      virtual ~WordSplitter() {}
      WordSplitter& operator= (const WordSplitter&) = default ;

      // return the next word in the stream of characters provided by the CharGetter
      StringPtr nextWord() ;
      // return a single string containing all of the words, separated by the given character
      StringPtr delimitedWords(char delim = ' ') ;

      operator bool () const ;  // did we successfully init?
      bool eof() const ;
   public:
      enum boundary { no_boundary = 0, word_start = 1, word_end = 2, word_start_and_end = 3 } ;
   protected:
      CharGetter &m_getter ;
      char        m_buffer[16] ;		// maxlookahead+maxlookback <= 16 !
      unsigned    m_maxlookahead { 1 } ;
      unsigned    m_maxlookback { 1 } ;

   protected:
      virtual boundary boundaryType(const char* window_start, const char* currpos,
				    const char* window_end) const ;
      virtual StringPtr postprocess(StringPtr& word) { return &word ; }
   } ;

//----------------------------------------------------------------------------

class WordSplitterWhitespace : public WordSplitter
   {
   public:
      typedef WordSplitter super ;
   public:
      WordSplitterWhitespace(class CharGetter& getter) : super(getter) {}
      WordSplitterWhitespace(const WordSplitterWhitespace&) = default ;
      virtual ~WordSplitterWhitespace() {}
      WordSplitterWhitespace& operator= (const WordSplitterWhitespace&) = default ;

   protected:
      virtual boundary boundaryType(const char* window_start, const char* currpos,
				    const char* window_end) const ;
   } ;

//----------------------------------------------------------------------------

class WordSplitterDelimiter : public WordSplitter
   {
   public:
      typedef WordSplitter super ;
   public:
      WordSplitterDelimiter(class CharGetter& getter, char delim = ' ') : super(getter), m_delim(delim) {}
      WordSplitterDelimiter(const WordSplitterDelimiter&) = default ;
      virtual ~WordSplitterDelimiter() {}
      WordSplitterDelimiter& operator= (const WordSplitterDelimiter&) = default ;

   protected:
      virtual boundary boundaryType(const char* window_start, const char* currpos,
				    const char* window_end) const ;
   protected:
      char m_delim ;
   } ;

//----------------------------------------------------------------------------

class WordSplitterCSV : public WordSplitter
   {
   public:
      typedef WordSplitter super ;
   public:
      WordSplitterCSV(class CharGetter& getter, char delim = ',', bool strip_quotes = true)
	 : super(getter), m_delim(delim), m_strip(strip_quotes) {}
      WordSplitterCSV(const WordSplitterCSV&) = default ;
      virtual ~WordSplitterCSV() {}
      WordSplitterCSV& operator= (const WordSplitterCSV&) = default ;

   protected:
      virtual boundary boundaryType(const char* window_start, const char* currpos,
				    const char* window_end) const ;
      virtual StringPtr postprocess(StringPtr& word) ;

   protected:
      char m_delim ;
      char m_strip ;
      mutable char m_quote { '\0' } ;
   } ;

//----------------------------------------------------------------------------

class WordSplitterEnglish : public WordSplitter
   {
   public:
      typedef WordSplitter super ;
   public:
      WordSplitterEnglish(class CharGetter& getter, bool force_lower = false)
	 : super(getter), m_force_lower(force_lower) {}
      WordSplitterEnglish(const WordSplitterEnglish&) = default ;
      virtual ~WordSplitterEnglish() {}
      WordSplitterEnglish& operator= (const WordSplitterEnglish&) = default ;

      bool forceLowercase() const { return m_force_lower ; }

   protected:
      virtual boundary boundaryType(const char* window_start, const char* currpos,
				    const char* window_end) const ;
   protected:
      bool m_force_lower ;
   } ;

/************************************************************************/
/************************************************************************/

} ; // end namespace Fr

#endif /* !_Fr_WORDS_H_INCLUDED */

// end of file words.h //

