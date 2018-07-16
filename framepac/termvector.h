/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#ifndef __FrTERMVECTOR_H_INCLUDED
#define __FrTERMVECTOR_H_INCLUDED

#include "framepac/vector.h"

namespace Fr
{

// forward declaration
class CharGetter ;

/************************************************************************/
/************************************************************************/

template <typename ValT>
class TermVectorT : public SparseVector<uint32_t,ValT>
   {
   public: // types
      typedef SparseVector<uint32_t,ValT> super ;
   public:
      static TermVectorT* create(size_t capacity = 0) { return new TermVectorT(capacity) ; }

      static TermVectorT* read(CharGetter& getter, size_t size_hint = 0) ;

      void vectorFreq(size_t f) { this->setWeight((float)f) ; }
      size_t vectorFreq() const { return (size_t)this->weight() ; }

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      TermVectorT(size_t capacity = 1) : super(capacity) {}
      ~TermVectorT() {}

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<TermVectorT> ;

      // type determination predicates
      static bool isTermVector_(const Object*) { return true ; }
      static const char* typeName_(const Object*) ;

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<TermVectorT*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { free_(obj) ; }

   private: // static members
      static Allocator s_allocator ;

   } ;

//----------------------------------------------------------------------------

template <>
inline const char* TermVectorT<uint32_t>::typeName_(const Object*) { return "TermCountVector" ; }

typedef TermVectorT<uint32_t> TermCountVector ;
extern template class TermVectorT<uint32_t> ;

//----------------------------------------------------------------------------

template <>
inline const char* TermVectorT<float>::typeName_(const Object*) { return "TermVector" ; }

typedef TermVectorT<float> TermVector ;
extern template class TermVectorT<float> ;

} ; // end of namespace Fr

#endif /* !__FrVECTOR_H_INCLUDED */

// end of termvector.h //
