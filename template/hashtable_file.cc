/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-27					*/
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
#include "framepac/hashtable.h"
#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

class HashTableHeader
   {
   public:
      uint64_t    m_size ;		// number of items in the hash table
      uint64_t    m_capacity ;		// capacity of table (number of hash buckets)
      uint64_t    m_pad[8] { 0 } ;	// padding for future extensions
   } ;

/************************************************************************/
/*	Methods for class HashTable					*/
/************************************************************************/

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::load(CFile& fp, const char* filename)
{
   int version = file_format ;
   if (!fp || !fp.verifySignature(signature,filename,version,min_file_format))
      return false ;
   uint8_t keysize, valsize ;
   if (!fp.readValue(&keysize) || !fp.readValue(&valsize))
      return false ;
   if (keysize != sizeof(KeyT) || valsize != sizeof(ValT))
      {
      SystemMessage::error("wrong data type - sizeof() does not match") ;
      return false ;
      }
   HashTableHeader header ;
   if (!fp.readValue(&header))
      return false ;
   this->reserve(header.m_capacity) ;
   for (size_t i = 0 ; i < header.m_size ; ++i)
      {
      // read an item from the file and add it to the hash table
      KeyT key ;
      ValT val  ;
      if (!fp.readValue(&key))
	 return false ;
      if (sizeof(ValT) > 0 && !fp.readValue(&val))
	 return false ;
      if (add(key,val))
	 {
	 // duplicate entry!
	 return false ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::load(const char* mmap_base, size_t mmap_len)
{
   size_t signature_size = CFile::signatureSize(signature) + 2*sizeof(uint8_t) ;
   if (!mmap_base || mmap_len < signature_size)
      return false ;
   if (*((uint8_t*)(mmap_base + signature_size-2)) != sizeof(KeyT) ||
      *((uint8_t*)(mmap_base + signature_size-2)) != sizeof(ValT))
      {
      SystemMessage::error("wrong data type - sizeof() does not match") ;
      return false ;
      }
   const HashTableHeader* header = reinterpret_cast<const HashTableHeader*>(mmap_base + signature_size) ;
   this->reserve(header->m_capacity) ;
   const char* curr =  mmap_base + signature_size + sizeof(HashTableHeader) ;
   for (size_t i = 0 ; i < header->m_size ; ++i)
      {
      // get an item and add it to the hash table
      KeyT key = *((KeyT*)curr) ;
      curr += sizeof(KeyT) ;
      ValT val ;
      if (sizeof(ValT) > 0)
	 {
	 val = *((ValT*)curr) ;
	 curr += sizeof(ValT) ;
	 }
      if (add(key,val))
	 {
	 // duplicate item!
	 return false ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename ValT>
bool HashTable<KeyT,ValT>::save(CFile& fp) const
{
   if (!fp || !fp.writeSignature(signature,file_format))
      return false ;
   uint8_t keysize = sizeof(KeyT) ;
   uint8_t valsize = sizeof(ValT) ;
   if (!fp.writeValue(keysize) || !fp.writeValue(valsize))
      return false ;
   HashTableHeader header ;
   header.m_size = currentSize() ;
   header.m_capacity = maxSize() ;
   if (!fp.writeValue(header))
      return false ;
   for (auto entry : *this)
      {
      if (!fp.writeValue(entry.first))
	 return false ;
      if (sizeof(ValT) > 0 && !fp.writeValue(entry.second))
	 return false ;
      }
   return true ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file hashtable_file.cc //
