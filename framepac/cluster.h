/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-26					*/
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

#ifndef __FrCLUSTER_H_INCLUDED
#define __FrCLUSTER_H_INCLUDED

#include "framepac/list.h"
#include "framepac/symbol.h"

namespace Fr {

//----------------------------------------------------------------------------
// due to the circular dependencies, we can't actually define many of the functions
//   inline in the iterator class definition; they will be defined after the underlying
//   class has been declared

class ClusterInfo ;

class ClusterInfoIter
   {
   private:
      List	*m_members ;
   public:
      ClusterInfoIter() : m_members(reinterpret_cast<List*>(*List::end())) {}
      ClusterInfoIter(ClusterInfo* inf) ;
      ClusterInfoIter(const ClusterInfo* inf) ;
      ClusterInfoIter(const ClusterInfoIter &it) : m_members(const_cast<List*>(it.m_members)) {}
      ~ClusterInfoIter() = default ;

      inline Object* operator* () const ;
      const List* operator-> () const { return  m_members ; }
      inline ClusterInfoIter& operator++ () ;
      bool operator== (const ClusterInfoIter& other) const { return m_members == other.m_members ; }
      bool operator!= (const ClusterInfoIter& other) const { return m_members != other.m_members ; }
   } ;

//----------------------------------------------------------------------------

class ClusterInfo : public Object
   {
   protected:
      typedef Fr::Initializer<ClusterInfo> Initializer ;

   public:
      // *** object factories ***
      static ClusterInfo* create() ;
      
      // *** standard info functions ***
      //size_t size() const ;
      //bool empty() const { return size() == 0 ; }
      operator bool () const { return !this->empty() ; }

      // *** iterator support ***
      ClusterInfoIter begin() const { return ClusterInfoIter(this) ; }
      ClusterInfoIter cbegin() const { return ClusterInfoIter(this) ; }
      static ClusterInfoIter end() { return ClusterInfoIter() ; }
      static ClusterInfoIter cend() { return ClusterInfoIter() ; }

      // *** utility functions ***
      bool contains(const Object*) const ; // is Object a member of the cluster?

      // *** access to internal state ***
      const List* members() const { return m_members ; }

   protected:
      List* m_members ;		// individual vectors in this cluster
      List* m_subclusters ;	// sub-clusters (if any) of this cluster
      Symbol* m_label ;		// cluster label
      uint32_t m_size ;		// number of elements in this cluster
      uint32_t m_flags ;

   private: // static members
      static Allocator s_allocator ;
      static Initializer s_init ;

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      ClusterInfo() {}
      ~ClusterInfo() {}
      ClusterInfo& operator= (const ClusterInfo&) = delete ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<ClusterInfo> ;
      
      // type determination predicates
      static bool isCluster_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "ClusterInfo" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object*obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) ;
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { return free_(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return reinterpret_cast<const ClusterInfo*>(obj)->m_size ; }
      static bool empty_(const Object* obj) { return size_(obj) == 0 ; }

      // *** standard access functions ***
      static Object* front_(Object*) ;
      static const Object* front_(const Object*) ;

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

   public:
      // *** startup/shutdown functions ***
      static void StaticInitialization() ;
      static void StaticCleanup() ;
   } ;

//----------------------------------------------------------------------------
// deferred definitions of functions subject to circular dependencies

inline ClusterInfoIter::ClusterInfoIter(ClusterInfo* inf)
   : m_members(const_cast<List*>(inf->members()))
{
   return  ;
}

//----------------------------------------------------------------------------

inline ClusterInfoIter::ClusterInfoIter(const ClusterInfo* inf)
   : m_members(const_cast<List*>(inf->members()))
{
   return ;
}

//----------------------------------------------------------------------------


class ClusteringAlgo
   {
   public:
      static ClusteringAlgo* instantiate(const char* algo_name, ...) ;
      virtual ~ClusteringAlgo() {}

   protected:
      ClusteringAlgo() {}

   } ;

} ; // end of namespace Fr

#endif /* !__FrCLUSTER_H_INCLUDED */

// end of file cluster.h //
