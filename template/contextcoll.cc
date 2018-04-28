/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-04-27					*/
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

#include "framepac/contextcoll.h"
#include "framepac/basisvector.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::ContextVectorCollection()
{
   m_term_map = static_cast<map_type*>(map_type::create()) ;
   m_context_map = static_cast<map_type*>(map_type::create()) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::~ContextVectorCollection()
{
   m_term_map->free() ;
   m_context_map->free() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
bool ContextVectorCollection<KeyT,IdxT,ValT,sparse>::setTermVector(const KeyT term, typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type* vector)
{
   return m_term_map->add(term,vector) ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
bool ContextVectorCollection<KeyT,IdxT,ValT,sparse>::setOneHotVector(const KeyT term, IdxT index, ValT value,
   double weight)
{
   typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type* vector;
   if (sparse)
      {
      vector = OneHotVector<IdxT,ValT>::create(index,value) ;
      }
   else
      {
      vector = DenseVector<ValT>::create(m_dimensions) ;
      vector->setElement(index,value) ;
      }
   vector->setWeight(weight) ;
   return this->setTermVector(term,vector) ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
bool ContextVectorCollection<KeyT,IdxT,ValT,sparse>::haveTermVector(const KeyT term) const
{
   return m_term_map->contains(term) ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type*
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::makeTermVector(const KeyT term)
{
   auto termvec = getTermVector(term) ;
   if (termvec)
      return termvec ;
   if (this->dimensions() == 0)
      {
      termvec = OneHotVector<IdxT,ValT>::create(m_term_map->size(),1) ;
      }
   else
      {
      termvec = BasisVector<IdxT,ValT>::create(this->dimensions(),this->plusDimensions(),this->minusDimensions()) ;
      }
   if (!this->setTermVector(term,termvec))
      {
      // a concurrent process added the term before we got to it, so retrieve the vector the other process set
      termvec->free() ;
      termvec = getTermVector(term) ;
      }
   return termvec ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type*
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::getTermVector(const KeyT term) const
{
   return static_cast<context_type*>(m_term_map->lookup(term)) ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
bool ContextVectorCollection<KeyT,IdxT,ValT,sparse>::addTerm(const KeyT key, const KeyT term, double wt)
{
   auto context = makeContextVector(key) ;
   auto termvec = makeTermVector(term) ;
   context->incr(termvec,wt*termvec->weight()) ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
bool ContextVectorCollection<KeyT,IdxT,ValT,sparse>::updateContextVector(const KeyT key, const KeyT term, double wt)
{
   auto context = makeContextVector(key) ;
   auto termvec = getTermVector(term) ;
   if (termvec)
      context->incr(termvec,wt*termvec->weight()) ;
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type*
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::getContextVector(const KeyT key) const
{
   return static_cast<context_type*>(m_context_map->lookup(key)) ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type*
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::makeContextVector(const KeyT key)
{
   auto context = getContextVector(key) ;
   if (!context)
      {
      context = static_cast<context_type*>(context_type::create(m_dimensions)) ;
      if (!m_context_map->add(key,context))
	 {
	 // another process snuck in and created a context vector for this term, so retrieve the vector which
	 //  is already in the map
	 context->free() ;
	 context = getContextVector(key) ;
	 }
      }
   return context ;
}

//----------------------------------------------------------------------------


} // end namespace Fr

// end of file contextcoll.cc //
