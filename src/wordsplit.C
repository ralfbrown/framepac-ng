/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-11					*/
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

#include "framepac/charget.h"
#include "framepac/stringbuilder.h"
#include "framepac/words.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class WordSplitter					*/
/************************************************************************/

WordSplitter::WordSplitter(class CharGetter& getter) : m_getter(getter)
{
   return ;
}

//----------------------------------------------------------------------------

StringPtr WordSplitter::nextWord()
{
   //TODO
   StringPtr word { nullptr } ;
   return postprocess(word) ;
}

//----------------------------------------------------------------------------

StringPtr WordSplitter::delimitedWords(char delim)
{
   StringBuilder sb ;
   bool first = true ;
   while (!eof())
      {
      StringPtr word = nextWord() ;
      if (!first) sb += delim ;
      sb.append(word->c_str()) ;
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
   return !m_getter.eof();
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
   if (currpos == window_start)
      return (*currpos == m_delim) ? word_start_and_end : word_start ;
   if (currpos == window_end)
      return (*currpos == m_delim) ? no_boundary : word_end ;
   //TODO
   return no_boundary ;
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
   if (!m_quote && (*currpos == '"' || *currpos == '\''))
      m_quote = *currpos ;
   // ignore the delimiter character if inside a quoted string
   if (m_quote && *currpos == m_delim)
      return no_boundary ;
   //TODO
   return no_boundary ;
}

//----------------------------------------------------------------------------

StringPtr WordSplitterCSV::postprocess(StringPtr& word)
{
   if (m_strip)
      {
      const char* s = word->stringValue() ;
      size_t len = word->size() ;
      if (len > 2 && (*s == '"' || *s == '\''))
	 {
	 return String::create(s+1,len-2) ;
	 }
      }
   return &word ;
}

//----------------------------------------------------------------------------

} // end of namespace Fr

// end of file wordsplit.C //
