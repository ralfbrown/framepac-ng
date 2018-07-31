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

#include "framepac/words.h"

namespace Fr
{

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
   // special handling for decimal numbers: a period or comma is not a boundary if there are digits on both sides
   if ((currchar == '.' || currchar == ',') && isdigit(prevchar) && isdigit(nextchar))
      {
      return no_boundary ;
      }
   // a period between to alpha characters is considered to be part of an abbreviation with periods, and thus
   //   not a boundary
   if (currchar == '.' && isalpha(prevchar))
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
   // a digit followed by an 'e' followed by an optional sign and more digits is part of a floating-point number
   if ((currchar == 'e' || currchar == 'E') && isdigit(prevchar))
      {
      if (window_end > currpos+1 && isdigit(currpos[2]))
	 return no_boundary ;
      if (window_end > currpos+2 && (currpos[2] == '+' || currpos[2] == '-') && isdigit(currpos[3]))
	 return no_boundary ;
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
      else if (isalpha(prevchar) && isalpha(nextchar))
	 return no_boundary ;
      // in all other cases, split before the dash
      else
	 return word_start_and_end ;
      }
   if (currchar == '\'')
      {
      //TODO: check if contraction or doubled single quote
      }
   if (currchar == '`')
      {
      //TODO: check if doubled back-quote
      }
#if 0
   if (PnP_mode && (currchar == ':' || currchar == '/'))
      {
      // treat colons and slashes as normal non-delimiter characters inside a glossary variable or marker

      }
#endif
   // at last, the default case: check whether the character is a delimiter, and split appropriately
   if (m_delim[(unsigned)currchar])
      {
//TODO   
      }
   return no_boundary ; //FIXME
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file wordsplit_eng.C //
