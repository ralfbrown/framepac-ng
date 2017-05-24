/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-22					*/
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

#ifndef __Fr_MEMORY_H_INCLUDED
#define __Fr_MEMORY_H_INCLUDED

#include <mutex>
#include <thread>
#include "framepac/atomic.h"
#include "framepac/init.h"
#include "framepac/objectvmt.h"

#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

/************************************************************************/
/************************************************************************/

// private classes, types, and functions
namespace FramepaC
{

//----------------------------------------------------------------------------

typedef uint16_t alloc_size_t ;

//----------------------------------------------------------------------------

struct SlabFreelist
   {
   private:
      SlabFreelist* m_next { nullptr };
   public:
      SlabFreelist() {}
      SlabFreelist(SlabFreelist* nxt) : m_next(nxt) {}
      // copy ctor:
      SlabFreelist(const SlabFreelist& nxt) : m_next(const_cast<SlabFreelist*>(nxt.next())) {}
      // move ctor:
      SlabFreelist(SlabFreelist&& nxt) : m_next(nxt.next()) { nxt.next((void*)nullptr) ; }
      ~SlabFreelist() {}
      void operator= (const SlabFreelist&) = delete ;

      SlabFreelist* next() { return m_next ; }
      const SlabFreelist* next() const { return m_next ; }
      void next(SlabFreelist* nxt) { m_next = nxt ; }
      void next(void* nxt) { m_next = (SlabFreelist*)nxt ; }
      void invalidateNext() { m_next = (SlabFreelist*)~0UL ; }
      bool invalidNext() const { return m_next == (SlabFreelist*)~0UL ; }
   } ;

//----------------------------------------------------------------------------

class SlabGroup ;

class Slab
   {
   public:
      Slab() ;
      ~Slab() ;
      void operator= (const Slab&) = delete ;

      // only allow placement new
      void* operator new(size_t) = delete ;
      void* operator new(size_t, void* where) { return where ; }

      SlabGroup* containingGroup() const
         { return reinterpret_cast<SlabGroup*>(const_cast<Slab*>(this) - m_info.m_slab_id) ; }

      static alloc_size_t slabOffset(void* block)
	 { return (alloc_size_t)(((uintptr_t)block) & (SLAB_SIZE-1)) ; }
      static const ObjectVMT* VMT(const void* block)
         { // convert the pointer to the allocated block into a pointer to the slab
	    Slab* slab = (Slab*)(((uintptr_t)block)&~(SLAB_SIZE-1)) ;
	    return slab->m_info.m_vmt ; }

      void linkSlab(Slab*& listhead) ;
      void unlinkSlab(Slab*& listhead) ;
      void* initFreelist(unsigned objsize) ;
      static unsigned bufferSize() { return sizeof(m_buffer) ; }

      void* allocObject() ;
      void releaseObject(void* obj) ;
   private:
      // group together all the header fields, so that we can use a single sizeof()
      //   in computing the header size
      class alignas(64) SlabInfo   // ensure separate cache line for the SlabInfo and SlabHeader
         {
	 public:
	    const ObjectVMT* m_vmt ;		// should be first to avoid having to add an offset
	    Slab*            m_nextslab { nullptr };
	    Slab*            m_prevslab { nullptr };
	    // m_owner ;
	    alloc_size_t     m_objsize ;
	    uint16_t         m_objcount ;
	    uint16_t         m_slab_id ;
         } ;
      class SlabHeader
         {
	 public:
	    alloc_size_t     m_freelist ;
	    uint16_t         m_freecount ;
	 public:
	    //SlabHeader() = default ;
	    ~SlabHeader() = default ;
         } ;
      // group together all the footer fields, so that we can use a
      //   single sizeof() in computing the footer size
      class SlabFooter
         {
	 public:
	    Fr::Atomic<alloc_size_t> m_firstfree ;

	 public:
	    SlabFooter() { m_firstfree.store(0) ; }
	    ~SlabFooter() {}
         } ;
   public:
      static constexpr size_t DATA_SIZE = SLAB_SIZE - sizeof(SlabInfo) - sizeof(SlabHeader) - sizeof(SlabFooter) ;
   private:
      SlabInfo   m_info ;
      SlabHeader m_header ;
      uint8_t  m_buffer[DATA_SIZE] ;
      // we put info that is modified by threads other than the slab's
      //   current owner at the end of the slab to ensure that it is
      //   in a different cache line than the unchanging but
      //   frequently-read header info and the data that the owner is
      //   constantly changing
      SlabFooter m_footer ;
      friend class SlabGroup ;
   } ;

//----------------------------------------------------------------------------

class SlabGroup
   {
   public:
      SlabGroup() ;
      SlabGroup(const SlabGroup&) = delete ;
      ~SlabGroup() { unlink() ; }
      void operator= (const SlabGroup&) = delete ;

      static Slab* allocateSlab() ;
      void releaseSlab(Slab* slab) ;

   private:
      static mutex s_mutex ;
      static SlabGroup* s_grouplist_fwd ;
      static SlabGroup* s_grouplist_rev ;
   private:
      Slab m_slabs[SLAB_GROUP_SIZE] ;
      SlabGroup* m_next ;
      SlabGroup* m_prev { nullptr };
      Slab*      m_freeslabs { nullptr };
      unsigned   m_numfree { lengthof(m_slabs) };
      size_t	 m_freemask[(SLAB_GROUP_SIZE/sizeof(size_t))/CHAR_BIT] ;
   protected:
      void* operator new(size_t sz) ;
      void operator delete(void* grp) ;
   protected:
      void unlink() ;
   } ;

// end of namespace FramepaC
}

/************************************************************************/

// public classes, types, and functions

namespace Fr
{

class AllocatorBase
   {
   public:
      AllocatorBase(unsigned objsize) : m_objsize(objsize)
	 { m_slabs = nullptr ; m_freelist = nullptr ; m_pending = nullptr ; }
      AllocatorBase(const AllocatorBase&) = delete ;
      ~AllocatorBase() ;
      void operator= (const AllocatorBase&) = delete ;

      [[gnu::hot]] [[gnu::malloc]]
      void* allocate()
         {
	 if (!m_freelist) return allocate_more() ;
	 void* item = m_freelist ;
	 m_freelist = m_freelist->next() ;
	 return item ;
	 }
      [[gnu::hot]]
      void release(void* blk)
         {
	 // to allow other threads to steal the "pending" list without races, we need to
	 //   ensure that there is never an unrecognized stale pointer in the list
	 // first, set the 'next' pointer in the block being freed to an obvious invalid
	 //   value, then atomically swap the head of the freelist with the new block
	 // if someone else sneaks in at that point, they will see the obvious invalid next
	 //   pointer in the chain and know that they need to wait for it to be fixed up,
	 //   which is done by storing the old head of the freelist in the next pointer
	 SlabFreelist* block = reinterpret_cast<SlabFreelist*>(blk) ;
	 block->invalidateNext() ;
	 SlabFreelist* next = Atomic<SlabFreelist*>::ref(m_pending).exchange(block) ;
	 block->next(next) ; 
	 }
      size_t reclaim() ;

   protected:
      typedef FramepaC::SlabFreelist SlabFreelist ;
      typedef FramepaC::Slab Slab ;
   protected: // data members
      static thread_local SlabFreelist* m_freelist ;
      static thread_local SlabFreelist* m_pending ;
      static thread_local Slab* m_hazard ;
      static HazardPointerList  m_hazardlist ;
      static Slab*              m_slabs ;
      Slab*                     m_activeslab { nullptr };
      FramepaC::alloc_size_t    m_objsize ;
   protected: // methods
      SlabFreelist* reclaimPending(SlabFreelist*&) ;
      SlabFreelist* stealPendingFrees() ;
      bool reclaim_foreign_frees() ;
      void* allocate_more() ;

      static void threadInit() ;
      static void threadCleanup() ;
   } ;

//----------------------------------------------------------------------------

template <class ObjT>
class Allocator : public AllocatorBase
   {
   private:
      const FramepaC::Object_VMT<ObjT>* m_vmt ;

   public:
      Allocator(const FramepaC::Object_VMT<ObjT>* vmt, unsigned extra = 0)
	 : AllocatorBase(sizeof(ObjT)+extra), m_vmt(vmt)
	 { /*assert(sizeof(ObjT)+extra <= FramepaC::SLAB_SIZE/8) ;*/ }
      Allocator(const Allocator&) = delete ;
      ~Allocator() = default ;
      void operator= (const Allocator&) = delete ;
   protected:
      typedef Fr::ThreadInitializer<Allocator> ThreadInitializer ;
   protected: // data members
      static ThreadInitializer m_threadinit ;
   } ;

} ;

//----------------------------------------------------------------------------

template <typename T, size_t SZ = 512>
class LocalAlloc
   {
   private:
      T  m_localbuffer[SZ] ;
      T* m_buffer ;

   public:
      LocalAlloc(size_t alloc)
	 {
	 m_buffer = (alloc > SZ) ? new T[alloc] : m_localbuffer ;
	 }
      LocalAlloc(size_t alloc, bool clear)
	 {
	 m_buffer = (alloc > SZ) ? new T[alloc] : m_localbuffer ;
	 if (m_buffer && clear)
	    memset(m_buffer,'\0',sizeof(T)*alloc) ;
	 }
      ~LocalAlloc()
	 {
	 if (m_buffer != m_localbuffer) delete [] m_buffer ;
	 }

      T* base() const { return m_buffer ; }
      T& operator [] (size_t idx) { return m_buffer[idx] ; }
      const T& operator [] (size_t idx) const { return m_buffer[idx] ; }
      operator T* () { return m_buffer ; }
   } ;

#endif /* !__Fr_MEMORY_H_INCLUDED */

// end of file memory.h //
