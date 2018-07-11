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
   //TODO
   return ;
}

//----------------------------------------------------------------------------

WordSplitter::~WordSplitter()
{
   //TODO
   return  ;
}

//----------------------------------------------------------------------------

StringPtr WordSplitter::nextWord()
{
   //TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

StringPtr WordSplitter::delimitedString(char delim)
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
   return true ; //FIXME
}

//----------------------------------------------------------------------------

bool WordSplitter::eof() const
{
   return true ; //FIXME
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
   (void)window_start; (void)currpos; (void)window_end;
   return no_boundary ; //FIXME
}

/************************************************************************/
/*	Methods for class WordSplitterDelimiter				*/
/************************************************************************/

WordSplitter::boundary WordSplitterDelimiter::boundaryType(const char* window_start, const char* currpos,
   const char* window_end) const
{
   (void)window_start; (void)currpos; (void)window_end;
   return no_boundary ; //FIXME
}

/************************************************************************/
/*	Methods for class List						*/
/************************************************************************/

List* List::createWordList(const char* s, char delim)
{
   CharGetterCString getter(s) ;
   WordSplitterDelimiter splitter(getter,delim) ;
   List* words = List::create() ;
   while (!splitter.eof())
      {
      words = words->push(splitter.nextWord()) ;
      }
   return words->reverse() ;
}

//----------------------------------------------------------------------------

} // end of namespace Fr

// end of file wordsplit.C //
