/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-12					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018,2019 Carnegie Mellon University		*/
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
/*	Methods for class SpellCorrectionData				*/
/************************************************************************/

SpellCorrectionData::SpellCorrectionData(const SymHashTable* gw, const SymCountHashTable* wc,
   ObjHashTable* subst, size_t maxsubst)
   : m_good_words(gw), m_wordcounts(wc)
{
   m_substitutions = subst ;
   m_maxsubst = maxsubst ;
   return ;
}

//----------------------------------------------------------------------------

SpellCorrectionData::~SpellCorrectionData()
{
   m_good_words = nullptr ;
   m_wordcounts = nullptr  ;
   m_substitutions = nullptr ;
   m_maxsubst = 0 ;
   return ;
}

//----------------------------------------------------------------------------

bool SpellCorrectionData::knownPhrase(const char* term, bool allow_norm, char split_char) const
{
   (void)term; (void)allow_norm; (void)split_char;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool SpellCorrectionData::spellingSuggestion(const char* term, char* suggestion, const char* typo_letters,
   bool allow_norm) const
{
   (void)term; (void)suggestion; (void)typo_letters; (void)allow_norm;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

List* SpellCorrectionData::spellingSuggestions(const char* term, const char* typo_letters, bool allow_norm,
   bool allow_self) const
{
   (void)term; (void)typo_letters; (void)allow_norm; (void)allow_self;

   return List::emptyList() ;
}


} // end namespace Fr


// end of file spelling.C //
