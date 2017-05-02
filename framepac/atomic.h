/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.01, last edit 2017-05-01					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/*  File atomic.h		atomic operations on simple variables	*/
/*									*/
/*  (c) Copyright 2015,2016,2017 Carnegie Mellon University		*/
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

#ifndef _Fr_ATOMIC_H_INCLUDED
#define _Fr_ATOMIC_H_INCLUDED

#include <atomic>
#include <type_traits>
#include "framepac/config.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

#ifdef FrSINGLE_THREADED

class atom_flag
   {
   private:
      bool f ;
   public:
      atom_flag() { }
      atom_flag( const atom_flag& ) = delete ;

      // atomic flags are not assignable
      atom_flag& operator= (const atomic_flag& ) = delete ;
      atom_flag& operator= (const atomic_flag& ) volatile = delete ;

      void clear() { f = false ; }
      void clear() volatile { f = false ; }

      bool test_and_set() { bool old = f ; f = true ; return old ; }
      bool test_and_set() volatile { bool old = f ; f = true ; return old ; }
   } ;

//----------------------------------------------------------------------------

template <typename T>
class Atomic
   {
   private:
      T v ;
   public:
      Atomic() {}
      Atomic(T value) { v = value ; }
      ~Atomic() {}

      T operator= (T newval) { v = newval ; return newval ; }
      T operator= (T newval) volatile { v = newval ; return newval ; }
      Atomic& operator= (const Atomic& ) = delete ;
      Atomic& operator= (const Atomic& ) volatile = delete ;

      static bool is_lock_free() const { return true ; }
      static constexpr bool is_always_lock_free = true ;

      void store( T desired, std::memory_order = std::memory_order_seq_cst ) { v = desired ; }
      void store( T desired, std::memory_order = std::memory_order_seq_cst ) volatile { v = desired ; }
      
      T load( std::memory_order = std::memory_order_seq_cst ) const { return v ; }
      T load( std::memory_order = std::memory_order_seq_cst ) const volatile { return v ; }
      T operator T () const { return v ; }
      T operator T () const volatile { return v ; }

      T exchange( T desired, std::memory_order = std::memory_order_seq_cst )
	 { T old = v ; v = desired ; return old ; }
      T exchange( T desired, std::memory_order = std::memory_order_seq_cst ) volatile
	 { T old = v ; v = desired ; return old ; }

      bool compare_exchange_weak( T& /*expected*/, T desired,
				  std::memory_order = std::memory_order_seq_cst )
	 { v = desired ; return true ; }
      bool compare_exchange_weak( T& /*expected*/, T desired,
				  std::memory_order = std::memory_order_seq_cst ) volatile
	 { v = desired ; return true ; }
      bool compare_exchange_strong( T& /*expected*/, T desired,
				    std::memory_order = std::memory_order_seq_cst )
	 { v = desired ; return true ; }
      bool compare_exchange_strong( T& /*expected*/, T desired,
				    std::memory_order = std::memory_order_seq_cst ) volatile
	 { v = desired ; return true ; }

      T fetch_add( T incr, std::memory_order = std::memory_order_seq_cst )
	 { T old = v ; v += incr ; return old ; }
      T fetch_add( T incr, std::memory_order = std::memory_order_seq_cst ) volatile
	 { T old = v ; v += incr ; return old ; }
      T fetch_sub( T incr, std::memory_order = std::memory_order_seq_cst )
	 { T old = v ; v -= incr ; return old ; }
      T fetch_sub( T incr, std::memory_order = std::memory_order_seq_cst ) volatile
	 { T old = v ; v -= incr ; return old ; }
      T fetch_and( T incr, std::memory_order = std::memory_order_seq_cst )
	 { T old = v ; v &= incr ; return old ; }
      T fetch_and( T incr, std::memory_order = std::memory_order_seq_cst ) volatile
	 { T old = v ; v &= incr ; return old ; }
      T fetch_or( T incr, std::memory_order = std::memory_order_seq_cst )
	 { T old = v ; v |= incr ; return old ; }
      T fetch_or( T incr, std::memory_order = std::memory_order_seq_cst ) volatile
	 { T old = v ; v |= incr ; return old ; }
      T fetch_xor( T incr, std::memory_order = std::memory_order_seq_cst )
	 { T old = v ; v ^= incr ; return old ; }
      T fetch_xor( T incr, std::memory_order = std::memory_order_seq_cst ) volatile
	 { T old = v ; v ^= incr ; return old ; }

      T operator++ () { return ++v ; }
      T operator++ () volatile { return ++v ; }
      T operator++ (int) { return v++ ; }
      T operator++ (int) volatile { return v++ ; }
      T operator-- () { return --v ; }
      T operator-- () volatile { return --v ; }
      T operator-- (int) { return v-- ; }
      T operator-- (int) volatile { return v-- ; }

      T operator+= ( T incr ) { v += incr ; return v ; }
      T operator+= ( T incr ) volatile { v += incr ; return v ; }
      T operator-= ( T incr ) { v -= incr ; return v ; }
      T operator-= ( T incr ) volatile { v -= incr ; return v ; }
      T operator&= ( T incr ) { v &= incr ; return v ; }
      T operator&= ( T incr ) volatile { v &= incr ; return v ; }
      T operator|= ( T incr ) { v |= incr ; return v ; }
      T operator|= ( T incr ) volatile { v |= incr ; return v ; }
      T operator^= ( T incr ) { v ^= incr ; return v ; }
      T operator^= ( T incr ) volatile { v ^= incr ; return v ; }

      // additional operations not in C++ standard library
      bool test_and_set_bit( unsigned bitnum )
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_clear = (v & mask) == 0 ;
	 v |= mask ;
	 return was_clear ;
	 }
      bool test_and_set_bit( unsigned bitnum ) volatile
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_clear = (v & mask) == 0 ;
	 v |= mask ;
	 return was_clear ;
	 }
      bool test_and_clear_bit( unsigned bitnum )
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_set = (v & mask) != 0 ;
	 v &= ~mask ;
	 return was_set ;
	 }
      bool test_and_clear_bit( unsigned bitnum ) volatile
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_set = (v & mask) != 0 ;
	 v &= ~mask ;
	 return was_set ;
	 }
      T test_and_set_mask( T bitmask )
	 {
	 T prev_val = v ;
	 v |= bitmask ;
	 return prev_val & bitmask ;
	 }
      T test_and_set_mask( T bitmask ) volatile
	 {
	 T prev_val = v ;
	 v |= bitmask ;
	 return prev_val & bitmask ;
	 }
      T test_and_clear_mask( T bitmask )
	 {
	 T prev_val = v ;
	 v &= ~bitmask ;
	 return prev_val & bitmask ;
	 }
      T test_and_clear_mask( T bitmask ) volatile
	 {
	 T prev_val = v ;
	 v &= ~bitmask ;
	 return prev_val & bitmask ;
	 }

      // generic functionality built on top of the atomic primitives
      //  (only available in pointer specializations)
      void push(T node) ;
      void push(T nodes, T tail) ;
      T pop() ;

      static Atomic<T> ref(T& obj) { return reinterpret_cast<Atomic<T> >(obj) ; }
   } ;

// memory barriers
ALWAYS_INLINE void atomic_thread_fence( std::memory_order )
{
#ifdef __GNUC__
   asm volatile ("" : : : "memory") ;
#endif
}

// various kinds of barriers
ALWAYS_INLINE void memoryBarrier() {}
ALWAYS_INLINE void loadBarrier() {}
ALWAYS_INLINE void storeBarrier() {}
ALWAYS_INLINE void barrier()
{
#ifdef __GNUC__
   asm volatile ("" : : : "memory") ;
#endif
}

// generic functionality built on top of the atomic primitives

template <typename T>
void Atomic<T*>::push(T* node)
{
   node->next(var.load()) ;
   store(node) ;
}

template <typename T>
void Atomic<T*>::push(T* nodes, T* tail)
{
   tail->next(load()) ;
   store(nodes) ;
}

template <typename T>
T* Atomic<T*>::pop()
{
   T* node = load() ;
   if (node)
      store(node->next()) ;
   return node ;
}

template <>
inline NullObject Atomic<NullObject>::exchange(class NullObject v,
					       std::memory_order) { return *this; }

#else // multi-threaded version

typedef std::atomic_flag atom_flag ;

template <typename T>
class Atomic : public std::atomic<T>
   {
   private:
      // no data members
   public:
      Atomic() : std::atomic<T>()
	 {}
      Atomic(T value) : std::atomic<T>(value)
	 {}
      Atomic(const Atomic<T>& value) : std::atomic<T>(value.load()) {}
      ~Atomic() = default ;

      Atomic& operator= (const Atomic& value) { this->store(value.load()) ; return *this ; }
      Atomic& operator= (const T& value) { this->store(value) ; return *this ; }
      T operator+= (const T& value) ;
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      operator+= (size_t value) ;

      // additional operations
      bool test_and_set_bit( unsigned bitnum )
	 {
	 T mask = (T)(1L << bitnum) ;
	 return test_and_set_mask(mask) == 0 ;
	 }
      bool test_and_set_bit( unsigned bitnum ) volatile
	 {
	 T mask = (T)(1L << bitnum) ;
	 return test_and_set_mask(mask) == 0 ;
	 }
      bool test_and_clear_bit( unsigned bitnum )
	 {
	 T mask = (T)(1L << bitnum) ;
	 return test_and_clear_mask(mask) != 0 ;
	 }
      bool test_and_clear_bit( unsigned bitnum ) volatile
	 {
	 T mask = (T)(1L << bitnum) ;
	 return test_and_clear_mask(mask) != 0 ;
	 }
      T test_and_set_mask( T bitmask )
	 {
	 return std::atomic<T>::fetch_or(bitmask) & bitmask ;
	 }
      T test_and_set_mask( T bitmask ) volatile
	 {
	 return std::atomic<T>::fetch_or(bitmask) & bitmask ;
	 }
      T test_and_clear_mask( T bitmask )
	 {
	 return std::atomic<T>::fetch_and(~bitmask) & bitmask ;
	 }
      T test_and_clear_mask( T bitmask ) volatile
	 {
	 return std::atomic<T>::fetch_and(~bitmask) & bitmask ;
	 }

      // generic functionality built on top of the atomic primitives
      //  (only available in pointer specializations)
      void push(T node) ;
      void push(T nodes, T tail) ;
      T pop() ;

      static Atomic<T>& ref(T& obj) { return reinterpret_cast<Atomic<T>&>(obj) ; }
   } ;

// bool doesn't support fetch_or or fetch_and, so we need specific specialization
template <>
inline bool Atomic<bool>::test_and_set_mask(bool mask)
{
   return mask ? exchange(mask) : false ;
}

template <>
inline bool Atomic<bool>::test_and_clear_mask(bool mask)
{
   return mask ? exchange(false) : false ;
}

// NullObject requires special handling
template <>
inline Atomic<NullObject>& Atomic<NullObject>::operator= (const NullObject&)
{
   return *this ;
}

template <>
inline NullObject Atomic<NullObject>::operator+= (const NullObject&)
{
   NullObject n ;
   return n ;
}

template <typename T>
inline T Atomic<T>::operator+= (const T& value)
{
   return this->fetch_add(value) + value ; 
}

template <typename T>
template <typename RetT>
inline typename std::enable_if<std::is_pointer<T>::value,RetT>::type
Atomic<T>::operator+= (size_t value)
{
   return this->fetch_add(value) + value ; 
}

// generic functionality built on top of the atomic primitives

template <typename T>
inline void Atomic<T>::push(T node)
{
   T list ;
   do {
      list = this->load() ;
      node.next(list) ;
      } while (!this->compare_exchange_weak(list,node)) ;
}

template <typename T>
inline void Atomic<T>::push(T nodes, T tail)
{
   T list ;
   do {
      list = this->load() ;
      tail.next(list) ;
      } while (!this->compare_exchange_weak(list,nodes)) ;
}

template <typename T>
inline T Atomic<T>::pop()
{
   T head ;
   T rest ;
   do {
      head = this->load() ;
      if (!head)
	 return head ;
      rest = head.next() ;
      } while (!this->compare_exchange_weak(head,rest)) ;
   head.next(nullptr) ;
   return head ;
}

// various kinds of barriers
ALWAYS_INLINE void memoryBarrier() 
{
   atomic_thread_fence(std::memory_order_seq_cst) ; 
}
ALWAYS_INLINE void loadBarrier()
{
   atomic_thread_fence(std::memory_order_acquire) ;
}
ALWAYS_INLINE void storeBarrier()
{ 
   atomic_thread_fence(std::memory_order_release) ;
}
ALWAYS_INLINE void barrier()
{
   atomic_thread_fence(std::memory_order_relaxed) ;/*FIXME*/ 
#ifdef __GNUC__
   asm volatile ("" : : : "memory") ;
#endif
}

#ifdef ERROR
template <> template <>
inline Fr::NullObject std::atomic<Fr::NullObject>::exchange(Fr::NullObject v,
							    std::memory_order) { return *this; }
#endif

#endif /* FrSINGLE_THREADED */

typedef Atomic<bool> atom_bool ;
typedef Atomic<char> atom_char ;
typedef Atomic<signed char> atom_schar ;
typedef Atomic<unsigned char> atom_uchar ;
typedef Atomic<short> atom_short ;
typedef Atomic<unsigned short> atom_ushort ;
typedef Atomic<int> atom_int ;
typedef Atomic<unsigned int> atom_uint ;
typedef Atomic<long> atom_long ;
typedef Atomic<unsigned long> atom_ulong ;
typedef Atomic<long long> atom_llong ;
typedef Atomic<unsigned long long> atom_ullong ;
typedef Atomic<char16_t> atom_char16_t ;
typedef Atomic<char32_t> atom_char32_t ;
typedef Atomic<wchar_t> atom_wchar_t ;
typedef Atomic<uint16_t> atom_uint16_t ;
typedef Atomic<uint32_t> atom_uint32_t ;
typedef Atomic<uint64_t> atom_uint64_t ;
typedef Atomic<size_t> atom_size_t ;

} // end of namespace Fr //

/************************************************************************/
/************************************************************************/

namespace Fr
{

class HazardPointer
   {
   public:
      HazardPointer() : m_pointer(nullptr) {}
      HazardPointer(const HazardPointer &) = delete ;
      ~HazardPointer() = default ;
      void operator= (const HazardPointer&) = delete ;

      void *pointer() const { return  m_pointer ; }

      void setHazard(void *hp) { m_pointer = hp ; }
      void clearHazard() { m_pointer = nullptr ; }
   private:
      void *m_pointer ;

   } ;

//----------------------------------------------------------------------------

class HazardPointerList
   {
   public:
      HazardPointerList() {}
      HazardPointerList(const HazardPointerList&) = delete ;
      ~HazardPointerList() {}

      HazardPointerList& operator= (const HazardPointerList&) = delete ;

      bool registerPointer(void *) ;
      bool unregisterPointer(void *) ;

      bool hasHazard(void *object) const ;

   protected: // data
      HazardPointerList *m_next { nullptr } ;
      void *m_pointers[7] { nullptr } ;

   protected: // methods
   } ;


} // end namespace Fr

#endif /* !__Fr_ATOMIC_H_INCLUDED */

// end of file atomic.h //
