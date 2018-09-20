/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-19					*/
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

#ifndef __Fr_MEMORY_H_INCLUDED
#define __Fr_MEMORY_H_INCLUDED

#include <cstring>
#include <mutex>
#include <thread>
#include "framepac/atomic.h"
#include "framepac/init.h"
#include "framepac/objectvmt.h"

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

namespace FramepaC
{
constexpr unsigned LOCAL_SLABCACHE_LOWWATER = 16 ;
constexpr unsigned LOCAL_SLABCACHE_HIGHWATER = 32 ;
} // end namespace FramepaC

/************************************************************************/
/************************************************************************/

// private classes, types, and functions
namespace FramepaC
{

//----------------------------------------------------------------------------

typedef uint16_t alloc_size_t ;

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
         {
	 return reinterpret_cast<SlabGroup*>(const_cast<Slab*>(this) - m_info.m_slab_id) ;
	 }

      static constexpr Slab* slab(const void* block)
	 {
	 return (Slab*)(((uintptr_t)block)&~(SLAB_SIZE-1)) ;
	 }
      static constexpr alloc_size_t slabOffset(void* block)
	 {
	 return (alloc_size_t)(((uintptr_t)block) & (SLAB_SIZE-1)) ;
	 }
      static constexpr const ObjectVMT* VMT(const void* block)
         {
	 // convert the pointer to the allocated block into a pointer to the slab, then index off that
	 return slab(block)->m_info.m_vmt ;
	 }

      void pushSlab(Slab*& listhead) ;
      void unlinkSlab()
	 {
         Slab** prev = prevSlabPtr() ;
	 if (!prev) return  ;		// already unlinked?
	 Slab* next = nextSlab() ;
	 (*prev) = next ;
	 if (next) next->setPrevSlabPtr(prev) ;
	 setPrevSlabPtr(nullptr) ;
	 }
      void pushFreeSlab(Slab*& listhead)
	 {
	 this->m_info.m_nextfree = listhead ;
	 listhead = this ;
	 }
      void setNextSlab(Slab* next) { m_info.m_nextslab = next ; }
      void setPrevSlabPtr(Slab** prev) { m_info.m_prevslab = prev ; }
      void setNextFreeSlab(Slab* next) { m_info.m_nextfree = next ; }
      void setNextForeignFree(Slab* next) { m_footer.setFreeSlabList(next) ; }//FIXME?
      void setVMT(const ObjectVMT* vmt) { m_info.m_vmt = vmt ; }
      void setSlabID(unsigned id) { m_info.m_slab_id = id ; }
      void setOwningAllocator(uint16_t alloc_idx) { m_info.m_alloc_index = alloc_idx ; }
      void clearOwner() ;
      alloc_size_t makeFreeList(unsigned objsize, unsigned align) ;
      void* initFreelist(unsigned objsize, unsigned align) ;
      void* bufferStart() { return m_buffer ; }
      const void* bufferStart() const { return m_buffer ; }
      static unsigned bufferSize() { return sizeof(m_buffer) ; }

      // information about this slab
      const ObjectVMT* VMT() const { return m_info.m_vmt ; }
      alloc_size_t freeList() const { return m_header.m_freelist ; }
      Slab* nextSlab() const { return m_info.m_nextslab ; }
      Slab** prevSlabPtr() const { return m_info.m_prevslab ; }
      Slab* nextFreeSlab() const { return m_info.m_nextfree ; }
      Slab* nextForeignFree() const { return m_footer.freeSlabList() ; }
      unsigned owningAllocator() const { return m_info.m_alloc_index ; }
      std::thread::id owningThread() const { return m_info.m_owner ; }
      size_t objectSize() const { return m_info.m_objsize ; }
      size_t objectCount() const { return m_info.m_objcount ; }
      size_t objectsInUse() const
	 { size_t used = m_header.m_usedcount, freed = m_footer.freeCount() ;
	   return used >= freed ? used - freed : 0 ; }
      bool objectsAvailable() const
	 { return m_header.m_usedcount < m_info.m_objcount ; }
      size_t slabOffset() const { return m_info.m_slab_id ; }

      [[gnu::hot]]
      void* allocObject()
	 {
	 //assert(m_header.m_freelist != 0) ;
	 m_header.m_usedcount++ ;
	 void* item = ((char*)this) + m_header.m_freelist ;
	 ASAN(ASAN_UNPOISON_MEMORY_REGION(item,m_info.m_objsize)) ;
	 m_header.m_freelist = *((alloc_size_t*)item) ;
	 return item ;
	 }
      void* reclaimForeignFrees() ;
      void releaseObject(void* obj, Slab*& freelist) ;
   private:
      // group together all the header fields
      // First, all of the fields which are unaffected by allocations/deallocations within this slab
      class alignas(64) SlabInfo   // ensure separate cache line for the SlabInfo and SlabHeader
         {
	 public:
	    const ObjectVMT*   m_vmt ;		// should be first to avoid having to add an offset
	    Slab*              m_nextslab { nullptr };
	    Slab**             m_prevslab { nullptr };
	    Slab*              m_nextfree { nullptr }; // next slab containing unallocated objects
	    Fr::Atomic<Slab*>* m_foreignlist { nullptr }; // pointer to thread-local list of foreign frees
	    std::thread::id    m_owner ;	// the thread that initially allocated this slab
	    alloc_size_t       m_objsize ;	// bytes per object
	    uint16_t           m_objcount ;	// number of objects in this slab
	    uint16_t           m_slab_id ; 	// index within SlabGroup
	    uint16_t	       m_alloc_index ;	// the Allocator to which this slab "belongs"
	    uint16_t           m_nextfree_id ;	// index of next free slab within the group
         } ;
      // Next, the fields that only the owning thread modifies.  These do not require any synchronization.
      class SlabHeader
         {
	 public:
	    alloc_size_t     m_freelist { 0 } ;	// first free object, or 0
	    uint16_t         m_usedcount { 0 } ;// number of objects being used; release Slab when it reaches 0
	 public:
	    //SlabHeader() = default ;
	    ~SlabHeader() = default ;
         } ;
      // group together all the footer fields, so that we can use a single sizeof() in computing the footer size
      // the footer contains all of the fields that may be modified by any thread and thus require atomic
      //   accesses
      class SlabFooter
         {
	 public:
	    SlabFooter() { m_ptr_count.store(0) ; }
	    ~SlabFooter() {}
	    void link(void* obj)
	       {
	       uint32_t ofs = slabOffset(obj) ;
	       uint32_t expected = m_ptr_count ;
	       uint32_t new_value ;
	       do {
	          *((alloc_size_t*)obj) = (alloc_size_t)expected ;
	          new_value = ((expected & 0xFFFF0000) + 0x10000) | ofs ;
	          } while (!m_ptr_count.compare_exchange_weak(expected,new_value)) ;
	       }
	    std::pair<alloc_size_t,uint16_t> grabList()
	       {
	       // grab the entire freelist by atomically swapping it with zero
	       uint32_t val = m_ptr_count.exchange(0) ;
	       // split up the retrieved result into pointer and count
	       return std::pair<alloc_size_t,uint16_t>(val&0xFFFF,val>>16) ;
	       }
	    size_t freeCount() const { return m_ptr_count.load() >> 16 ; }
	    Slab* freeSlabList() const { return m_freelist ; }
	    void setFreeSlabList(Slab* slb) { m_freelist = slb ; }
      	 protected:
	    Fr::Atomic<uint32_t> m_ptr_count { 0 } ;
	    Fr::Atomic<Slab*>    m_freelist { nullptr }; // next slab containing unallocated objects
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

class SlabGroupQueue ;

class SlabGroup
   {
   public:
      SlabGroup() ;
      SlabGroup(const SlabGroup&) = delete ;
      ~SlabGroup() { }
      void operator= (const SlabGroup&) = delete ;

      static Slab* allocateSlab() ;
      static void releaseSlab(Slab* slab) ;
      static void reclaim() ; // release any unused groups back to the OS
      
      void _delete() { delete this ; }

      size_t freeSlabs() const { return m_numfree ; }

   private:
      static SlabGroupQueue s_freequeue ;
   private:
      Slab                 m_slabs[SLAB_GROUP_SIZE] ;
      Fr::Atomic<Slab*>    m_freeslabs { nullptr } ;
      Fr::Atomic<unsigned> m_numfree { lengthof(m_slabs) } ;
   protected:
      void* operator new(size_t sz) ;
      void operator delete(void* grp) ;
   protected:
      static Slab* popFreeSlab() ;
   } ;

} // end namespace FramepaC

/************************************************************************/

namespace FramepaC
{
constexpr unsigned MAX_ALLOCATOR_TYPES = 500 ;
} // end namespace FramepaC


// public classes, types, and functions

namespace Fr
{

//----------------------------------------------------------------------------

class Allocator
   {
   protected:
      typedef FramepaC::Slab Slab ;
      typedef Fr::ThreadInitializer<Allocator> ThreadInitializer ;
   public:
      class TLS
	 {
	 public:
	    FramepaC::Slab* m_allocslabs ;
	    FramepaC::Slab* m_freelist ;
	    FramepaC::Slab* m_foreignfree ;
	 } ;
      class SharedInfo
	 {
	 public:
	    SharedInfo() = default ;
	    SharedInfo(const FramepaC::ObjectVMT* vmt, FramepaC::alloc_size_t objsize)
	       : m_vmt(vmt), m_objsize(objsize)
	       {
		  if (objsize < alignof(double)) m_alignment = objsize ;
	       }
	    SharedInfo(const FramepaC::ObjectVMT* vmt, FramepaC::alloc_size_t objsize,
	       unsigned alignment)
	       : m_vmt(vmt), m_objsize(objsize), m_alignment(alignment)
	       {
	       }
	    ~SharedInfo() {}
	 public: // data members
	    const FramepaC::ObjectVMT*  m_vmt ;
	    Fr::Atomic<FramepaC::Slab*> m_orphans { nullptr } ;  // slabs which used to belong to terminated threads
	    FramepaC::alloc_size_t      m_objsize ;
	    unsigned		        m_alignment { alignof(double) } ;
	 } ;
      
   public:
      Allocator(const FramepaC::ObjectVMT* vmt, unsigned objsize) ;
      Allocator(const FramepaC::ObjectVMT* vmt, unsigned objsize, unsigned align) ;
      ~Allocator() {}
      // allocators are not copyable
      Allocator(const Allocator&) = delete ;
      void operator= (const Allocator&) = delete ;

      size_t objectSize() const { return s_shared[m_type].m_objsize ; }

      [[gnu::hot]] [[gnu::malloc]]
      void* allocate()
	 {
	 Slab* slb = s_tls[m_type].m_freelist ;
	 if (!slb) return allocate_more() ;
	 void* item = slb->allocObject() ;
	 if (!slb->freeList()) popFreelist() ;
	 return item ;
	 }
      [[gnu::hot]]
      void release(void* blk)
	 {
	 if (blk)
	    {
	    Slab* slb = Slab::slab(blk) ;
	    slb->releaseObject(blk,s_tls[m_type].m_freelist) ;
	    ASAN(ASAN_POISON_MEMORY_REGION(blk,s_shared[m_type].m_objsize)) ;
	    }
	 }
      [[gnu::hot]]
      static void free(void* blk)
	 {
	 if (blk)
	    {
	    Slab* slb = Slab::slab(blk) ;
	    slb->releaseObject(blk,s_tls[slb->owningAllocator()].m_freelist) ;
	    }
	 }

      static void releaseSlab(FramepaC::Slab* slb) ;
      void reclaim(bool keep_one = false) ;
      static void gc() ;

      static void threadCleanup() ;

   protected:
      void* allocate_more() ;
      void popFreelist() ;
      static void reclaim(uint16_t alloc_id, bool keep_one) ;

   protected: // data members
      unsigned		       m_type { 0 } ;
      static SharedInfo        s_shared[FramepaC::MAX_ALLOCATOR_TYPES] ;
public:
      static thread_local TLS  s_tls[FramepaC::MAX_ALLOCATOR_TYPES] ;
      
      // each thread maintains a small pool of available Slabs for use by any instantiation of Allocator to reduce
      //   the frequency with which the global slab pool needs to be accessed (which could incur contention).  If we
      //   need another Slab while the pool is empty, we allocate N Slabs from the global pool; if releasing a Slab
      //   causes the pool to grow to 2N, we return N Slabs back to the global pool.
      static thread_local Slab*    s_local_free_slabs ;
      static thread_local unsigned s_local_free_count ;
      
      static ThreadInitializer     s_threadinit ;
   } ;

//----------------------------------------------------------------------------
//  a class to wrap various instantiations of Allocator on NonObject for allocating
//    fixed-size chunks of uninitialized memory

class SmallAlloc
   {
   public:
      static SmallAlloc* create(size_t objsize) ;
      static SmallAlloc* create(size_t objsize, size_t align) ;

#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
      // when sanitizing, just forward to the system allocator so that the sanitizer can track allocations
      void* allocate() { return ::new char[m_allocator->objectSize()] ; }
      void release(void* blk) { ::delete[] reinterpret_cast<char*>(blk) ; }
#else
      void* allocate() { return m_allocator->allocate() ; }
      void release(void* blk) { m_allocator->release(blk) ; }
#endif /* __SANITIZE_ADDRESS__ || __SANITIZE_THREAD__ */

      void reclaim() { m_allocator->reclaim() ; }

   private:
      SmallAlloc(Allocator* alloc) { m_allocator = alloc ; }
      ~SmallAlloc() { delete m_allocator ; }

   protected:
      Allocator* m_allocator ;
   } ;

//----------------------------------------------------------------------------

template <typename T, size_t SZ = 512>
class LocalAlloc
   {
   public:
      LocalAlloc(size_t alloc)
	 {
	 m_buffer = (alloc > SZ) ? new T[alloc] : m_localbuffer ;
	 }
      LocalAlloc(size_t alloc, bool clear)
	 {
	 m_buffer = (alloc > SZ) ? new T[alloc] : m_localbuffer ;
	 if (m_buffer && clear)
	    std::memset(m_buffer,'\0',sizeof(T)*alloc) ;
	 }
      LocalAlloc(T* buf)
	 {
	 m_buffer = buf ? buf : m_localbuffer ;
	 }
      ~LocalAlloc()
	 {
	 if (m_buffer != m_localbuffer) delete[] m_buffer ;
	 }

      T* operator& () const { return m_buffer ; }
      T* base() const { return m_buffer ; }
      T& operator [] (size_t idx) { return m_buffer[idx] ; }
      const T& operator [] (size_t idx) const { return m_buffer[idx] ; }
      operator T* () const { return m_buffer ; }

   private:
      T  m_localbuffer[SZ] ;
      T* m_buffer ;
   } ;

//----------------------------------------------------------------------------

template <typename T>
class ScopedPtr
   {
   public:
      typedef T* pointer ;
      typedef T element_type ;
   public:
      ScopedPtr(unsigned N) { m_item = new T[N] ; }
      ScopedPtr(unsigned N, const T* s, unsigned copyN)
	 { m_item = new T[N] ; std::copy(s,s+copyN,m_item) ; }
      ScopedPtr(T* item = nullptr) { m_item = item ; }
      ScopedPtr(const ScopedPtr&) = delete ;
      ScopedPtr(ScopedPtr&& orig) { m_item = orig.move() ; }
      ~ScopedPtr() { reset(nullptr) ; }
      ScopedPtr& operator= (const ScopedPtr&) = delete ;
      ScopedPtr& operator= (ScopedPtr&& orig) { reset(orig.release()) ; return *this ; }
      ScopedPtr& operator= (T* new_item) { reset(new_item) ; return *this ; }

      T* get() const noexcept { return m_item ; }
      T* move() { T* s = m_item ; clear() ; return s ; }
      void clear() { m_item = nullptr ; }
      void reset(T* ptr) { T* old = m_item ; m_item = ptr ; delete old ; }
      T* release() { T* ptr = m_item ; clear() ; return ptr ; }

      T* operator-> () { return m_item ; }
      const T* operator-> () const { return m_item ; }
      T* operator* () { return m_item ; }
      const T* operator* () const { return m_item ; }
      explicit operator T* () const { return m_item ; }
      operator const T* () const { return m_item ; }
      T& operator[] (size_t N) { return m_item[N] ; }
      const T& operator[] (size_t N) const { return m_item[N] ; }
      explicit operator bool () const { return m_item != nullptr ; }
      bool operator ! () const { return m_item == nullptr ; }
   protected:
      T* m_item ;
   } ;

//----------------------------------------------------------------------------

// perform memory reclamation on all memory pools
void gc() ;

// print memory usage statistics
void memory_stats(std::ostream&) ;

} ; // end namespace Fr

#endif /* !__Fr_MEMORY_H_INCLUDED */

// end of file memory.h //
