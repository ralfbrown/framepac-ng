/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-31					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2018 Carnegie Mellon University			*/
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

#include "framepac/byteorder.h"   // for FrLITTLE_ENDIAN
#include "framepac/charget.h"
#include "framepac/list.h"
#include "framepac/stringbuilder.h"
#include "framepac/words.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class WordSplitter					*/
/************************************************************************/

WordSplitter::WordSplitter(class CharGetter& getter) : m_getter(getter)
{
   fillBuffer() ;
   return ;
}

//----------------------------------------------------------------------------

WordSplitter::WordSplitter(const char* s) : m_getter(*new CharGetterCString(s))
{
   m_getter_local = &m_getter ;
   fillBuffer() ;
   return ;
}

//----------------------------------------------------------------------------

WordSplitter::WordSplitter(const std::string& s) : m_getter(*new CharGetterStdString(s))
{
   m_getter_local = &m_getter ;
   fillBuffer() ;
   return ;
}

//----------------------------------------------------------------------------

WordSplitter::WordSplitter(CFile& file) : m_getter(*new CharGetterFILE(file))
{
   m_getter_local = &m_getter ;
   fillBuffer() ;
   return ;
}

//----------------------------------------------------------------------------

WordSplitter::~WordSplitter()
{
   delete m_getter_local ;
   return ;
}

//----------------------------------------------------------------------------

void WordSplitter::fillBuffer()
{
   m_lookback = 0 ;
   m_lookahead = 0 ;
   while (m_lookahead < sizeof(m_buffer)/2 && !m_getter.eof())
      {
      m_buffer[m_lookahead++] = *m_getter ;
      }
   return ;
}

//----------------------------------------------------------------------------

void WordSplitter::shiftBuffer()
{
   if (m_getter.eof())
      {
      // adjust pointers
      if (!m_lookahead) return ;
      m_lookahead-- ;
      m_lookback++ ;
      return  ;
      }
   if (m_lookahead + m_lookback >= sizeof(m_buffer))
      {
      if (sizeof(m_buffer) == sizeof(size_t))
	 {
	 // optimization for when the buffer is the same size as the machine word: treat it as a word and shift
# ifdef FrLITTLE_ENDIAN
	 *((size_t*)&m_buffer) >>= 8 ;
# else
	 *((size_t*)&m_buffer) <<= 8 ;
# endif /* FrLITTLE_ENDIAN */
	 }
      else
	 memmove(m_buffer,m_buffer+1,sizeof(m_buffer)-1) ;
      m_lookback-- ;
      }
   m_buffer[m_lookahead + m_lookback++] = m_getter.get() ;
   return ;
}

//----------------------------------------------------------------------------

StringPtr WordSplitter::nextWord()
{
   if (m_lookahead == 0) // out of input?
      {
      if (m_new_word && m_lookback > 0)
	 {
	 // we have an unconsumed character, so return it as a single-char word
	 m_new_word = false ;
	 StringBuilder sb ;
	 sb += m_buffer[m_lookback-1] ;
	 return postprocess(sb.string()) ;
	 }
      return nullptr ;
      }
   StringBuilder sb ;
   bool in_word = false ;
   if (m_new_word)
      {
      // did the previous nextWord() find that the then-current char is the start of a word?  If so, we continue
      //   building on it until the next boundary
      m_new_word = false ;
      sb += m_buffer[m_lookback-1] ;
      in_word = true ;
      }
   while (m_lookahead > 0)
      {
      char currchar = m_buffer[m_lookback] ;
      boundary b = boundaryType(m_buffer,m_buffer+m_lookback,m_buffer+m_lookback+m_lookahead) ;
      shiftBuffer() ;
      if (b == word_end || b == word_start_and_end)
	 {
	 if (b == word_start_and_end)
	    m_new_word = true ;	// remember that the current char belongs to the next word
	 break ;
	 }
      if (b == word_start)
	 {
	 if (in_word)			// oops, two consecutive word starts
	    {
	    m_new_word = true ;
	    break ;
	    }
	 in_word = true ;
	 }
      if (in_word)
	 sb += currchar ;
      }
   return postprocess(sb.string()) ;
}

//----------------------------------------------------------------------------

List* WordSplitter::allWords()
{
   ListBuilder lb ;
   while (!eof())
      {
      StringPtr word = nextWord() ;
      if (word)
	 lb += word.move() ;
      }
   return lb.move() ;
}

//----------------------------------------------------------------------------

StringPtr WordSplitter::delimitedWords(char delim)
{
   StringBuilder sb ;
   bool in_word = false ;
   while (m_lookahead > 0)
      {
      char currchar = m_buffer[m_lookback] ;
      boundary b = boundaryType(m_buffer,m_buffer+m_lookback,m_buffer+m_lookback+m_lookahead) ;
      shiftBuffer() ;
      if (b == word_start || b == word_start_and_end)
	 {
	 if (sb.size() > 0)
	    sb += delim ;
	 in_word = true ;
	 }
      if (b == word_end)
	 in_word = false ;
      if (in_word)
	 sb += currchar ;
      }
   return sb.string() ;
}

//----------------------------------------------------------------------------

WordSplitter::operator bool () const
{
   return m_getter && !m_getter.eof();
}

//----------------------------------------------------------------------------

bool WordSplitter::eof() const
{
   return m_lookahead == 0 && !m_new_word ;
}

//----------------------------------------------------------------------------

WordSplitter::boundary WordSplitter::boundaryType(const char* window_start, const char* currpos,
   const char* window_end) const
{
   (void)window_start; (void)currpos; (void)window_end;
   return no_boundary ; //FIXME
}

/************************************************************************/
/*	Methods for class WordSplitterWhitespace			*/
/************************************************************************/

WordSplitter::boundary WordSplitterWhitespace::boundaryType(const char* window_start, const char* currpos,
   const char* window_end) const
{
   if (currpos == window_start)
      return (currpos == window_end || isspace(currpos[0])) ? no_boundary : word_start ;
   if (currpos == window_end)
      return (isspace(currpos[-1])) ? no_boundary : word_end ;
   if (isspace(currpos[-1]) && !isspace(currpos[0]))
      return word_start ;
   if (!isspace(currpos[-1]) && isspace(currpos[0]))
      return word_end ;
   return no_boundary ;
}

/************************************************************************/
/*	Methods for class WordSplitterDelimiter				*/
/************************************************************************/

WordSplitter::boundary WordSplitterDelimiter::boundaryType(const char* window_start, const char* currpos,
   const char* window_end) const
{
   char currchar = currpos[0] ;
   bool is_delim = (currchar == m_delim) ;
   if (currpos == window_start)
      {
      // if the first char is the delimiter, we had an empty word at the start of the input
      return is_delim ? word_end : word_start ;
      }
   if (currpos == window_end)
      return is_delim ? no_boundary : word_end ;
   char prevchar = currpos[-1] ;
   bool was_delim = (prevchar == m_delim) ;
   if (!is_delim)
      return was_delim ? word_start : no_boundary ;
   return (prevchar == m_delim) ? empty_word : word_end ;
}

/************************************************************************/
/*	Methods for class WordSplitterDelimiter				*/
/************************************************************************/

WordSplitter::boundary WordSplitterCSV::boundaryType(const char* window_start, const char* currpos,
   const char* window_end) const
{
   if (currpos == window_start)
      return (*currpos == m_delim) ? word_start_and_end : word_start ;
   if (currpos == window_end)
      return (*currpos == m_delim) ? no_boundary : word_end ;
   // is this a quoted item?
   if (!m_quote && (*currpos == '"' || *currpos == '\'') && currpos[-1] == m_delim)
      m_quote = *currpos ;
   // ignore the delimiter character if inside a quoted string; if we see a matching quote, turn off the in-string flag
   if (m_quote)
      {
      if (*currpos == m_delim)
	 return (currpos[-1] == m_quote) ? word_end : no_boundary ;
      else if (*currpos == m_quote)
	 m_quote = '\0' ;
      }
   if (*currpos == m_delim)
      return (currpos[-1] == m_delim) ? empty_word : word_end ;
   if (currpos[-1] == m_delim)
      return word_start ;
   return no_boundary ;
}

//----------------------------------------------------------------------------

String* WordSplitterCSV::postprocess(String* word)
{
   if (!word || !m_strip)
      return word ;
   const char* s = word->stringValue() ;
   size_t len = word->size() ;
   if (len > 2 && (*s == '"' || *s == '\''))
      {
      String* stripped = String::create(s+1,len-2) ;
      word->free() ;
      word = stripped ;
      }
   return word ;
}

//----------------------------------------------------------------------------

} // end of namespace Fr

// end of file wordsplit.C //
