/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.02, last edit 2017-07-16					*/
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
   public:
      static TermVectorT* create(size_t capacity = 0) { return new TermVectorT(capacity) ; }

      static TermVectorT* read(CharGetter& getter, size_t size_hint = 0) ;

      size_t vectorFreq() const { return m_freq ; }

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      TermVectorT(size_t capacity = 1) : SparseVector<uint32_t,ValT>(capacity), m_freq(0)
	 {
	 }
      ~TermVectorT() ;

      using SparseVector<uint32_t,ValT>::setElement ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<TermVectorT> ;

      // type determination predicates
      static bool isTermVector_(const Object*) { return true ; }
      static const char* typeName_(const Object*) ;

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<TermVectorT*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { free_(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      using SparseVector<uint32_t,ValT>::cStringLength_ ;
      using SparseVector<uint32_t,ValT>::toCstring_ ;
      using SparseVector<uint32_t,ValT>::jsonStringLength_ ;
      using SparseVector<uint32_t,ValT>::toJSONString_ ;

      // *** standard info functions ***
      static size_t size_(const Object*) ;
      //static bool empty_(const Object* obj) : inherited from SparseVector

      // *** standard access functions ***
      static Object* front_(Object*) { return nullptr ; }
      static const Object* front_(const Object*) { return nullptr ; }
      static double floatValue_(const Object* obj) { return static_cast<const TermVectorT*>(obj)->length() ; }
      static long int intValue(const Object* obj)
	 { return (long)(floatValue_(obj) + 0.5) ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

   private: // static members
      static Allocator s_allocator ;

   protected:
      size_t m_freq ;
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
