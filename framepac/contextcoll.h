/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-04					*/
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

template <typename KeyT, typename IdxT, typename ValT, bool sparse = true>
class ContextVectorCollection
   {
   public:
      typedef HashTable<KeyT,Object*>* map_type ;
      typedef Vector<ValT>* context_type ;

      ContextVectorCollection() ;
      ~ContextVectorCollection() ;

      template <typename RetT = SparseVector<IdxT,ValT>>
      typename std::enable_if<sparse,RetT>::type contextVector(const KeyT key) const
	 { return static_cast<RetT>(getContextVector(key)) ; }
      
      template <typename RetT = DenseVector<ValT>>
      typename std::enable_if<!sparse,RetT>::type contextVector(const KeyT key) const
	 { return static_cast<RetT>(getContextVector(key)) ; }

      bool setTermVector(const KeyT term, context_type vector) ;
      context_type getTermVector(const KeyT term) const ;

      bool updateContextVector(const KeyT key, const KeyT term, double weight = 1.0) ;

   protected: // data
      map_type m_term_map ;
      map_type m_context_map ;
      bool m_sparse_vectors { sparse } ;

   protected: // methods
      context_type getContextVector(const KeyT key) const ;
   } ;

// the typical application for this class uses either Symbol or
//  uint32_t as the term type and SparseVector<uint32_t,float> as the
//  context vectors, so predefine those instantiations
extern template class ContextVectorCollection<Symbol*,uint32_t,float> ;
typedef ContextVectorCollection<Symbol*,uint32_t,float> ContextVectorCollSym ;

extern template class ContextVectorCollection<uint32_t,uint32_t,float> ;
typedef ContextVectorCollection<uint32_t,uint32_t,float> ContextVectorCollU32 ;

} // end of namespace Fr

#endif /* !Fr_CONTEXTCOLL_H_INCLUDED */

// end of file contextcoll.h //
