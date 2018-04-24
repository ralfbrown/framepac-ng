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

template <typename ValT>
DenseVector<ValT>::DenseVector(const char* rep)
{
   BufferBuilder<ValT,16> values ;
   while (values.read(rep))
      {
      // nothing to do
      }
   this->m_size = values.size() ;
   this->m_capacity = values.capacity() ;
   this->m_values = values.move() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr DenseVector<ValT>::clone_(const Object* obj)
{
   return obj ? new DenseVector<ValT>(*static_cast<const DenseVector*>(obj)) : nullptr ;
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr DenseVector<ValT>::subseq_int(const Object* obj, size_t start, size_t stop)
{
   if (start > stop || !obj)
      return nullptr ;
   auto orig = static_cast<const DenseVector<ValT>*>(obj) ;
   auto copy = DenseVector<ValT>::create(stop-start) ;
   for (size_t i = start ; i < stop ; ++i)
      {
      copy->m_values[i-start] = orig->m_values[i] ;
      }
   return copy ;
}

//----------------------------------------------------------------------------

template <typename ValT>
ObjectPtr DenseVector<ValT>::subseq_iter(const Object*, ObjectIter start, ObjectIter stop)
{
   return subseq_int(start.baseObject(),start.currentIndex(),stop.currentIndex()) ;
}

//----------------------------------------------------------------------------

template <typename ValT>
DenseVector<ValT>* DenseVector<ValT>::add(const DenseVector<ValT>* other) const
{
   if (!other)
      return static_cast<DenseVector<ValT>*>(&*this->clone().move()) ;
   size_t len = std::max(this->numElements(),other->numElements()) ;
   size_t minlen = std::min(this->numElements(),other->numElements()) ;
   DenseVector<ValT>* result = DenseVector<ValT>::create(len) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      result->m_values[i] = this->m_values[i] + other->m_values[i] ;
      }
   for (size_t i = this->numElements() ; i < len ; ++i)
      {
      result->m_values[i] = other->m_values[i] ;
      }
   for (size_t i = other->numElements() ; i < len ; ++i)
      {
      result->m_values[i] = this->m_values[i] ;
      }
   return result ;
}

//----------------------------------------------------------------------------

template <typename ValT>
template <typename IdxT>
SparseVector<IdxT,ValT>* DenseVector<ValT>::add(const SparseVector<IdxT,ValT>* other) const
{
   if (other) return other->add(this) ;
   SparseVector<IdxT,ValT>* result = SparseVector<IdxT,ValT>::create() ;
   //TODO
   return result ;
}

//----------------------------------------------------------------------------

template <typename ValT>
DenseVector<ValT>* DenseVector<ValT>::incr(const DenseVector<ValT>* other, double wt)
{
   for (size_t idx = 0 ; idx < this->numElements() && idx < other->numElements() ; ++idx)
      {
      this->m_values[idx] += (wt*other->m_values[idx]) ;
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename ValT>
template <typename IdxT>
DenseVector<ValT>* DenseVector<ValT>::incr(const SparseVector<IdxT,ValT>* other)
{
   if (other) return other->add(this) ;
   size_t pos(0) ;
   while (pos < other->numElements())
      {
      IdxT index = other->elementIndex(pos) ;
      if (index >= this->numElements())
	 break ;
      ValT value = other->elementValue(pos++) ;
      this->m_values[index] += value ;
      }
   return this ;
}

//----------------------------------------------------------------------------

template <typename ValT>
template <typename IdxT>
DenseVector<ValT>* DenseVector<ValT>::incr(const SparseVector<IdxT,ValT>* other, ValT wt)
{
   if (other) return other->add(this) ;
   size_t pos(0) ;
   while (pos < other->numElements())
      {
      IdxT index = other->elementIndex(pos) ;
      if (index >= this->numElements())
	 break ;
      ValT value = other->elementValue(pos++) ;
      this->m_values[index] += wt*value ;
      }
   return this ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file densevector.cc //
