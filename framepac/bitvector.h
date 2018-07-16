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

#ifndef __Fr_BITVECTOR_H_INCLUDED
#define __Fr_BITVECTOR_H_INCLUDED

#include "framepac/object.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

class BitVector ; // forward declaration

// declarations of other classes that we reference but don't need details for right here
class Array ;
class List ;

//----------------------------------------------------------------------------
// due to the circular dependencies, we can't actually define some of the functions
//   inline in the iterator class definition; they will be defined after the underlying
//   class has been declared

class BitVectorIter
   {
   public:
      BitVectorIter(BitVector* v, size_t n = 0) : m_vector(v), m_bitnum(n) {}
      BitVectorIter(const BitVector* v, size_t n = 0) : m_vector(v), m_bitnum(n) {}
      BitVectorIter(const BitVectorIter &o) : m_vector(o.m_vector), m_bitnum(o.m_bitnum) {}
      ~BitVectorIter() = default ;

      inline bool operator* () const ;
      inline BitVectorIter& operator++ () { ++m_bitnum ; return *this ; }
      bool operator== (const BitVectorIter& other) const
	 { return m_vector == other.m_vector && m_bitnum == other.m_bitnum ; }
      bool operator!= (const BitVectorIter& other) const { return !(*this == other) ; }

   private:
      const BitVector* m_vector ;
      size_t           m_bitnum ;
   } ;

//----------------------------------------------------------------------------

class BitVector : public Object
   {
   public:
      typedef Object super ;
   public:
      // *** object factories ***
      static BitVector* create(size_t capacity) { return new BitVector(capacity) ; }
      static BitVector* create(const char*) ; // string of '0' and '1' characters
      static BitVector* create(const List*) ;
      static BitVector* create(const List&) ;
      static BitVector* create(const Array&) ;

      bool getBit(size_t N) const ;
      void setBit(size_t N, bool set) ;

      // *** standard info functions ***
      size_t size() const { return m_size ; }
      size_t empty() const { return size() == 0 ; }

      // *** standard access functions ***
      Object* front() const ; // { return getBit(0) ; }

      BitVector* subseq(BitVectorIter start, BitVectorIter stop, bool shallow = false) const ;

      // *** iterator support ***
      BitVectorIter begin() const { return BitVectorIter(this) ; }
      BitVectorIter cbegin() const { return BitVectorIter(this) ; }
      BitVectorIter end() const { return BitVectorIter(this,size()) ; }
      BitVectorIter cend() const { return BitVectorIter(this,size()) ; }

      // STL compatibility
      size_t capacity() const { return m_capacity ; }

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      BitVector(size_t capacity) ;
      ~BitVector() ;
      BitVector& operator= (const BitVector&) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<BitVector> ;
      friend class BitVectorIter ; 

      // type determination predicates
      static bool isBitVector_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "BitVector" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object*) ;
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) ;
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object*) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return obj->size() ; }
      static bool empty_(const Object* obj) { return obj->size() == 0 ; }

      // *** standard access functions ***
      static Object* front_(Object*) ;
      static const Object* front_(const Object*) ;
      static const char* stringValue_(const Object*) { return nullptr ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

   private: // static members
      static Allocator s_allocator ;
   protected: // data members
      size_t* m_bits ;
      size_t  m_size ;
      size_t  m_capacity ;
      
   } ;

//----------------------------------------------------------------------------

bool BitVectorIter::operator* () const
{
   return m_vector->getBit(m_bitnum) ;
}

//----------------------------------------------------------------------------

} ;  // end of namespace Fr

#endif /* !__Fr_BITVECTOR_H_INCLUDED */

// end of file bitvector.h //
