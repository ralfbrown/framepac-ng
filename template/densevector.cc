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
   if (rep)
      {
      //TODO
      }
   this->m_size = values.size() ;
   this->m_capacity = values.capacity() ;
   this->m_values = values.move() ;
   return ;
}

//----------------------------------------------------------------------------

template <typename ValT>
DenseVector<ValT>* DenseVector<ValT>::add(const DenseVector<ValT>* other) const
{
   if (!other)
      return static_cast<DenseVector<ValT>*>(&*this->clone()) ;
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

} // end namespace Fr

// end of file densevector.cc //
