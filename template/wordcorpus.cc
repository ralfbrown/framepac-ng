/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-06					*/
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

#include "framepac/wordcorpus.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::WordCorpusT(const char *filename, bool readonly)
   : m_wordmap(),
     m_wordbuf(),
     m_fwdindex(),
     m_revindex(),
     m_attributes(nullptr),
     m_numwords(0)
{
   (void)filename; (void)readonly ;

   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::WordCorpusT(CFile &fp, bool readonly)
   : m_wordmap(),
     m_wordbuf(),
     m_fwdindex(),
     m_revindex(),
     m_attributes(nullptr),
     m_numwords(0)
{
   (void)fp; (void)readonly ;

   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::~WordCorpusT()
{
   //TODO
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::load(CFile &fp)
{
   (void)fp ;

   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::load(const char *filename)
{
   CInputFile fp(filename) ;
   return fp ? load(fp) : false ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::loadContextEquivs(const char* filename, bool force_lowercase)
{
   (void)filename; (void)force_lowercase;
//FIXME
   return false ; 
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
size_t WordCorpusT<IdT,IdxT>::loadAttributes(const char* filename, unsigned attr_bit, bool add_words)
{
   (void)filename; (void)attr_bit; (void)add_words;
//FIXME
   return 0 ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::save(CFile &fp) const
{
   (void)fp ;

   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::save(const char *filename) const
{
   COutputFile fp(filename) ;
   return fp ? save(fp) : false ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::discardText()
{
   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::discardAttributes()
{
   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::discardContextEquivs()
{
   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::corpusSize() const
{
   return 0 ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::vocabSize() const
{
   return 0 ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::findID(const char* word) const
{
   return 0 ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::findOrAddID(const char* word)
{
   return 0 ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::addWord(const char* word)
{
   return 0 ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::addWord(IdT word)
{
   return 0 ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::addNewline()
{
   return 0 ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::setAttributeIf(unsigned attr_bit, WordCorpusT<IdT,IdxT>::AttrCheckFunc *fn)
{
   bool set_any = false ;
   for (size_t i = 0 ; i < vocabSize() ; ++i)
      {
      if (fn(getWord(i)))
	 {
	 set_any = true ;
	 setAttribute(i,attr_bit) ;
	 }
      }
   return set_any ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file wordcorpus.cc //
