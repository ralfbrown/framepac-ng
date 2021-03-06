/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-11					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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

#include <cstring>
#include "template/bufbuilder.cc"
#include "framepac/vector.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class DenseVector					*/
/************************************************************************/

template <typename IdxT, typename ValT>
DenseVector<IdxT,ValT>::DenseVector(const char* rep)
{
   BufferBuilder<ValT,16> values ;
   while (values.read(rep))
      {
      // nothing to do
      }
   this->m_size = values.size() ;
   this->m_capacity = values.capacity() ;
   this->m_values.full = values.move() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
DenseVector<IdxT,ValT>::DenseVector(size_t cap) : super(cap)
{
   this->reserve(cap) ;
   this->m_size = cap ;
   ValT* values = this->m_values.full ;
   std::fill(values,values+cap,ValT(0)) ;
   return ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr DenseVector<IdxT,ValT>::clone_(const Object* obj)
{
   return obj ? new DenseVector<IdxT,ValT>(*static_cast<const DenseVector*>(obj)) : nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr DenseVector<IdxT,ValT>::subseq_int(const Object* obj, size_t start, size_t stop)
{
   if (start > stop || !obj)
      return nullptr ;
   auto orig = static_cast<const DenseVector<IdxT,ValT>*>(obj) ;
   auto copy = DenseVector<IdxT,ValT>::create(stop-start) ;
   std::copy(orig->m_values.full+start,orig->m_values.full+stop,copy->m_values.full) ;
   return copy ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ObjectPtr DenseVector<IdxT,ValT>::subseq_iter(const Object*, ObjectIter start, ObjectIter stop)
{
   return subseq_int(start.baseObject(),start.currentIndex(),stop.currentIndex()) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
DenseVector<IdxT,ValT>* DenseVector<IdxT,ValT>::add(const DenseVector<IdxT,ValT>* other) const
{
   if (!other)
      return static_cast<DenseVector<IdxT,ValT>*>(&*this->clone().move()) ;
   size_t len = std::max(this->numElements(),other->numElements()) ;
   size_t minlen = std::min(this->numElements(),other->numElements()) ;
   DenseVector<IdxT,ValT>* result = DenseVector<IdxT,ValT>::create(len) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      result->m_values.full[i] = this->m_values.full[i] + other->m_values.full[i] ;
      }
   for (size_t i = this->numElements() ; i < len ; ++i)
      {
      result->m_values.full[i] = other->m_values.full[i] ;
      }
   for (size_t i = other->numElements() ; i < len ; ++i)
      {
      result->m_values.full[i] = this->m_values.full[i] ;
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
SparseVector<IdxT,ValT>* DenseVector<IdxT,ValT>::add(const SparseVector<IdxT,ValT>* other) const
{
   if (other) return other->add(this) ;
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create() ;
   //TODO
   return result ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
DenseVector<IdxT,ValT>* DenseVector<IdxT,ValT>::incr(const DenseVector<IdxT,ValT>* other, double wt)
{
   for (size_t indx = 0 ; indx < this->numElements() && indx < other->numElements() ; ++indx)
      {
      this->m_values.full[indx] += (wt*other->m_values.full[indx]) ;
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
DenseVector<IdxT,ValT>* DenseVector<IdxT,ValT>::incr(const SparseVector<IdxT,ValT>* other)
{
   if (other) return (DenseVector<IdxT,ValT>*)other->add(this) ;
   size_t pos(0) ;
   while (pos < other->numElements())
      {
      IdxT index = other->elementIndex(pos) ;
      if (index >= this->numElements())
	 break ;
      ValT value = other->elementValue(pos++) ;
      this->m_values.full[index] += value ;
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
DenseVector<IdxT,ValT>* DenseVector<IdxT,ValT>::incr(const SparseVector<IdxT,ValT>* other, ValT wt)
{
   if (other) return (DenseVector<IdxT,ValT>*)other->add(this) ;
   size_t pos(0) ;
   while (pos < other->numElements())
      {
      IdxT index = other->elementIndex(pos) ;
      if (index >= this->numElements())
	 break ;
      ValT value = other->elementValue(pos++) ;
      this->m_values.full[index] += wt*value ;
      }
   return this ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file densevector.cc //
