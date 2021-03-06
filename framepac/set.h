/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-19					*/
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

#ifndef _Fr_SET_H_INCLUDED
#define _Fr_SET_H_INCLUDED

#include "framepac/hashtable.h"

namespace Fr {

//----------------------------------------------------------------------------
// forward declarations

class Set ;
class List ;

//----------------------------------------------------------------------------

class SetIter
   {
   private:
      Set   *m_set ;
      size_t m_index ;
   public:
      SetIter(Set *s,size_t idx) : m_set(s), m_index(idx) {}
      SetIter(const SetIter &) = default ;
      ~SetIter() = default ;

      Object* operator* () ;
      const Object* operator* () const ;
      Object* operator-> () ;
      SetIter& operator++ () { ++m_index ; return *this ; }
      const Object* operator[] (size_t index) const ;

      bool operator== (const SetIter& other) const { return m_set == other.m_set && m_index == other.m_index ; }
      bool operator!= (const SetIter& other) const { return m_set != other.m_set || m_index != other.m_index ; }
   } ;

//----------------------------------------------------------------------------

class ConstSetIter
   {
   private:
      const Set *m_set ;
      size_t     m_index ;
   public:
      ConstSetIter(const Set *s,size_t idx) : m_set(s), m_index(idx) {}
      ConstSetIter(const ConstSetIter &) = default ;
      ~ConstSetIter() = default ;

      Object* operator* () const ;
      const Object* operator-> () ;
      ConstSetIter& operator++ () { ++m_index ; return *this ; }
      const Object* operator[] (size_t index) const ;

      bool operator== (const ConstSetIter& other) const { return m_set == other.m_set && m_index == other.m_index ; }
      bool operator!= (const ConstSetIter& other) const { return m_set != other.m_set || m_index != other.m_index ; }
   } ;

//----------------------------------------------------------------------------

class Set : public ObjHashSet
   {
   public: // types
      typedef ObjHashSet super ;
      typedef SetIter iterator ;
      typedef ConstSetIter const_iterator ;
   public:
      static Set *create(size_t capacity = 0) { return new Set(capacity) ; }
      static Set *create(const List *) ;
      static Set *create(const Set *orig) { return new Set(orig) ; }

      bool add(Object* key) { return add(key) ; }

      // *** standard info functions ***
      size_t size() const { return m_size ; }
      size_t empty() const { return size() == 0 ; }

      // *** standard access functions ***
      Object *front() const { return *cbegin() ; }
      Set *subseq(SetIter start, SetIter stop, bool shallow = false) const ;

      // *** iterator support ***
      iterator begin() { return iterator(this,0) ; }
      const_iterator begin() const { return const_iterator(this,0) ; }
      const_iterator cbegin() const { return const_iterator(this,0) ; }
      iterator end() { return iterator(this,size()) ; }
      const_iterator end() const { return const_iterator(this,size()) ; }
      const_iterator cend() const { return const_iterator(this,size()) ; }

      // STL compatibility
      bool reserve(size_t n) ;
      
   protected: // construction/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      Set(size_t initial_size = 0) ;
      Set(const Set* orig) ;
      Set(const Set& orig) ;
      ~Set() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Set> ;
      friend class SetIter ;
      friend class SymbolTable ; //TEMP

      // type determination predicates
      static bool isSet_(const Object*) { return true ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*, char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
			        size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return static_cast<const Set*>(obj)->size() ; }
      static bool empty_(const Object* obj) { return static_cast<const Set*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object* front_(Object* obj) { return static_cast<const Set*>(obj)->front() ; }
      static const Object* front_(const Object *obj) { return static_cast<const Set*>(obj)->front() ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

      // *** iterator support ***

   private: // static members
      static Allocator s_allocator ;
      static const char s_typename[] ;

   protected:
      size_t     m_size ;
   } ;

//----------------------------------------------------------------------------

//inline Object* SetIter::operator* () { return m_set->getIndex(m_index) ; }
//inline const Object* SetIter::operator* () const { return m_set->getIndex(m_index) ; }
inline Object* SetIter::operator* () { return nullptr ; } //FIXME
inline const Object* SetIter::operator* () const { return nullptr ; } //FIXME

inline Object* ConstSetIter::operator* () const { return nullptr ; } //FIXME

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Set> ;

} ; // end namespace FramepaC

#endif /* !_Fr_SET_H_INCLUDED */

// end of file set.h //
