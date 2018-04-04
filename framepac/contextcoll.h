/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-03					*/
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

#ifndef Fr_CONTEXTCOLL_H_INCLUDED
#define Fr_CONTEXTCOLL_H_INCLUDED

#include "framepac/hashtable.h"
#include "framepac/vector.h"

namespace Fr
{

template <typename KeyT, typename IdxT, typename ValT>
class ContextVectorCollection
   {
   public:
      typedef HashTable<KeyT,Object*> map_type ;
      typedef Vector<ValT> context_type ;

      ContextVectorCollection() ;
      ~ContextVectorCollection() ;

   protected:
      map_type m_term_map ;
      map_type m_context_map ;
   } ;

// the typical application for this class uses Symbol as the term type and SparseVector<uint32_t,float>
//  as the context vectors, so predefine that instantiation
extern template class ContextVectorCollection<Symbol*,uint32_t,float> ;
typedef ContextVectorCollection<Symbol*,uint32_t,float> ContextVectorColl ;

} // end of namespace Fr

#endif /* !Fr_CONTEXTCOLL_H_INCLUDED */

// end of file contextcoll.h //
