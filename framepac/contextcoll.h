/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-09-03					*/
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
      typedef HashTable<KeyT,Object*> map_type ;
      typedef Vector<ValT> context_type ;

      ContextVectorCollection() ;
      ~ContextVectorCollection() ;

      template <typename RetT = SparseVector<IdxT,ValT>*>
      typename std::enable_if<sparse,RetT>::type contextVector(const KeyT key) const
	 { return static_cast<RetT>(getContextVector(key)) ; }
      
      template <typename RetT = DenseVector<ValT>*>
      typename std::enable_if<!sparse,RetT>::type contextVector(const KeyT key) const
	 { return static_cast<RetT>(getContextVector(key)) ; }

      void setDimensions(size_t dim) { if (!m_sparse_vectors) m_dimensions = dim ; }
      size_t dimensions() const { return m_dimensions ; }
      void setBasisDimensions(size_t plus, size_t minus) { m_plusdim = plus ; m_minusdim = minus ; }
      size_t plusDimensions() const { return m_plusdim ; }
      size_t minusDimensions() const { return m_minusdim ; }

      bool haveTermVector(const KeyT term) const ;
      bool setTermVector(const KeyT term, context_type* vector) ;
      bool setOneHotVector(const KeyT term, IdxT index, ValT value, double weight = 1.0) ;
      context_type* makeTermVector(const KeyT term) ;
      context_type* getTermVector(const KeyT term) const ;

      bool addTerm(const KeyT key, const KeyT term, double weight = 1.0) ;
      bool updateContextVector(const KeyT key, const KeyT term, double weight = 1.0) ;

   protected: // data
      map_type* m_term_map ;
      map_type* m_context_map ;
      unsigned m_dimensions { 0 } ;
      unsigned m_plusdim { 4 } ;
      unsigned m_minusdim { 4 } ;
      bool m_sparse_vectors { sparse } ;

   protected: // methods
      context_type* getContextVector(const KeyT key) const ;
      context_type* makeContextVector(const KeyT key) ;
   } ;

// the typical application for this class uses either Symbol or
//  uint32_t as the term type and either SparseVector<uint32_t,float>
//  or DenseVector<float> as the context vectors, so predefine those
//  instantiations
extern template class ContextVectorCollection<Symbol*,uint32_t,float,true> ;
typedef ContextVectorCollection<Symbol*,uint32_t,float> ContextVectorCollSym ;

extern template class ContextVectorCollection<Symbol*,uint32_t,float,false> ;
typedef ContextVectorCollection<Symbol*,uint32_t,float> ContextVectorCollSymDense ;

extern template class ContextVectorCollection<uint32_t,uint32_t,float,true> ;
typedef ContextVectorCollection<uint32_t,uint32_t,float> ContextVectorCollU32 ;

extern template class ContextVectorCollection<uint32_t,uint32_t,float,false> ;
typedef ContextVectorCollection<uint32_t,uint32_t,float> ContextVectorCollU32Dense ;

} // end of namespace Fr

#endif /* !Fr_CONTEXTCOLL_H_INCLUDED */

// end of file contextcoll.h //
