/****************************** -*- C++ -*- *****************************/
/*									*/
/*  FramepaC-ng  -- frame manipulation in C++				*/
/*  Version 0.01, last edit 2017-05-07					*/
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
   public:
      Atomic() {}
      Atomic(T value) { v = value ; }
      ~Atomic() {}

      T& operator= (T& newval) noexcept { v = newval ; return newval ; }
      T& operator= (T& newval) volatile noexcept { v = newval ; return newval ; }
      const T& operator= (const T& newval) noexcept { v = newval ; return newval ; }
      const T& operator= (const T& newval) volatile noexcept { v = newval ; return newval ; }

      static bool is_lock_free() const noexcept { return true ; }
      static constexpr bool is_always_lock_free = true ;

      void store(const T& desired, std::memory_order = std::memory_order_seq_cst ) noexcept { v = desired ; }
      void store(const T& desired, std::memory_order = std::memory_order_seq_cst ) volatile noexcept { v = desired ; }
      void store_relax(const T& desired) noexcept { v = desired ; }
      void store_relax(const T& desired) volatile noexcept { v = desired ; }

      T load( std::memory_order = std::memory_order_seq_cst ) const noexcept { return v ; }
      T load( std::memory_order = std::memory_order_seq_cst ) const volatile noexcept { return v ; }
      T load_relax() const noexcept { return v ; }
      T load_relax() const volatile noexcept { return v ; }

      operator T () const noexcept { return v ; }
      operator T () const volatile noexcept { return v ; }

      T exchange(const T desired, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v = desired ; return old ; }
      T exchange(const T desired, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v = desired ; return old ; }
      T exchange_relax(const T desired) noexcept { T old = v ; v = desired ; return old ; }
      T exchange_relax(const T desired) volatile noexcept { T old = v ; v = desired ; return old ; }

      bool compare_exchange_weak( T& /*expected*/, T desired,
				  std::memory_order = std::memory_order_seq_cst ) noexcept
	 { v = desired ; return true ; }
      bool compare_exchange_weak( T& /*expected*/, T desired,
				  std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { v = desired ; return true ; }
      bool compare_exchange_strong( T& /*expected*/, T desired,
				    std::memory_order = std::memory_order_seq_cst ) noexcept
	 { v = desired ; return true ; }
      bool compare_exchange_strong( T& /*expected*/, T desired,
				    std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { v = desired ; return true ; }

      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_add( T incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v += incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_add( T incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v += incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v += incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v += incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_add_relax( T incr) noexcept
	 { T old = v ; v += incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_add_relax( T incr) volatile noexcept
	 { T old = v ; v += incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add_relax(std::ptrdiff_t incr) noexcept
	 { T old = v ; v += incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add_relax(std::ptrdiff_t incr) volatile noexcept
	 { T old = v ; v += incr ; return old ; }

      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_sub( T incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v -= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_sub( T incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v -= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v -= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v -= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_sub_relax( T incr) noexcept
	 { T old = v ; v -= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_sub_relax( T incr) volatile noexcept
	 { T old = v ; v -= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub_relax(std::ptrdiff_t incr) noexcept
	 { T old = v ; v -= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub_relax(std::ptrdiff_t incr) volatile noexcept
	 { T old = v ; v -= incr ; return old ; }

      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_and( T incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v &= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_and( T incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v &= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_and(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v &= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_and(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v &= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_and_relax( T incr) noexcept
	 { T old = v ; v &= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_and_relax( T incr) volatile noexcept
	 { T old = v ; v &= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_and_relax(std::ptrdiff_t incr) noexcept
	 { T old = v ; v &= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_and_relax(std::ptrdiff_t incr) volatile noexcept
	 { T old = v ; v &= incr ; return old ; }

      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_or( T incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v |= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_or( T incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v |= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_or(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v |= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_or(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v |= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_or_relax( T incr) noexcept
	 { T old = v ; v |= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_or_relax( T incr) volatile noexcept
	 { T old = v ; v |= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_or_relax(std::ptrdiff_t incr) noexcept
	 { T old = v ; v |= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_or_relax(std::ptrdiff_t incr) volatile noexcept
	 { T old = v ; v |= incr ; return old ; }

      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_xor( T incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v ^= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_xor( T incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v ^= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_xor(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) noexcept
	 { T old = v ; v ^= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_xor(std::ptrdiff_t incr, std::memory_order = std::memory_order_seq_cst ) volatile noexcept
	 { T old = v ; v ^= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_xor_relax( T incr) noexcept
	 { T old = v ; v ^= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_xor_relax( T incr) volatile noexcept
	 { T old = v ; v ^= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_xor_relax(std::ptrdiff_t incr) noexcept
	 { T old = v ; v ^= incr ; return old ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_xor_relax(std::ptrdiff_t incr) volatile noexcept
	 { T old = v ; v ^= incr ; return old ; }

      T operator++ () noexcept { return ++v ; }
      T operator++ () volatile noexcept { return ++v ; }
      T operator++ (int) noexcept { return v++ ; }
      T operator++ (int) volatile noexcept { return v++ ; }
      T operator-- () noexcept { return --v ; }
      T operator-- () volatile noexcept { return --v ; }
      T operator-- (int) noexcept { return v-- ; }
      T operator-- (int) volatile noexcept { return v-- ; }

      T operator+= ( T incr ) noexcept { v += incr ; return v ; }
      T operator+= ( T incr ) volatile noexcept { v += incr ; return v ; }
      T operator-= ( T incr ) noexcept { v -= incr ; return v ; }
      T operator-= ( T incr ) volatile noexcept { v -= incr ; return v ; }
      T operator&= ( T incr ) noexcept { v &= incr ; return v ; }
      T operator&= ( T incr ) volatile noexcept { v &= incr ; return v ; }
      T operator|= ( T incr ) noexcept { v |= incr ; return v ; }
      T operator|= ( T incr ) volatile noexcept { v |= incr ; return v ; }
      T operator^= ( T incr ) noexcept { v ^= incr ; return v ; }
      T operator^= ( T incr ) volatile noexcept { v ^= incr ; return v ; }

      // additional operations not in C++ standard library
      bool test_and_set_bit( unsigned bitnum ) noexcept
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_clear = (v & mask) == 0 ;
	 v |= mask ;
	 return was_clear ;
	 }
      bool test_and_set_bit( unsigned bitnum ) volatile noexcept
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_clear = (v & mask) == 0 ;
	 v |= mask ;
	 return was_clear ;
	 }
      bool test_and_clear_bit( unsigned bitnum ) noexcept
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_set = (v & mask) != 0 ;
	 v &= ~mask ;
	 return was_set ;
	 }
      bool test_and_clear_bit( unsigned bitnum ) volatile noexcept
	 {
	 T mask = (T)(1L << bitnum) ;
	 bool was_set = (v & mask) != 0 ;
	 v &= ~mask ;
	 return was_set ;
	 }
      T test_and_set_mask( T bitmask ) noexcept
	 {
	 T prev_val = v ;
	 v |= bitmask ;
	 return prev_val & bitmask ;
	 }
      T test_and_set_mask( T bitmask ) volatile noexcept
	 {
	 T prev_val = v ;
	 v |= bitmask ;
	 return prev_val & bitmask ;
	 }
      T test_and_clear_mask( T bitmask ) noexcept
	 {
	 T prev_val = v ;
	 v &= ~bitmask ;
	 return prev_val & bitmask ;
	 }
      T test_and_clear_mask( T bitmask ) volatile noexcept
	 {
	 T prev_val = v ;
	 v &= ~bitmask ;
	 return prev_val & bitmask ;
	 }

      void decreaseTo(T desired)
	 {
	    T oldval ;
	    do {
	       oldval = load() ;
	       if (oldval <= desired)
		  break ;
	       } while (!compare_exchange_weak(oldval,desired) ;
	 }
      void increaseTo(T desired)
	 {
	    T oldval ;
	    do {
	       oldval = load() ;
	       if (oldval >= desired)
		  break ;
	       } while (!compare_exchange_weak(oldval,desired) ;
	 }

      // generic functionality built on top of the atomic primitives
      //  (only available in pointer specializations)
      template <typename RetT = T>
      void push(typename std::enable_if<std::is_pointer<T>::value,RetT>::type node)
	 {
	    node->next(var.load(std::memory_order_acquire)) ;
	    store(node) ;
	 }

      template <typename RetT = T>
      void push(typename std::enable_if<std::is_pointer<T>::value,RetT>::type nodes, T tail)
	 {
	    tail->next(load(std::memory_order_acquire)) ;
	    store(nodes) ;
	 }

      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      pop()
	 {
	    T node = load(std::memory_order_acquire) ;
	    aif (node)
	       store(node->next()) ;
	    return node ;
	 }

      static Atomic<T> ref(T& obj) { return reinterpret_cast<Atomic<T> >(obj) ; }

   private:
      T v ;
   } ;


template <>
inline NullObject Atomic<NullObject>::exchange(class NullObject v,
					       std::memory_order) { return *this; }

#else // multi-threaded version

typedef std::atomic_flag atom_flag ;

//----------------------------------------------------------------------------

template <typename T>
class Atomic
   {
   public:
      Atomic() {}
      Atomic(T value) : v(value) {}
      Atomic(const Atomic<T>& value) : v(value.load(std::memory_order_acquire)) {}
      ~Atomic() {}

      T& operator= (T& value) noexcept { ref().store(value,std::memory_order_release) ; return value ; }
      T& operator= (T& value) volatile noexcept { ref().store(value,std::memory_order_release) ; return value ; }
      const T& operator= (const T& value) noexcept { ref().store(value,std::memory_order_release) ; return value ; }
      const T& operator= (const T& value) volatile noexcept { ref().store(value,std::memory_order_release) ; return value ; }

      bool is_lock_free() const { return std::atomic<T>::is_lock_free() ; }

      void store(const T value) noexcept { ref().store(value,std::memory_order_release) ; }
      void store(const T value) volatile noexcept { ref().store(value,std::memory_order_release) ; }
      void store(const T value, std::memory_order order) noexcept { ref().store(value,order) ; }
      void store(const T value, std::memory_order order) volatile noexcept { ref().store(value,order) ; }
      void store_relax(const T value) noexcept { ref().store(value,std::memory_order_relaxed) ; }
      void store_relax(const T value) volatile noexcept { ref().store(value,std::memory_order_relaxed) ; }

      T load() const noexcept
	 { std::atomic<T>& atom = ref() ;
	    return atom.load(std::memory_order_acquire) ; }
      T load() const volatile noexcept
	 { std::atomic<T>& atom = ref() ;
	    return atom.load(std::memory_order_acquire) ; }
      T load(std::memory_order order) const noexcept
	 { std::atomic<T>& atom = ref() ;
	    return atom.load(order) ; }
      T load(std::memory_order order) const volatile noexcept
	 { std::atomic<T>& atom = ref() ;
	    return atom.load(atom,order) ; }
      T load_relax() const noexcept
	 { std::atomic<T>& atom = ref() ;
	    return atom.load(atom,std::memory_order_relaxed) ; }
      T load_relax() const volatile noexcept
	 { std::atomic<T>& atom = ref() ;
	    return atom.load(atom,std::memory_order_relaxed) ; }

      operator T () const noexcept { return ref().load(std::memory_order_acquire) ; }
      operator T () const volatile noexcept { return ref().load(std::memory_order_acquire) ; }

      T exchange(const T newvalue) noexcept { return ref().exchange(newvalue) ; }
      T exchange(const T newvalue) volatile noexcept { return ref().exchange(newvalue) ; }
      T exchange(const T newvalue, std::memory_order order) noexcept { return ref().exchange(newvalue,order) ; }
      T exchange(const T newvalue, std::memory_order order) volatile noexcept { return ref().exchange(newvalue,order) ; }
      T exchange_relax(const T newvalue) noexcept { return ref().exchange(newvalue,std::memory_order_relaxed) ; }
      T exchange_relax(const T newvalue) volatile noexcept { return ref().exchange(newvalue,std::memory_order_relaxed) ; }

      bool compare_exchange_weak(T& expected, T desired) noexcept
	 { return ref().compare_exchange_weak(expected,desired) ; }
      bool compare_exchange_weak(T& expected, T desired) volatile noexcept
	 { return ref().compare_exchange_weak(expected,desired) ; }
      bool compare_exchange_weak(T& expected, T desired, std::memory_order order) noexcept
	 { return ref().compare_exchange_weak(expected,desired,order) ; }
      bool compare_exchange_weak(T& expected, T desired, std::memory_order order) volatile noexcept
	 { return ref().compare_exchange_weak(expected,desired,order) ; }

      bool compare_exchange_strong(T& expected, T desired) noexcept
	 { return ref().compare_exchange_strong(expected,desired) ; }
      bool compare_exchange_strong(T& expected, T desired) volatile noexcept
	 { return ref().compare_exchange_strong(expected,desired) ; }
      bool compare_exchange_strong(T& expected, T desired, std::memory_order order) noexcept
	 { return ref().compare_exchange_strong(expected,desired,order) ; }
      bool compare_exchange_strong(T& expected, T desired, std::memory_order order) volatile noexcept
	 { return ref().compare_exchange_strong(expected,desired,order) ; }

      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_add(T arg) noexcept { return ref().fetch_add(arg) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      fetch_add(T arg) volatile noexcept { return ref().fetch_add(arg) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add(std::ptrdiff_t arg) noexcept { return ref().fetch_add(arg) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add(std::ptrdiff_t arg) volatile noexcept { return ref().fetch_add(arg) ; }
      T fetch_add_relax(T arg) noexcept { return ref().fetch_add(arg,std::memory_order_relaxed) ; }
      T fetch_add_relax(T arg) volatile noexcept { return ref().fetch_add(arg,std::memory_order_relaxed) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add_relax(std::ptrdiff_t arg) noexcept { return ref().fetch_add(arg,std::memory_order_relaxed) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_add_relax(std::ptrdiff_t arg) volatile noexcept { return ref().fetch_add(arg,std::memory_order_relaxed) ; }

      T fetch_sub(T arg) noexcept { return ref().fetch_sub(arg) ; }
      T fetch_sub(T arg) volatile noexcept { return ref().fetch_sub(arg) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub(std::ptrdiff_t arg) noexcept { return ref().fetch_sub(arg) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub(std::ptrdiff_t arg) volatile noexcept { return ref().fetch_sub(arg) ; }
      T fetch_sub_relax(T arg) noexcept { return ref().fetch_sub(arg,std::memory_order_relaxed) ; }
      T fetch_sub_relax(T arg) volatile noexcept { return ref().fetch_sub(arg,std::memory_order_relaxed) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub_relax(std::ptrdiff_t arg) noexcept { return ref().fetch_sub(arg,std::memory_order_relaxed) ; }
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      fetch_sub_relax(std::ptrdiff_t arg) volatile noexcept { return ref().fetch_sub(arg,std::memory_order_relaxed) ; }

      T fetch_and(T arg) noexcept { return ref().fetch_and(arg) ; }
      T fetch_and(T arg) volatile noexcept { return ref().fetch_and(arg) ; }
      T fetch_and_relax(T arg) noexcept { return ref().fetch_and(arg,std::memory_order_relaxed) ; }
      T fetch_and_relax(T arg) volatile noexcept { return ref().fetch_and(arg,std::memory_order_relaxed) ; }

      T fetch_or(T arg) noexcept { return ref().fetch_or(arg) ; }
      T fetch_or(T arg) volatile noexcept { return ref().fetch_or(arg) ; }
      T fetch_or_relax(T arg) noexcept { return ref().fetch_or(arg,std::memory_order_relaxed) ; }
      T fetch_or_relax(T arg) volatile noexcept { return ref().fetch_or(arg,std::memory_order_relaxed) ; }

      T fetch_xor(T arg) noexcept { return ref().fetch_xor(arg) ; }
      T fetch_xor(T arg) volatile noexcept { return ref().fetch_xor(arg) ; }
      T fetch_xor_relax(T arg) noexcept { return ref().fetch_xor(arg,std::memory_order_relaxed) ; }
      T fetch_xor_relax(T arg) volatile noexcept { return ref().fetch_xor(arg,std::memory_order_relaxed) ; }

      T operator++ () noexcept { return ref().fetch_add(1) + 1 ; }
      T operator++ () volatile noexcept { return ref().fetch_add(1) + 1 ; }
      T operator++ (int) noexcept { return ref().fetch_add(1) ; }
      T operator++ (int) volatile noexcept { return ref().fetch_add(1) ; }

      T operator-- () noexcept { return ref().fetch_sub(1) - 1 ; }
      T operator-- () volatile noexcept { return ref().fetch_sub(1) - 1 ; }
      T operator-- (int) noexcept { return ref().fetch_sub(1) ; }
      T operator-- (int) volatile noexcept { return ref().fetch_sub(1) ; }

      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      operator+= (const T value) noexcept { return ref().fetch_add(value) + value ; } 
      template <typename RetT = T>
      typename std::enable_if<std::is_integral<T>::value,RetT>::type
      operator+= (const T value) volatile noexcept { return ref().fetch_add(value) + value ; } 
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      operator+= (size_t value) noexcept { return ref().fetch_add(value) + value ; } 
      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      operator+= (size_t value) volatile noexcept { return ref().fetch_add(value) + value ; } 

      T operator-= (const T value) noexcept { return ref().fetch_sub(value) - value ; } 
      T operator-= (const T value) volatile noexcept { return ref().fetch_sub(value) - value ; } 

      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      operator-= (size_t value) ;

      T operator&= (const T mask) noexcept { return ref().fetch_and(mask) & mask ; } 
      T operator&= (const T mask) volatile noexcept { return ref().fetch_and(mask) & mask ; } 

      T operator|= (const T mask) noexcept { return ref().fetch_or(mask) | mask ; } 
      T operator|= (const T mask) volatile noexcept { return ref().fetch_or(mask) | mask ; } 

      T operator^= (const T mask) noexcept { return ref().fetch_xor(mask) ^ mask ; } 
      T operator^= (const T mask) volatile noexcept { return ref().fetch_xor(mask) ^ mask ; } 

      // additional operations beyond those for std::atomic
#if __GNUC__ >= 4 && (defined(__i386__) || defined(__x86_64__)) && 0
      bool test_and_set_bit( unsigned bitnum )
	 {
	    bool origbit ;
	    __asm__(
	       "lock bts %[bit], %[var]; setb %[flag]"
	       : [flag] "=q" (origbit), [var] "+m" (v)
	       : [bit] "Ir" (bitnum)
	       ) ;
	    return origbit ;
	 }
      bool test_and_set_bit( unsigned bitnum ) volatile
	 {
	    bool origbit ;
	    __asm__(
	       "lock bts %[bit], %[var]; setb %[flag]"
	       : [flag] "=q" (origbit), [var] "+m" (v)
	       : [bit] "Ir" (bitnum)
	       ) ;
	    return origbit ;
	 }
#else
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
#endif /* GCC inline-assembler for x86 */
#if __GNUC__ >= 4 && (defined(__i386__) || defined(__x86_64__)) && 0
      bool test_and_clear_bit( unsigned bitnum )
	 {
	    bool origbit ;
	    __asm__(
	       "lock btr %[bit], %[var]; setb %[flag]"
	       : [flag] "=q" (origbit), [var] "+m" (v)
	       : [bit] "Ir" (bitnum)
	       ) ;
	    return origbit ;
	 }
      bool test_and_clear_bit( unsigned bitnum ) volatile
	 {
	    bool origbit ;
	    __asm__(
	       "lock btr %[bit], %[var]; setb %[flag]"
	       : [flag] "=q" (origbit), [var] "+m" (v)
	       : [bit] "Ir" (bitnum)
	       ) ;
	    return origbit ;
	 }
#else
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
#endif /* GCC inline-assembler for x86 */
      T test_and_set_mask( T bitmask ) noexcept { return ref().fetch_or(bitmask) & bitmask ; }
      T test_and_set_mask( T bitmask ) volatile noexcept { return ref().fetch_or(bitmask) & bitmask ; }
      T test_and_clear_mask( T bitmask ) noexcept { return ref().fetch_and(~bitmask) & bitmask ; }
      T test_and_clear_mask( T bitmask ) volatile noexcept { return ref().fetch_and(~bitmask) & bitmask ; }

      void decreaseTo( T desired ) { if (desired < v) v = desired ; }
      void increaseTo( T desired ) { if (desired > v) v = desired ; }

      // generic functionality built on top of the atomic primitives
      //  (only available in pointer specializations)
      template <typename RetT = T>
      void push(typename std::enable_if<std::is_pointer<T>::value,RetT>::type node)
	 {
	    T list ;
	    do {
	       list = this->load(std::memory_order_consume) ;
	       node.next(list) ;
	       } while (!this->compare_exchange_weak(list,node)) ;
	 }

      template <typename RetT = T>
      void push(typename std::enable_if<std::is_pointer<T>::value,RetT>::type nodes, T tail)
	 {
	    T list ;
	    do {
	       list = this->load(std::memory_order_acquire) ;
	       tail.next(list) ;
	       } while (!this->compare_exchange_weak(list,nodes)) ;
	 }

      template <typename RetT = T>
      typename std::enable_if<std::is_pointer<T>::value,RetT>::type
      pop()
	 {
	    T head ;
	    T rest ;
	    do {
	       head = this->load(std::memory_order_acquire) ;
	       if (!head)
		  return head ;
	       rest = head.next() ;
	       } while (!this->compare_exchange_weak(head,rest)) ;
	    head.next(nullptr) ;
	    return head ;
	 }

      static Atomic<T>& ref(T& value) noexcept { return reinterpret_cast<Atomic<T>&>(value) ; }
      static Atomic<const T>& ref(const T& value) noexcept { return reinterpret_cast<Atomic<const T>&>(value) ; }

   protected:
      std::atomic<T>& ref() noexcept { return reinterpret_cast<std::atomic<T>&>(v) ; }
      std::atomic<T>& ref() const noexcept { return *((std::atomic<T>*)const_cast<T*>(&v)) ; }
      std::atomic<T>& ref() volatile noexcept { return *((std::atomic<T>*)const_cast<T*>(&v)) ; }

   private:
      T v ;
   } ;

//----------------------------------------------------------------------------
// specializations for Atomic<bool>

// bool doesn't support fetch_or or fetch_and, so we need specific specialization
template <>
inline bool Atomic<bool>::test_and_set_mask(bool mask) noexcept
{
   return mask ? exchange(mask) : false ;
}

template <>
inline bool Atomic<bool>::test_and_set_mask(bool mask) volatile noexcept
{
   return mask ? exchange(mask) : false ;
}

template <>
inline bool Atomic<bool>::test_and_clear_mask(bool mask) noexcept
{
   return mask ? exchange(false) : false ;
}

template <>
inline bool Atomic<bool>::test_and_clear_mask(bool mask) volatile noexcept
{
   return mask ? exchange(false) : false ;
}

#ifdef ERROR
template <> template <>
inline Fr::NullObject std::atomic<Fr::NullObject>::exchange(Fr::NullObject v,
							    std::memory_order) { return *this; }
#endif

#endif /* FrSINGLE_THREADED */

/************************************************************************/
/*	Various kinds of barriers					*/
/************************************************************************/

#ifdef FrSINGLE_THREADED

ALWAYS_INLINE void atomic_thread_fence( std::memory_order )
{
#ifdef __GNUC__
   asm volatile ("" : : : "memory") ;
#endif
}

ALWAYS_INLINE void memoryBarrier() {}
ALWAYS_INLINE void loadBarrier() {}
ALWAYS_INLINE void storeBarrier() {}
ALWAYS_INLINE void barrier()
{
#ifdef __GNUC__
   asm volatile ("" : : : "memory") ;
#endif
}

#else   /* multi-threaded version */

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
   atomic_thread_fence(std::memory_order_relaxed) ;
#ifdef __GNUC__
   asm volatile ("" : : : "memory") ;
#endif
}

#endif /* FrSINGLE_THREADED */


//----------------------------------------------------------------------------
// aliases to shorten things for standard types

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
