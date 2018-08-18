/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-19					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#include "framepac/file.h"
#include "framepac/message.h"
#include "framepac/mmapfile.h"
#include "framepac/sufarray.h"

namespace Fr {

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

class SuffixArrayHeader
   {
   public:
      uint64_t m_numids ;	// number of tokens in the corpus
      uint64_t m_types ;	// number of types (distinct IDs)
      uint64_t m_sentinel ;	// ID for end-of-data sentinel
      uint64_t m_newline ;	// ID for newline
      uint64_t m_last_linenum ; // dividing point between token IDs and line-number records
      uint64_t m_index ;	// offset of array of indices
      uint64_t m_ids { 0 } ;	// offset of array of IDs
      uint64_t m_freq { 0 } ;	// offset of array of ID frequencies
      uint64_t m_pad[8] { 0 } ; // padding for future extensions
   } ;

/************************************************************************/
/*	Methods for template class SuffixArray				*/
/************************************************************************/

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::load(const char* filename, bool allow_mmap, const IdT* using_ids)
{
   CInputFile file(filename,CFile::binary) ;
   return file ? load(file,filename,allow_mmap,using_ids) : false ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::load(CFile& fp, const char* filename, bool allow_mmap, const IdT* using_ids)
{
   if (!fp)
      return false ;
   off_t base_offset = fp.tell() ;
   int version = file_format ;
   if (!fp.verifySignature(signature,filename,version,min_file_format))
      return false ;
   uint8_t idsize, idxsize ;
   if (!fp.readValue(&idsize) || !fp.readValue(&idxsize))
      return false ;
   if (idsize != sizeof(IdT) || idxsize != sizeof(IdxT))
      {
      SystemMessage::error("wrong data type - sizeof() does not match") ;
      return false ;
      }
   if (allow_mmap && loadMapped(filename,base_offset,using_ids))
      return true ;
   SuffixArrayHeader header ;
   if (!fp.readValue(&header))
      return false ;
   m_numids = IdxT(header.m_numids) ;
   m_types = IdT(header.m_types) ;
   m_sentinel = IdT(header.m_sentinel) ;
   m_newline = IdT(header.m_newline) ;
   m_last_linenum = IdxT(header.m_last_linenum) ;
   bool success = true ;
   if (header.m_index)
      {
      success &= fp.readVarsAt(header.m_index + base_offset,&m_index,m_numids) ;
      }
   if (success && header.m_freq)
      {
      success &= fp.readVarsAt(header.m_freq + base_offset,&m_freq,m_types) ;
      }
   if (using_ids)
      {
      m_ids = const_cast<IdT*>(using_ids) ;
      m_external_ids = true ;
      }
   else if (header.m_ids)
      {
      success &= fp.readVarsAt(header.m_ids + base_offset,&m_ids,m_numids) ;
      }
   if (!success)
      {
      delete[] m_index ;
      m_index = nullptr  ;
      delete[] m_freq ;
      m_freq = nullptr ;
      if (!m_external_ids)
	 delete[] m_ids ;
      m_ids = nullptr ;
      m_numids = 0 ;
      m_types = 0 ;
      }
   return success ;
}

//----------------------------------------------------------------------------

template <class IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::loadMapped(const char* filename, off_t base_offset, const IdT* using_ids)
{
   if (!filename || !*filename)
      return false;
   MemMappedROFile mm(filename,base_offset) ;
   if (!mm)
      return false ;
   m_mmap = std::move(mm) ;
   return loadFromMmap(*m_mmap,m_mmap.size(),using_ids) ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::loadFromMmap(const char* mmap_base, size_t mmap_len, const IdT* using_ids)
{
   size_t header_size = CFile::signatureSize(signature) + 2*sizeof(uint8_t) ;
   if (!mmap_base || mmap_len < header_size + sizeof(SuffixArrayHeader))
      return false;
   m_readonly = true ;
   m_external_ids = false ;
   const SuffixArrayHeader* header = reinterpret_cast<const SuffixArrayHeader*>(mmap_base + header_size) ;
   m_numids = IdxT(header->m_numids) ;
   m_types = IdT(header->m_types) ;
   m_sentinel = IdT(header->m_sentinel) ;
   m_newline = IdT(header->m_newline) ;
   m_last_linenum = IdxT(header->m_last_linenum) ;
   if (header->m_index)
      {
      m_index = const_cast<IdxT*>(reinterpret_cast<const IdxT*>(mmap_base + header->m_index)) ;
      }
   if (header->m_freq)
      {
      m_freq = const_cast<IdxT*>(reinterpret_cast<const IdxT*>(mmap_base + header->m_freq)) ;
      }
   if (using_ids)
      {
      m_ids = const_cast<IdT*>(using_ids) ;
      m_external_ids = true ;
      }
   else if (header->m_ids)
      {
      m_ids = const_cast<IdT*>(reinterpret_cast<const IdT*>(mmap_base + header->m_ids)) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::save(CFile& fp, bool include_ids) const
{
   if (!fp)
      return false ;
   off_t base_offset = fp.tell() ;
   if (!fp.writeSignature(signature,file_format))
      return false ;
   uint8_t idsize = sizeof(IdT) ;
   uint8_t idxsize = sizeof(IdxT) ;
   if (!fp.writeValue(idsize) || !fp.writeValue(idxsize))
      return false ;
   off_t header_offset = fp.tell() ;
   SuffixArrayHeader header ;
   header.m_numids = m_numids ;
   header.m_types = m_types ;
   header.m_ids = 0 ;
   header.m_sentinel = m_sentinel ;
   header.m_newline = m_newline ;
   header.m_last_linenum = m_last_linenum;
   if (!fp.writeValue(header))
      return false ;
   if (m_index)
      {
      header.m_index = fp.tell() - base_offset ;
      if (!fp.writeValues(m_index,m_numids))
	 return false ;
      }
   if (m_freq)
      {
      header.m_freq = fp.tell() - base_offset ;
      if (!fp.writeValues(m_freq,m_types))
	 return false ;
      }
   if (include_ids && m_ids)
      {
      header.m_ids = fp.tell() - base_offset ;
      if (!fp.writeValues(m_ids,m_numids))
	 return false ;
      }
   // now that we've written all the other data, we have a complete header, so return to the start of the file
   //   and update the header
   off_t lastpos = fp.tell() ;
   fp.seek(header_offset) ;
   bool success = true ;
   if (!fp.writeValue(header))
      success = false ;
   fp.flush() ;
   fp.seek(lastpos) ;
   return success ;
}

//----------------------------------------------------------------------------


} // end of namespace Fr

// end of file sufarray_file.cc //
