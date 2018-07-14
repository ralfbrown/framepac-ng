/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-18					*/
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

#ifndef _Fr_BIDINDEX_H_INCLUDED
#define _Fr_BIDINDEX_H_INCLUDED

#include "framepac/hashtable.h"

namespace Fr
{

// The naive approach to having a bidirectional mapping between keys
// and integral indices is to have an array for the index->key
// direction and a hashmap from key->index.  But by passing a pointer
// to the array into the hashmap code, we can instead use a hashset of
// the indices and indirect through the array to get the key whenever
// the hash lookup needs the key's value, saving us a copy of (the
// pointer to) the key.  We wrap the key value in
//

template <class keyT, typename idxT>
class BidirIndex : public HashTable<keyT,idxT>
   {
   public:
      BidirIndex(size_t initial_size = 1000) : HashTable<keyT,idxT>(initial_size) {}
      BidirIndex(const BidirIndex&) = delete ;
      ~BidirIndex() { delete [] m_reverse_index ; }
      BidirIndex& operator= (const BidirIndex&) = delete ;

      bool findKey(keyT key, idxT* id) const { return lookup(key,id) ; }
      idxT addKey(keyT key) ;
      void addKeySilent(keyT key) ;
      bool finalize() ; // generate the reverse index from the hash table

      idxT indexSize() const { return m_max_index ; }

      idxT getIndex(keyT key) { idxT index ; return lookup(key,&index) ? index : m_errorID ; }
      keyT getKey(idxT index) { return index < m_max_index ? m_reverse_index[index] : (keyT)0 ; }

   protected:
      atomic<idxT> m_next_index { 0 } ;
      idxT         m_max_index { 0 } ;
      idxT         m_errorID { (idxT)-1 } ;
      keyT*        m_reverse_index { nullptr } ;

   protected:
      using HashTable<keyT,idxT>::lookup ;
      using HashTable<keyT,idxT>::contains ;
   } ;

} // end namespace Fr

#endif /* !_Fr_BIDINDEX_H_INCLUDED */

// end of file bidindex.h //
