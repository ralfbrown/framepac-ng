/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-31					*/
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

#include <stdarg.h>
#include "framepac/bidindex.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <class keyT, typename idxT>
idxT BidirIndex<keyT,idxT>::addKey(keyT key)
{
   idxT old_index ;
   if (lookup(key,&old_index))
      return old_index ;
   idxT index = m_next_index++ ;
   if (add(key,index))
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
   if (add(key,index))
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
   m_reverse_index = new keyT*[m_max_index] ;
   if (m_reverse_index)
      {
      // in case there are any gaps in the index values, zero out the array first
      for (size_t i = 0 ; i < m_max_index ; i++)
	 {
	 m_reverse_index = 0 ;
	 }
      // iterate through the hash table, storing each key at the appropriate location
      //   in the m_reverse_index array
      return iterate(make_reverse_entry,m_reverse_index) ;
      }
   return false ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file bidindex.cc //
