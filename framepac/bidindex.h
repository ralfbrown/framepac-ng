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

#ifndef _Fr_BIDINDEX_H_INCLUDED
#define _Fr_BIDINDEX_H_INCLUDED

// FIX to allow old and new FramepaC to co-exist
#undef FrHASHTABLE_MIN_SIZE
#undef FrHASHTABLE_SEARCHRANGE
#undef INCR_COUNT
#undef DECR_COUNT
#undef FrMAKE_INTEGER_HASHTABLE_CLASS
#undef FrMAKE_SYMBOL_HASHTABLE_CLASS

#include "framepac/hashtable.h"
//FIXME: until hashtable.h is fixed
//#include "framepac/symbol.h"
//#include "framepac/string.h"

namespace Fr
{

template <class keyT, typename idxT>
class BidirIndex //FIXME : public HashTable<keyT,idxT>
   {
   private:
      atomic<idxT> m_next_index { 0 } ;
      idxT         m_max_index { 0 } ;
      idxT         m_errorID { (idxT)-1 } ;
      keyT*        m_reverse_index { nullptr } ;
   public:
      //FIXME BidirIndex(size_t initial_size = 1000) : HashTable(initial_size) {}
      BidirIndex(size_t /*initial_size FIXME*/ = 1000) : m_next_index(0) {}
      BidirIndex(const BidirIndex&) = delete ;
      ~BidirIndex() { delete [] m_reverse_index ; }
      BidirIndex& operator= (const BidirIndex&) = delete ;

      idxT addKey(keyT key) ;
      void addKeySilent(keyT key) ;
      bool finalize() ; // generate the reverse index from the hash table

      idxT getIndex(keyT key) { idxT index ; return lookup(key,&index) ? index : m_errorID ; }
      keyT getKey(idxT index) { return index < m_max_index ? m_reverse_index[index] : (keyT)0 ; }
   } ;

} // end namespace Fr

#endif /* !_Fr_BIDINDEX_H_INCLUDED */

// end of file bidindex.h //
