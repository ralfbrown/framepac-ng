/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-08					*/
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

#include "framepac/stringbuilder.h"
#include "framepac/texttransforms.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

extern std::locale utf8locale ;

/************************************************************************/
/************************************************************************/

char* canonicalize_sentence(const char* sent, std::locale& locale, bool force_uppercase,
			    const char *delim, char const* const* abbrevs)
{
   (void)delim; (void)abbrevs ;

   if (!sent)
      return nullptr ;
   StringBuilder sb ;
   // strip leading and trailing whitespace
   sent = trim_whitespace(sent,locale) ;
   if (force_uppercase)
      {
      uppercase_string(sent,locale) ;
      }
   while (*sent)
      {
      sent = skip_whitespace(sent,locale) ;
//FIXME

      ++sent ;
      }
   return sb.cstring() ;
}

//----------------------------------------------------------------------------

char* canonicalize_sentence(const char* sent, bool force_uppercase,
			    const char *delim, char const* const* abbrevs)
{
   return canonicalize_sentence(sent,utf8locale,force_uppercase,delim,abbrevs) ;
}

} // end namespace Fr

// end of file canonsent.C //
