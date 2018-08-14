/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-25					*/
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

#include <stdarg.h>
#include "framepac/bidindex.h"
#include "framepac/file.h"
#include "framepac/message.h"
#include "framepac/mmapfile.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::load(const char* filename, bool allow_mmap)
{
   CInputFile file(filename,CFile::binary) ;
   return file ? load(file,filename,allow_mmap) : false ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::load(CFile& fp, const char* filename, bool allow_mmap)
{
   int version = file_format ;
   if (!fp || !fp.verifySignature(signature,filename,version,min_file_format))
      return false ;
   uint8_t keysize, idxsize ;
   if (!fp.readValue(&keysize) || !fp.readValue(&idxsize))
      return false ;
   if (keysize != sizeof(keyT) || idxsize != sizeof(idxT))
      {
      SystemMessage::error("wrong data type - sizeof() does not match") ;
      return false ;
      }
   if (allow_mmap && loadMapped(filename))
      return true ;

   size_t count ;
   //FIXME: the following line will currently only instantiate for keyT=CString
   if (!fp.readStringArray(m_reverse_index,count))
      return false ;
   m_next_index = count ;
   m_max_index = count ;
   m_common_buffer = count ;
   for (idxT i = 0 ; i < count ; ++i)
      {
      this->add(m_reverse_index[i],i) ;
      }
   return false;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::loadMapped(const char* filename)
{
   if (!filename || !*filename)
      return false;
   MemMappedROFile mm(filename) ;
   if (!mm)
      return false ;
   if (!loadFromMmap(*mm,mm.size()))
      return false ;
   m_readonly = true ;			// can't modify if we're pointing into a memory-mapped file
   return true ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::loadFromMmap(const void* mmap_base, size_t mmap_len)
{
   size_t header_size = CFile::signatureSize(signature) + 2*sizeof(uint8_t) ;
   if (!mmap_base || mmap_len < header_size)
      return false;
   m_external_storage = true ;
   mmap_base = ((char*)mmap_base) + header_size ;
   mmap_len -= header_size ;
   //uint64_t bufsize = ((uint64_t*)mmap_base)[0] ;
   m_max_index = (idxT)(((uint64_t*)mmap_base)[1]) ;
   m_common_buffer = m_max_index ;
   m_next_index = m_max_index ;
   m_reverse_index = new keyT[m_max_index] ;
   if (!m_reverse_index)
      return false ;
   //FIXME: the following will currently only instantiate properly for keyT=CString
   const char* strings = (char*)&(((uint64_t*)mmap_base)[2]) ;
   for (idxT i = 0 ; i < m_max_index ; ++i)
      {
      m_reverse_index[i] = strings ;
      strings = strchr(strings,'\0') + 1 ;
      this->add(m_reverse_index[i],i) ;
      }
   return true ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::save(const char* filename) const
{
   COutputFile file(filename,CFile::binary) ;
   return file ? save(file) : false ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::save(CFile& fp) const
{
   if (!m_reverse_index)
      return false ;			// index must have been finalized prior to trying to save it
   if (!fp || !fp.writeSignature(signature,file_format))
      return false ;
   uint8_t keysize = sizeof(keyT) ;
   uint8_t idxsize = sizeof(idxT) ;
   if (!fp.writeValue(keysize) || !fp.writeValue(idxsize))
      return false ;
   //FIXME: the following line will currently only instantiate for keyT=CString
   if (!fp.writeStringArray(m_reverse_index,m_next_index))
      return false ;
   return true;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
void BidirIndex<keyT,idxT>::clearReverseElement(keyT*)
{
   // default action is to do nothing, as deleting the reverse-index array will invoke dtors and clean up;
   //   but for keyT=CString, we'll need to override and explicitly release storage
   return ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
void BidirIndex<keyT,idxT>::releaseCommonBuffer(keyT*)
{
   // default action is to do nothing, as deleting the reverse-index array will invoke dtors and clean up;
   //   but for keyT=CString, we'll need to override and explicitly release storage
   return ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
void BidirIndex<keyT,idxT>::clearReverseIndex(keyT* index, idxT common, idxT total)
{
   for (size_t i = common ; i < total ; ++i)
      {
      clearReverseElement(&index[i]) ;
      }
   if (common > 0 && !m_external_storage)
      {
      // delete the underlying buffer that the first N elements of m_reverse_index all point at
      releaseCommonBuffer(index) ;
      }
   delete[] index ;
   return ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
void BidirIndex<keyT,idxT>::clear()
{
   clearReverseIndex(m_reverse_index,m_common_buffer,m_max_index) ;
   m_reverse_index = nullptr ;
   m_max_index = 0 ;
   m_next_index = 0 ;
   super::clear() ;
   m_readonly = false ;
   return ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
idxT BidirIndex<keyT,idxT>::addKey(keyT key)
{
   idxT old_index ;
   if (lookup(key,&old_index))
      return old_index ;
   idxT index = m_next_index++ ;
   if (this->add(key,index))
      {
      // somebody added the key between the time we looked it up and the time
      //   we actually tried to add it, so get its value from the other addition
      // also try to restore m_next_index to avoid gaps in the reverse index later
      m_next_index.compare_exchange_weak(index,index-1) ;
      return lookup(key) ;
      }
   return index ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
void BidirIndex<keyT,idxT>::addKeySilent(keyT key)
{
   if (contains(key))
      return ;
   idxT index = m_next_index++ ;
   if (this->add(key,index))
      {
      // somebody added the key between the time we looked it up and the time
      //   we actually tried to add it
      // try to restore m_next_index to avoid gaps in the reverse index later
      m_next_index.compare_exchange_weak(index,index-1) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
static bool make_reverse_entry(keyT key, idxT index, va_list args)
{
   keyT *key_array = va_arg(args,keyT*) ;
   key_array[index] = key ;
   return true ;			// continue iterating
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::finalize()
{
   m_max_index = m_next_index.load() ;
   delete[] m_reverse_index ;
   m_reverse_index = new keyT[m_max_index] ;
   if (m_reverse_index)
      {
      // in case there are any gaps in the index values, zero out the array first
      for (size_t i = 0 ; i < m_max_index ; i++)
	 {
	 m_reverse_index = nullptr ;
	 }
      // iterate through the hash table, storing each key at the appropriate location
      //   in the m_reverse_index array
      return this->iterate(make_reverse_entry,m_reverse_index) ;
      }
   return false ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
size_t BidirIndex<keyT,idxT>::cStringLength_(const Object* obj, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   (void)obj; (void)wrap_at; (void)indent; (void)wrapped_indent;
//TODO
   return 0 ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
char* BidirIndex<keyT,idxT>::toCstring_(const Object* obj, char* buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)wrap_at; (void)indent; (void)wrapped_indent ;
//TODO
   return nullptr ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
size_t BidirIndex<keyT,idxT>::jsonStringLength_(const Object* obj, bool wrap, size_t indent)
{
   (void)obj ; (void)wrap; (void)indent ; //TODO
   return 0 ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::toJSONString_(const Object* obj, char* buffer, size_t buflen,
			 bool /*wrap*/, size_t indent)
{
   (void)obj; (void)buffer; (void)buflen; (void)indent ;
//TODO
   return false ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
size_t BidirIndex<keyT,idxT>::hashValue_(const Object* obj)
{
   return Object::hashValue_(obj) ; //FIXME
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::equal_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2)
      return true ;

   //TODO
   return false ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
int BidirIndex<keyT,idxT>::compare_(const Object* obj1, const Object* obj2)
{
   if (obj1 == obj2)
      return 0 ;

   //TODO
   return +1 ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
int BidirIndex<keyT,idxT>::lessThan_(const Object* obj1, const Object* obj2)
{
   return compare_(obj1,obj2) < 0 ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file bidindex.cc //
