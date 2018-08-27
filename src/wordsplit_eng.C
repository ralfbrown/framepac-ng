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

#include "framepac/words.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

const char WordSplitterEnglish::s_default_delim[256] =
   {
   // delimiters for plain ASCII and supersets such as UTF-8
      1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,	   // ^@ to ^O
      1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,	   // ^P to ^_
      1, 1, 1, 1, 1, 1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 1,	   // SP to /
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 1, 1, 0, 1, 0, 1,	   //  0 to ?
      1, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   //  @ to O
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 1, 1, 1, 1, 0,	   //  P to _
      1, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   //  ` to o
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 1, 1, 1, 1, 0,	   //  p to DEL
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0x80 to 0x8F
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0x90 to 0x9F
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xA0 to 0xAF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xB0 to 0xBF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xC0 to 0xCF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xD0 to 0xDF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xE0 to 0xEF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xF0 to 0xFF
#if 0 // delimiters for Latin-1
      0, 0, 1, 0, 1, 1, 0, 0,	0, 1, 0, 0, 0, 0, 0, 0,	   // 0x80 to 0x8F
      0, 1, 1, 1, 1, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0x90 to 0x9F
      1, 1, 0, 1, 0, 1, 0, 1,	0, 0, 0, 1, 0, 0, 0, 0,	   // 0xA0 to 0xAF
      1, 0, 0, 0, 0, 1, 1, 1,	1, 0, 0, 1, 0, 0, 0, 1,	   // 0xB0 to 0xBF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xC0 to 0xCF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xD0 to 0xDF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xE0 to 0xEF
      0, 0, 0, 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0, 0, 0,	   // 0xF0 to 0xFF
#endif
   } ;

/************************************************************************/
/*	Methods for class WordSplitterEnglish				*/
/************************************************************************/

static bool known_abbreviation(const char* start, const char* pastend)
{
   if (pastend == start)
      return false ;

   //TODO
   return false ;
}

//----------------------------------------------------------------------------

WordSplitter::boundary WordSplitterEnglish::boundaryType(const char* window_start, const char* currpos,
   const char* window_end) const
{
   // get the previous and current characters.  For the purposes of the boundary checks that follow,
   //   the character before the first character of actual text and the character following the last
   //   character of actual text are assumed to be whitespace.
   char prevchar = (currpos > window_start) ? currpos[-1] : ' ' ;
   char currchar = (window_end > currpos) ? currpos[0] : ' ' ;
   // going from non-whitespace to whitespace always signals that the previous character was the last
   //   character of a token, so we signal end of word
   if (isspace(currchar) && !isspace(prevchar))
      return word_end ;
   // going from whitespace to non-whitespace always signals that the current character is the first
   //  character of a token, so we signal start of word
   if (isspace(prevchar) && !isspace(currchar))
      return word_start ;
   // if both previous and current character are whitespace, we were not in a token and are still not in
   //   a token, so there is no boundary here
   if (isspace(prevchar) && isspace(currchar))
      return no_boundary ;
   // at this point, neither previous nor current character are whitespace, so we are either in the middle
   //   of a single token (no boundary) or at a point where a string of non-whitespace characters needs to
   //   be split into multiple tokens (both word end and word start)
   char nextchar = (window_end > currpos+1) ? currpos[1] : ' ' ;
   if (isdigit(prevchar))
      {
      // special handling for decimal numbers: a period or comma is not a boundary if there are digits on both sides
      if ((currchar == '.' || currchar == ',') && isdigit(nextchar))
	 {
	 return no_boundary ;
	 }
      // a digit followed by an 'e' followed by an optional sign and more digits is part of a floating-point number
      if ((currchar == 'e' || currchar == 'E'))
	 {
	 if (isdigit(nextchar))
	    return no_boundary ;
	 if (window_end > currpos+1 && (nextchar == '+' || nextchar == '-') && isdigit(currpos[2]))
	    return no_boundary ;
	 }
      }
   bool prev_alpha = isalpha(prevchar) ;
   // a period between two alpha characters is considered to be part of an abbreviation with periods, and thus
   //   not a boundary
   if (currchar == '.' && prev_alpha)
      {
      if (isalnum(nextchar))
	 return no_boundary ;
      // check for something like the last period in "N.A.T.O. "
      if (isspace(nextchar) && currpos > window_start+2 && currpos[-2] == '.' && isalpha(currpos[-3]))
	 return no_boundary ;
      // check for the trailing period in a known abbreviation without embedded periods, e.g. "Mrs."
      if (isspace(nextchar) && known_abbreviation(window_start,currpos))
	 return no_boundary ;
      // we've exhausted the cases were a period should stay attached to the preceding letter, so signal
      //  a split
      return word_start_and_end ;
      }
   //  multiple consecutive periods stay together
   if (currchar == '.' && prevchar == '.')
      {
      return no_boundary ;
      }
   if (isalnum(currchar))
      {
      if (prevchar == '.')
	 {
	 if (keepingEmbeddedPeriods())
	    return no_boundary ;
	 // embedded period as in N.A.T.O. or foo.com or 2.3
	 if (currpos > window_start + 1 && isalnum(currpos[-2]))
	    return no_boundary ;
	 else
	    return word_start_and_end ;
	 }
      // check if hyphenated word or contraction; don't break after the hyphen/squote
      if (prevchar == '-' || prevchar == '\'')
	 {
	 if (currpos > window_start + 1 && isalpha(currpos[-2]))
	    return no_boundary ;
	 }
      }
   // a plus or minus followed by digits stays attached to the digits
   if ((prevchar == '+' || prevchar == '-') && isdigit(currchar))
      {
      // unless it's part of a number range (123-456)
      if (currpos > window_start+1 && isdigit(currpos[-2]))
	 return word_start_and_end ;
      return no_boundary ;
      }
   if (currchar == '-')
      {
      // multiple consecutive dashes stay together
      if (prevchar == '-')
	 return no_boundary ;
      // as do hyphenated words
      else if (prev_alpha && isalnum(nextchar))
	 return no_boundary ;
      // in all other cases, split before the dash
      else
	 return word_start_and_end ;
      }
   else if (prevchar == '-')
      {
      if (isalnum(currchar) && currpos > window_start + 1 && currpos[-2] == '-')
	 {
	 // break between double hyphen and a following letter or digit
	 return word_start_and_end ;
	 }
      }
   if (currchar == '\'')
      {
      if (prevchar == '\'')
	 return no_boundary ;   // repeated single quote
      if (prev_alpha && isalpha(nextchar))
	 return no_boundary ;	// contraction or ohina (Hawai'ian)
      }
   if (currchar == '`')
      {
      if (prevchar == '`')
	 return no_boundary ;   // repeated back-quote
      }
   if (m_tag_mode)
      {
      // Remember whether we are inside a tag.  This works because we are called exactly once per character,
      //   in strict left-to-right order.
      if (currchar == '<')
	 m_in_tag = true ;
      if (currchar == '>')
	 m_in_tag = false ;
      }
   else
      {
      if (prevchar == '<' || prevchar == '>')
	 {
	 return word_start_and_end ;
	 }
      else if (currchar == '<' || currchar == '>')
	 {
	 return word_start_and_end ;
	 }
      }
   if (m_in_tag && (currchar == ':' || currchar == '/'))
      {
      // treat colons and slashes as normal non-delimiter characters inside a glossary variable or marker
      return no_boundary ;
      }
   // at last, the default case: check whether the character is a delimiter, and split appropriately
   if (m_delim[(unsigned char)currchar])
      {
      // we know that the previous character isn't whitespace, so the delimiter causes a word break
      return word_start_and_end ;
      }
   else if (prevchar != '.' && m_delim[(unsigned char)prevchar])
      {
      // if the previous character was a delimiter and the current one isn't, we need to split following
      //   the previous char
      return word_start_and_end ;
      }
   return no_boundary ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file wordsplit_eng.C //
