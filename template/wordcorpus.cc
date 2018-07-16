/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-14					*/
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
WordCorpusT<IdT,IdxT>::WordCorpusT()
   : m_wordmap(),
     m_wordbuf(),
     m_fwdindex(),
     m_revindex()
{
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::WordCorpusT(const char *filename, bool readonly)
   : m_wordmap(),
     m_wordbuf(),
     m_fwdindex(),
     m_revindex()
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
     m_revindex()
{
   (void)fp; (void)readonly ;

   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::~WordCorpusT()
{
   discardAttributes() ;
   discardContextEquivs() ;
   discardText() ;
   //TODO
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::isCorpusFile(const char* filename)
{
   (void)filename ;

   return false ;//FIXME
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
size_t WordCorpusT<IdT,IdxT>::loadAttribute(const char* filename, unsigned attr_bit, bool add_words)
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
   delete[] m_attributes ;
   m_attributes = nullptr ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::discardContextEquivs()
{
   return false ;//FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::findID(const char* word) const
{
   if (!word || !*word)
      return ErrorID ;
   CString key(word) ;
   IdT id ;
   return m_wordmap.findKey(key,&id) ? id : ErrorID ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::findOrAddID(const char* word)
{
   CString key(word) ;
   return m_wordmap.addKey(key) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::addWord(const char* word)
{
   IdT id = findOrAddID(word) ;
   m_wordbuf += id ;
   return id ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::addWord(IdT word)
{
   //FIXME?
   m_wordbuf += word ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::addNewline()
{
   return addWord(newlineID()) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::rareWordThreshold(IdxT thresh,  const char* token)
{
   m_rare_thresh = thresh ;
   if (m_rare == (IdT)~0)
      {
      m_rare = findOrAddID(token) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::getID(IdxT N) const
{
   (void)N;
   return ErrorID ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::getContextID(IdxT N) const
{
   (void)N;
   return ErrorID ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::getContextID(const char* word) const
{
   (void)word;
   return ErrorID ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getFreq(IdT N) const
{
   (void)N;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getFreq(const char* word) const
{
   (void)word;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::getWord(IdT N) const
{
   (void)N;
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::getNormalizedWord(IdT N) const
{
   (void)N;
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::newlineWord() const
{
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::rareWord() const
{
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::getWordForLoc(IdxT N) const
{
   (void)N;
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::setAttributes(IdT word, uint8_t mask) const
{
   size_t cap = vocabSize() ;
   if (cap > m_attributes_alloc)
      {
      uint8_t* new_attr = new uint8_t[cap] { 0 } ;
      if (new_attr)
	 {
	 if (m_attributes_alloc)
	    memcpy(new_attr,m_attributes,m_attributes_alloc*sizeof(uint8_t)) ;
	 m_attributes = new_attr ;
	 m_attributes_alloc = cap ;
	 }
      }
   if (word < m_attributes_alloc)
      {
      m_attributes[word] |= mask ;
      }
   return ;
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

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::createIndex(bool bidirectional)
{
   (void)bidirectional;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::lookup(const IdT *key, unsigned keylen, IdxT& first_match, IdxT& last_match) const
{
   (void)key; (void)keylen; (void)first_match; (void)last_match;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateForward(unsigned minlen, unsigned maxlen, size_t minfreq, SAEnumFunc* fn, void* user)
{
   (void)minlen; (void)maxlen; (void)minfreq; (void)fn; (void)user;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateForwardParallel(unsigned minlen, unsigned maxlen, size_t minfreq,
   SAEnumFunc* fn, void* user)
{
   (void)minlen; (void)maxlen; (void)minfreq; (void)fn; (void)user;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateReverse(unsigned minlen, unsigned maxlen, size_t minfreq, SAEnumFunc* fn, void* user)
{
   (void)minlen; (void)maxlen; (void)minfreq; (void)fn; (void)user;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateReverseParallel(unsigned minlen, unsigned maxlen, size_t minfreq,
   SAEnumFunc* fn, void* user)
{
   (void)minlen; (void)maxlen; (void)minfreq; (void)fn; (void)user;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getForwardPosition(IdxT N) const
{
   (void)N;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getReversePosition(IdxT N) const
{
   (void)N;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::setContextSizes(unsigned lcontext, unsigned rcontext)
{
   m_left_context = lcontext ;
   m_right_context = rcontext ;
   m_total_context = lcontext + rcontext + (lcontext+rcontext==0) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::positionalID(IdT word, int offset) const
{
   return (word * m_total_context) + offset + m_left_context - (offset > 0) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
int WordCorpusT<IdT,IdxT>::offsetOfPosition(IdT pos) const
{
   int offset = (int)(pos % m_total_context) - m_left_context ;
   return offset + (offset >= 0) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::wordForPositionalID(IdT pos) const
{
   return pos / m_total_context ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::printVocab(CFile&) const
{
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::printWords(CFile&, size_t max) const
{
   (void)max;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::printSuffixes(CFile&, bool by_ID, size_t max) const
{
   (void)by_ID; (void)max;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::reserveIDs(IdxT count, IdT* newline)
{
   (void)count; (void)newline;
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::setID(IdxT N, IdT id)
{
   (void)N; (void)id;
   return ; //FIXME
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file wordcorpus.cc //
