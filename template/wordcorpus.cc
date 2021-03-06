/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-18					*/
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

#include "framepac/mmapfile.h"
#include "framepac/wordcorpus.h"
#include "framepac/words.h"
#include "framepac/texttransforms.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::WordCorpusT()
   : m_wordbuf(),
     m_fwdindex(),
     m_revindex()
{
   m_wordmap = BiMap::create() ;
   m_contextmap = Map::create() ;
   m_sentinel = findOrAddID("<end_of_data>") ;
   m_newline = findOrAddID("<eol>") ;
   setContextSizes(0,0) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::WordCorpusT(const char *filename, bool readonly)
   : WordCorpusT<IdT,IdxT>()
{
   if (isCorpusFile(filename))
      {
      load(filename) ;
      }
   m_readonly = readonly ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::WordCorpusT(CFile &fp, const char* filename, bool readonly)
   : WordCorpusT<IdT,IdxT>()
{
   load(fp,filename) ;
   m_readonly = readonly ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
WordCorpusT<IdT,IdxT>::~WordCorpusT()
{
   discardAttributes() ;
   discardContextEquivs() ;
   discardText() ;
   freeTermFrequencies() ;
   m_wordmap->free() ;
   m_wordmap = nullptr ;
//   m_contextmap->free() ; //FIXME: memleak without this free(), crash with....
   m_contextmap = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::isCorpusFile(const char* filename)
{
   CInputFile fp(filename,CFile::binary) ;
   int version = file_format ;
   return fp && fp.verifySignature(signature,filename,version,min_file_format,true) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::load(CFile &fp, const char* filename, bool allow_mmap)
{
   if (!fp)
      return false ;
   off_t base_offset = fp.tell() ;
   int version = file_format ;
   if (!fp.verifySignature(signature,filename,version,min_file_format))
      return false ;
   WordCorpusHeader header ;
   if (!fp.readValue(&header))
      return false ;
   // ensure that the writer used the same data sizes
   if (header.m_idsize != sizeof(IdT) || header.m_idxsize != sizeof(IdxT))
      return false ;
   if (version < file_format)
      {
      // this is an older but still supported format and needs some conversion/adjustment
      return false ; //FIXME
      }
   else
      {
      // get the header values
      m_last_linenum = header.m_last_linenum ;
      }
   bool success = true ;
   if (!allow_mmap || !loadMapped(filename,base_offset))
      {
      // we couldn't memory-map the data, so read it into allocated memory
      if (header.m_wordmap)
	 {
	 fp.seek(header.m_wordmap + base_offset) ;
	 success &= m_wordmap->load(fp,filename,false) ;
	 }
      if (success && header.m_wordbuf)
	 {
	 fp.seek(header.m_wordbuf + base_offset) ;
	 success &= m_wordbuf.load(fp,filename) ;
	 }
      if (success && header.m_fwdindex)
	 {
	 fp.seek(header.m_fwdindex + base_offset) ;
	 success &= m_fwdindex.load(fp,filename,m_wordbuf.currentBuffer()) ;
	 }
      if (success && header.m_revindex)
	 {
	 fp.seek(header.m_revindex + base_offset) ;
	 success &= m_revindex.load(fp,filename) ;
	 }
      if (success && header.m_freq)
	 {
	 fp.seek(header.m_freq + base_offset) ;
	 m_freq = new IdxT[header.m_vocabsize] ;
	 success &= fp.readValues(&m_freq,header.m_vocabsize) ;
	 }
      if (success && header.m_attributes)
	 {
	 fp.seek(header.m_attributes + base_offset) ;
	 m_attributes = new uint8_t[header.m_numwords] ;
	 m_attributes_alloc = header.m_numwords ;
	 m_attributes_mapped = false ;
	 success &= fp.readValues(&m_attributes,m_attributes_alloc) ;
	 }
      }
   if (success)
      {
      //TODO?
      }
   return success ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::load(const char *filename, bool allow_mmap)
{
   CInputFile fp(filename) ;
   return load(fp,filename,allow_mmap) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::loadMapped(const char *filename, off_t base_offset)
{
   if (!filename || !*filename)
      return false ;
   MemMappedROFile mm(filename,base_offset) ;
   if (!mm)
      return false ;
   m_mmap = std::move(mm) ;
   return loadFromMmap(*m_mmap,m_mmap.size()) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::loadFromMmap(const char* mmap_base, size_t mmap_len)
{
   size_t header_size = CFile::signatureSize(signature) ;
   if (!mmap_base || mmap_len < header_size)
      return false;
   m_readonly = true ;			// can't modify if we're pointing into a memory-mapped file
   const WordCorpusHeader* header = (WordCorpusHeader*)(mmap_base+header_size) ;
   // ensure that the writer used the same data sizes
   if (header->m_idsize != sizeof(IdT) || header->m_idxsize != sizeof(IdxT))
      return false ;
   m_last_linenum = header->m_last_linenum ;
   m_rare = header->m_rare_ID ;
   m_rare_thresh = header->m_rare_threshold ;
   if (header->m_wordmap)
      {
      m_wordmap->loadFromMmap(mmap_base+header->m_wordmap,mmap_len-header->m_wordmap) ;
      }
   if (header->m_wordbuf)
      {
      m_wordbuf.loadFromMmap(mmap_base+header->m_wordbuf,mmap_len-header->m_wordbuf) ;
      }
   if (header->m_fwdindex)
      {
      m_fwdindex.loadFromMmap(mmap_base+header->m_fwdindex,mmap_len-header->m_fwdindex,m_wordbuf.currentBuffer()) ;
      }
   if (header->m_revindex)
      {
      m_revindex.loadFromMmap(mmap_base+header->m_revindex,mmap_len-header->m_revindex) ;
      }
   if (header->m_freq)
      {
      m_freq = (IdxT*)((char*)mmap_base + header->m_freq) ;
      }
   if (header->m_attributes)
      {
      m_attributes = (uint8_t*)((char*)mmap_base + header->m_attributes) ;
      m_attributes_mapped = true ;
      }
   m_mapped = true ;
   return true  ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::loadContextEquivs(const char* filename, bool force_lowercase)
{
   discardContextEquivs() ;
   CInputFile fp(filename) ;
   if (!fp)
      return false ;
   while (auto line = fp.getTrimmedLine())
      {
      char* lineptr = (char*)line ;
      char *tab = strchr(lineptr,'\t') ;
      if (tab)
	 {
	 *tab++ = '\0' ;
	 if (force_lowercase)
	    {
	    lowercase_string(lineptr) ;
	    }
	 lineptr = remove_quoting(lineptr) ;
	 WordSplitterEnglish splitter(lineptr) ;
	 StringPtr keystr = splitter.delimitedWords() ;
	 const char *key = keystr->stringValue() ;
	 size_t keylen = count_words(key) ;
	 if (keylen > m_max_context)
	    m_max_context = keylen ;
	 tab = remove_quoting(tab) ;
	 ID ctxt = m_wordmap->addKey(key) ;
	 ID id = m_wordmap->addKey(tab) ;
	 this->m_contextmap->add(m_wordmap->getKey(ctxt),id) ;
	 }
      else if (*lineptr)
	 {
	 cerr << "Skipping line without tab separating text and classname: " << lineptr << endl ;
	 }
      }
   return true ; 
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::consolidateContextEquivs()
{
   if (!this->m_contextmap)
      return false ;
   auto vs = this->vocabSize() ;
   this->m_contextequivs = new IdT[vs] ;
   if (!this->m_contextequivs)
      return false ;
   std::fill(*this->m_contextequivs,*this->m_contextequivs+vs,IdT(~0)) ;
   for (auto eq : *this->m_contextmap)
      {
      IdT id ;
      if (m_wordmap->findKey(eq.first,&id) && id < vs)
	 {
	 this->m_contextequivs[id] = eq.second ;
	 }
      }
   // replace numbers which haven't yet been mapped by <number>
   if (m_number != (IdT)~0)
      {
      for (size_t i = 0 ; i < vs ; ++i)
	 {
	 auto id = this->m_contextequivs[i] ;
	 if (id == IdT(~0))
	    {
	    auto key = this->getWord(i) ;
	    if (is_number(key))
	       {
	       this->m_contextequivs[i] = m_number ;
	       }
	    }
	 }
      }
   // replace rare words which haven't yet been mapped by <rare>
   if (rareWordThreshold() > 0 && rareID() && this->m_freq)
      {
      for (size_t i = 0 ; i < vs ; ++i)
	 {
	 if (this->m_contextequivs[i] == (IdT)~0 && this->m_freq[i] < this->m_rare_thresh)
	    this->m_contextequivs[i] = rareID() ;
	 }
      }
   // replace any words which haven't yet been mapped by themselves
   for (size_t i = 0 ; i < vs ; ++i)
      {
      if (this->m_contextequivs[i] == (IdT)~0)
	 this->m_contextequivs[i] = i ;
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
size_t WordCorpusT<IdT,IdxT>::loadAttribute(const char* filename, unsigned attr_bit, bool add_words)
{
   CInputFile fp(filename) ;
   if (!fp)
      return 0 ;
   size_t count = 0 ;
   while (auto line = fp.getTrimmedLine())
      {
      if (line[0] && line[0] != ';' && line[0] != '#')
	 {
	 ID word = add_words ? m_wordmap->addKey(*line) : m_wordmap->getIndex(*line) ;
	 if (word != ErrorID)
	    {
	    setAttribute(word,attr_bit) ;
	    ++count ;
	    }
	 }
      }
   return count ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::save(CFile &fp) const
{
   if (!fp)
      return false ;
   off_t base_offset = fp.tell() ;
   if (!fp.writeSignature(signature,file_format))
      return false ;
   // we don't have all the info needed to write the header yet, so write a placeholder and remember
   //   the file offset so we can return later and update it
   off_t headerpos = fp.tell() ;
   WordCorpusHeader header ;
   header.m_numwords = corpusSize() ;
   header.m_vocabsize = vocabSize() ;
   header.m_last_linenum = m_last_linenum ;
   header.m_rare_ID = m_rare ;
   header.m_rare_threshold = m_rare_thresh ;
   header.m_wordmap = 0 ;
   header.m_wordbuf = 0 ;
   header.m_contextmap = 0 ;
   header.m_fwdindex = 0 ;
   header.m_revindex = 0 ;
   header.m_freq = 0 ;
   header.m_attributes = 0 ;
   memset(header.m_pad,'\0',sizeof(header.m_pad)) ;
   header.m_idsize = sizeof(IdT) ;
   header.m_idxsize = sizeof(IdxT) ;
   header.m_keep_linenumbers = m_keep_linenumbers ;
   memset(header.m_pad2,'\0',sizeof(header.m_pad2)) ;
   if (!fp.writeValue(header))
      return false ;

   bool success = true ;
   header.m_wordmap = fp.tell() - base_offset ;
   success &= m_wordmap->save(fp) ;
   header.m_wordbuf = fp.tell() - base_offset ;
   success &= m_wordbuf.save(fp) ;
   header.m_contextmap = fp.tell() - base_offset ;
   success &= m_contextmap->save(fp) ;
   header.m_fwdindex = fp.tell() - base_offset ;
   success &= m_fwdindex.save(fp,false) ;
   header.m_revindex = fp.tell() - base_offset ;
   success &= m_revindex.save(fp,false) ;
   if (m_freq)
      {
      header.m_freq = fp.tell() - base_offset ;
      fp.writeValues(m_freq,vocabSize()) ;
      }
   if (m_attributes)
      {
      header.m_attributes = fp.tell() - base_offset ;
      fp.writeValues(m_attributes,corpusSize()) ;
      }
   // now that we've written all the other data, we have a complete header, so return to the start of the file
   //   and update the header
   off_t lastpos = fp.tell() ;
   fp.seek(headerpos) ;
   if (!fp.writeValue(header))
      success = false ;
   fp.flush() ;
   fp.seek(lastpos) ;
   return success ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::save(const char *filename) const
{
   COutputFile fp(filename) ;
   return save(fp) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::discardText()
{
   m_wordbuf.clear() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::discardAttributes() const
{
   if (!m_attributes_mapped)
      {
      delete[] m_attributes ;
      }
   m_attributes = nullptr ;
   m_attributes_alloc = 0 ;
   m_attributes_mapped = false ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::discardContextEquivs()
{
   m_contextmap->clear() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::findID(const char* word) const
{
   if (!word || !*word)
      return ErrorID ;
   CString key(word) ;
   IdT id ;
   return m_wordmap->findKey(key,&id) ? id : ErrorID ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::findOrAddID(const char* word)
{
   CString key(word) ;
   return m_wordmap->addKey(key) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::addWord(const char* word)
{
   IdT id = findOrAddID(word) ;
   return addWord(id) ? id : ErrorID ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::addWord(IdT word)
{
   if (m_wordbuf.size() >= m_last_linenum)
      return false ;			// collision with IDs used for recording line numbers
   m_wordbuf += word ;
   freeIndices() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::addNewline()
{
   if (!addWord(m_last_linenum))
      return false ;
   if (m_keep_linenumbers)
      --m_last_linenum ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::setNumberToken(const char* token)
{
   if (!token) token = "<NUMBER>" ;
   m_number = findOrAddID(token) ;
   return m_number ;
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
   return (N < corpusSize()) ? m_wordbuf[N] : ErrorID ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::getContextEquivID(IdxT N) const
{
   IdT id = getID(N) ;
   if (id != ErrorID)
      {
      if (id >= m_last_linenum)
	 return m_newline ;
      else if (m_contextequivs)
	 return m_contextequivs[id] ;
      }
   return id ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::getContextID(IdxT N) const
{
   IdT id = getID(N) ;
   if (id != ErrorID)
      {
      if (id >= m_last_linenum)
	 return m_newline ;
      else if (m_freq)
	 return (m_freq[id] < m_rare_thresh) ? m_rare : id ;
      }
   return id ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdT WordCorpusT<IdT,IdxT>::getContextID(const char* word) const
{
   IdT id = ErrorID ;
   return m_contextmap->lookup(word,&id) ? id : ErrorID ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::computeTermFrequencies()
{
   if (m_freq)
      return;
   size_t num_types = m_wordmap->size() ;
   m_freq = new IdxT[num_types] ;
   std::fill(m_freq,m_freq+num_types,0) ;
   // scan the array of term IDs and accumulate counts
   const IdT* buf = m_wordbuf.currentBuffer() ;
   size_t tokens = m_wordbuf.size() ;
   for (size_t i = 0 ; i < tokens ; ++i)
      {
      IdT id = buf[i] ;
      if (id < num_types)
	 ++m_freq[id] ;
      else if (id >= m_last_linenum)
	 ++m_freq[m_newline] ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::freeTermFrequencies()
{
   if (!m_mapped)
      {
      delete[] m_freq ;
      }
   m_freq = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getFreq(IdT N) const
{
   return (N <= vocabSize() && m_freq) ? m_freq[N] : IdxT(0) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getFreq(const char* word) const
{
   return getFreq(findID(word)) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::getWord(IdT N) const
{
   return m_wordmap->getKey(N).str() ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::getNormalizedWord(IdT N) const
{
   if (N <= vocabSize())
      return getWord(N) ;
   else if (N >= m_last_linenum)
      return newlineWord() ;
   else
      return nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::newlineWord() const
{
   return getWord(m_newline) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::rareWord() const
{
   return m_rare ? getWord(m_rare) : nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
const char* WordCorpusT<IdT,IdxT>::getWordForLoc(IdxT N) const
{
   return (N <= corpusSize()) ? getNormalizedWord(m_wordbuf[N]) : newlineWord() ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::setAttributes(IdT word, uint8_t mask) const
{
   size_t cap = vocabSize() ;
   if (cap > m_attributes_alloc)
      {
      auto new_attr = new uint8_t[cap] ;
      if (new_attr)
	 {
	 if (m_attributes_alloc)
	    memcpy(new_attr,m_attributes,m_attributes_alloc*sizeof(uint8_t)) ;
	 std::fill(new_attr+m_attributes_alloc,new_attr+cap,0) ;
	 discardAttributes() ;
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
void WordCorpusT<IdT,IdxT>::clearAttributes(uint8_t mask) const
{
   if (!m_attributes)
      return ;
   for (size_t i = 0 ; i < vocabSize() ; ++i)
      {
      clearAttributes(i,mask) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::createIndex(bool bidirectional)
{
   bool success = createForwardIndex() ;
   if (success && bidirectional)
      {
      success &= createReverseIndex() ;
      }
   return success ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::lookup(const IdT *key, unsigned keylen, IdxT& first_match, IdxT& last_match) const
{
   return m_fwdindex.lookup(key,keylen,first_match,last_match) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateForward(unsigned minlen, unsigned maxlen,
   const std::function<SAEnumFunc>& fn, const std::function<SAFilterFunc>& filter)
{
   if (!fn)
      return false ;			// can't enumerate without a function to call!
   if (!createForwardIndex())
      return false ;			// didn't have an index and couldn't create it, so fail
   return m_fwdindex.enumerateSegment(Range<IdxT>(getFreq(m_sentinel),m_fwdindex.indexSize()),0,
      Range<IdT>(1,vocabSize()),Range<unsigned>(minlen,maxlen),fn,filter) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateForward(IdxT start, IdxT stop, unsigned minlen, unsigned maxlen,
   const std::function<SAEnumFunc>& fn, const std::function<SAFilterFunc>& filter) const
{
   if (!m_fwdindex)
      return false ;
   return m_fwdindex.enumerate(Range<IdxT>(start,stop),Range<unsigned>(minlen,maxlen),fn,filter) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateForwardParallel(unsigned minlen, unsigned maxlen,
   const std::function<SAEnumFunc>& fn, const std::function<SAFilterFunc>& filter, bool async)
{
   if (!fn)
      return false ;			// can't enumerate without a function to call!
   if (!createForwardIndex())
      return false ;			// didn't have an index and couldn't create it, so fail
   return m_fwdindex.enumerateParallel(Range<unsigned>(minlen,maxlen),fn,filter,async) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::finishForwardParallel() const
{
   m_fwdindex.finishParallel() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateReverse(unsigned minlen, unsigned maxlen,
   const std::function<SAEnumFunc>& fn, const std::function<SAFilterFunc>& filter)
{
   if (!fn)
      return false ;			// can't enumerate without a function to call!
   if (!createReverseIndex())
      return false ;			// didn't have an index and couldn't create it, so fail
   return m_revindex.enumerateSegment(Range<IdxT>(getFreq(m_sentinel),m_revindex.indexSize()),0,
      Range<IdT>(1,vocabSize()),Range<unsigned>(minlen,maxlen),fn,filter) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateReverse(IdxT start, IdxT stop, unsigned minlen, unsigned maxlen,
   const std::function<SAEnumFunc>& fn, const std::function<SAFilterFunc>& filter) const
{
   if (!m_revindex)
      return false ;
   return m_revindex.enumerate(Range<IdxT>(start,stop),Range<unsigned>(minlen,maxlen),fn,filter) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::enumerateReverseParallel(unsigned minlen, unsigned maxlen,
   const std::function<SAEnumFunc>& fn, const std::function<SAFilterFunc>& filter, bool async)
{
   if (!fn)
      return false ;			// can't enumerate without a function to call!
   if (!createReverseIndex())
      return false ;			// didn't have an index and couldn't create it, so fail
   return m_revindex.enumerateParallel(Range<unsigned>(minlen,maxlen),fn,filter,async) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::finishReverseParallel() const
{
   m_revindex.finishParallel() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::createForwardIndex()
{
   if (m_fwdindex)
      return true ;
   if (!corpusSize() || m_wordbuf[corpusSize()-1] != m_sentinel)
      {
      // add the end-of-data sentinel
      addWord(m_sentinel) ;
      }
   m_wordmap->finalize() ;
   computeTermFrequencies(); 
   m_fwdindex.generate(m_wordbuf.currentBuffer(),m_wordbuf.size(), m_wordmap->size(), m_newline, m_freq) ;
   m_fwdindex.setFreqTable(m_freq) ;
   m_fwdindex.setSentinel(m_sentinel) ;
   return m_fwdindex == true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::createReverseIndex()
{
   if (m_revindex)
      return true ;
   if (!corpusSize() || m_wordbuf[corpusSize()-1] != m_sentinel)
      {
      // add the end-of-data sentinel
      addWord(m_sentinel) ;
      }
   m_wordmap->finalize() ;
   computeTermFrequencies(); 
   m_wordbuf.reverse() ;
   m_revindex.generate(m_wordbuf.currentBuffer(),m_wordbuf.size(), m_wordmap->size(), m_newline, m_freq) ;
   //TODO: adjust offsets in index to match un-reversed positions in buffer
   m_wordbuf.reverse() ;
   m_revindex.setFreqTable(m_freq) ;
   m_revindex.setSentinel(m_sentinel) ;
   return m_revindex == true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::freeIndices()
{
   m_fwdindex.clear() ;
   m_revindex.clear() ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getForwardPosition(IdxT N) const
{
   return m_fwdindex.indexAt(N) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::getReversePosition(IdxT N) const
{
   return m_revindex.indexAt(N) ;
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
int WordCorpusT<IdT,IdxT>::offsetOfPosition(IdT pos, unsigned left_context, unsigned total_context)
{
   int offset = (int)(pos % total_context) - left_context ;
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
bool WordCorpusT<IdT,IdxT>::printVocab(CFile& fp) const
{
   if (!fp)
      return false ;
   if (haveTermFrequencies())
      {
      for (size_t i = 0 ; i < vocabSize() ; ++i)
	 {
	 const char* word = getWord(i) ;
	 if (word)
	    {
	    fp.printf("%lu\t%s\n",(unsigned long)getFreq(i),word) ;
	    }
	 else
	    {
	    fp.puts("<null> ==> error!\n") ;
	    }
	 }
      }
   else
      {
      for (size_t i = 0 ; i < vocabSize() ; ++i)
	 {
	 const char* word = getWord(i) ;
	 fp.printf("%s\n",word?word:"(null)") ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::printWords(CFile& fp, size_t max) const
{
   if (!fp)
      return false ;
   if (max > corpusSize())
      max = corpusSize() ;
   bool line_start = true ;
   for (size_t i = 0 ; i < max ; ++i)
      {
      const char* word = getWordForLoc(i) ;
      if (!line_start)
	 fp.putc(' ') ;
      if (!word)
	 {
	 fp.puts("<eol>\n") ;
	 line_start = true ;
	 }
      else
	 {
	 fp.puts(word) ;
	 line_start = (strcmp(word,"<eol>") == 0) ;
	 if (line_start)
	    fp.putc('\n') ;
	 }
      }
   // unless the last thing we printed was a newline, terminate the current line
   if (!line_start)
      fp.putc('\n') ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool WordCorpusT<IdT,IdxT>::printSuffixes(CFile& fp, bool by_ID, size_t max) const
{
   if (!fp)
      return false ;
   if (!m_fwdindex)
      return false ;
   if (max == 0)
      max = 2 ;
   for (size_t i = 0 ; i < corpusSize() ; ++i)
      {
      IdxT offset = m_fwdindex.indexAt(i) ;
      if (offset > corpusSize())
	 {
	 fp.puts("-1\t<eol>\n") ;
	 continue ;
	 }
      fp.printf("%lu\t",(unsigned long)offset) ;
      size_t maxhere = max ;
      if (offset + maxhere > corpusSize())
	 maxhere = corpusSize() - offset ;
      for (size_t j = 0 ; j < maxhere ; ++j)
	 {
	 if (j > 0)
	    fp.putc(' ') ;
	 if (by_ID)
	    {
	    IdxT id = m_fwdindex.idAt(offset+j) ;
	    if (id >= m_last_linenum)
	       fp.puts("<eol>") ;
	    else
	       fp.printf("%lu",(unsigned long)id) ;
	    }
	 else
	    {
	    const char* word = getWordForLoc(offset+j) ;
	    fp.puts(word ? word : "<eol>") ;
	    }
	 }
      if (offset + maxhere < corpusSize())
	 fp.puts(" ...") ;
      fp.putc('\n') ;
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
IdxT WordCorpusT<IdT,IdxT>::reserveIDs(IdxT count, IdT* newline)
{
   if (newline)
      *newline = (--m_last_linenum)+1 ;
   return m_wordbuf.reserveElements(count) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
void WordCorpusT<IdT,IdxT>::setID(IdxT N, IdT id)
{
   m_wordbuf.setElement(N,id) ;
   return ;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file wordcorpus.cc //
