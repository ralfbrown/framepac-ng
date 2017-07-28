/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.02, last edit 2017-07-27					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

#include "framepac/spelling.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class CognateData					*/
/************************************************************************/

CognateData::CognateData()
{

   return ;
}

//----------------------------------------------------------------------------

CognateData::CognateData(const char* cognates, size_t fuzzy_match_score)
{
   (void)cognates; (void)fuzzy_match_score; //FIXME

   return ;
}

//----------------------------------------------------------------------------

CognateData::CognateData(const List* cognates, size_t fuzzy_match_score)
{
   (void)cognates; (void)fuzzy_match_score; //FIXME

   return ;
}

//----------------------------------------------------------------------------

CognateData::~CognateData()
{

   return ;
}

//----------------------------------------------------------------------------

CognateData* CognateData::defaultInstance()
{
   static CognateData instance ;

   return &instance ;
}

//----------------------------------------------------------------------------

CognateData* CognateData::load(const char* /*filename*/, size_t /*fuzzy_match_score*/)
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

bool CognateData::save(const char* /*filename*/)
{

   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool CognateData::areCognate(char /*letter1*/, char /*letter2*/, bool /*casefold*/) const
{

   return false ; //FIXME
}

//----------------------------------------------------------------------------

size_t CognateData::cognateLetters(const char* str1, const char* str2, size_t& len1, size_t& len2) const
{
   (void)str1; (void)str2; (void)len1; (void)len2;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

double CognateData::score(const char* word1, const char* word2, bool exact_letter_match_only,
   CognateAlignment** align) const
{
   (void)word1; (void)word2; (void)exact_letter_match_only; (void)align;

   return 0.0 ; //FIXME
}

//----------------------------------------------------------------------------

double CognateData::score(const Object* word1, const Object* word2, bool exact_letter_match_only,
   CognateAlignment** align) const
{
   return (word1 && word2) ? score(word1->stringValue(),word2->stringValue(),exact_letter_match_only,align) : 0.0 ;
}

//----------------------------------------------------------------------------



} // end namespace Fr


// end of file cognate.C //
