/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::load(const char* filename, bool allow_mmap)
{
   if (allow_mmap && loadMapped(filename))
      return true ;
   CInputFile file(filename,CFile::binary) ;
   return file ? load(file) : false ;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::load(CFile& file)
{
   if (!file || file.eof())
      return false ;
   //TODO: check file signature

   size_t count ;
   //FIXME: the following line will currently only instantiate for keyT=CString
   if (!file.readStringArray(m_reverse_index,count))
      return false ;
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
   //TODO
   m_readonly = true ;			// can't modify if we're pointing into a memory-mapped file
   return false;
}

//----------------------------------------------------------------------------

template <class keyT, typename idxT>
bool BidirIndex<keyT,idxT>::loadFromMmap(void* mmap_base, size_t mmap_len)
{
   if (!mmap_base || mmap_len == 0)
      return false;
   //TODO
   return false;
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
bool BidirIndex<keyT,idxT>::save(CFile& file) const
{
   if (!file)
      return false ;
   //TODO
   return false;
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
   if (common > 0)
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
   delete [] m_reverse_index ;
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

} // end namespace Fr

// end of file bidindex.cc //
