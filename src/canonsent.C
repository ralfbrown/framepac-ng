/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-07					*/
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

#include "framepac/texttransforms.h"
#include "framepac/words.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

extern std::locale utf8locale ;

/************************************************************************/
/************************************************************************/

CharPtr canonicalize_sentence(const char* orig_sent, std::locale& locale, bool force_uppercase,
			      const char *delim, char const* const* abbrevs)
{
   (void)abbrevs ;

   if (!orig_sent)
      return nullptr ;
   CharPtr sent { dup_string(orig_sent) } ;
   if (force_uppercase)
      {
      uppercase_string((char*)sent,locale) ;
      }
   WordSplitterEnglish splitter(sent,delim) ;
   StringPtr words = splitter.delimitedWords() ;
   return dup_string(words->c_str()) ;
}

//----------------------------------------------------------------------------

CharPtr canonicalize_sentence(const char* sent, bool force_uppercase,
			      const char *delim, char const* const* abbrevs)
{
   return canonicalize_sentence(sent,utf8locale,force_uppercase,delim,abbrevs) ;
}

} // end namespace Fr

// end of file canonsent.C //
