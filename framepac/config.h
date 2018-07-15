/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-13					*/
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

#ifndef __Fr_CONFIG_H_INCLUDED
#define __Fr_CONFIG_H_INCLUDED

#include <cstdint>
#include <cstdlib>
#include <sys/types.h>

using namespace std ;

/************************************************************************/
/************************************************************************/

#define FramepaC_VERSION "0.07"

// uncomment the following to remove thread-local storage and synchronization
//   primitives; FramepaC will no longer have any thread safety
//#define FrSINGLE_THREADED

// uncomment the following to enable inclusion of GPL'ed libraries for some
//   optional features.  If you uncomment the line, you MUST distribute under
//   the terms of the GPL, and MUST remove any non-GPL alternate license from
//   the file LICENSE.
//#define FRAMEPAC_GPL

/************************************************************************/
/*	Compile-Time Configuration					*/
/************************************************************************/

//*** Hash Tables ***

// specify how much checking and output you want for hash tables:
//   0 - completely silent except for critical errors
//   1 - important messsages only
//   2 - status messages
#define FrHASHTABLE_VERBOSITY 0

// uncomment the following line to collect operational statistics (slightly slower)
//#define FrHASHTABLE_STATS

// uncomment the following line to interleave bucket headers and
//   bucket contents rather than using separate arrays (fewer cache
//   misses, but wastes some space on 64-bit machines if the key or
//   value are 64 bits)
#define FrHASHTABLE_INTERLEAVED_ENTRIES

// uncomment the following line to pass integer keys through FastHash64
//   instead of using them as-is
//#define FrHASHTABLE_USE_FASTHASH64

/************************************************************************/
/************************************************************************/

// default limit for text processing
#ifndef FrMAX_LINE
# define FrMAX_LINE 65500
#endif

/************************************************************************/
/************************************************************************/

// private classes, types, and functions
namespace FramepaC
{

// number of bytes per allocation slab; higher results in less
//   overhead per allocation, but increased cache conflicts (with the
//   typical 256KB 4-way cache, a slab size of 64KB would put EVERY
//   SlabFooter in the same cache set!)
// MUST BE A POWER OF TWO
constexpr int SLAB_SIZE = 4096 ;

// number of slabs to allocate from the OS at once; higher values
//   reduce OS overhead and reduce the wastage due to the need to
//   align on the slab size, but may lead to wasted memory as a
//   result of requesting more from the OS than actually needed.
//   Setting this to one less than a power of two will make SlabGroups
//   use up memory in powers of two
// 4096 slabs of 4096 bytes = 16MB
constexpr int SLAB_GROUP_SIZE = 4095 ;

} ;

/************************************************************************/
/************************************************************************/

#ifdef FrSINGLE_THREADED
# define thread_local
# define if_THREADED(x)
# define if_NONTHREADED(x) x
#else
# ifdef __GNUC__
//  for x86 GCC, the __thread extension is *far* faster than thread_local,
//  because the latter involves a branch and multiple function calls
//  per access to a variable of that storage class, while __thread
//  simply uses an fs: override
#  define thread_local __thread
#endif /* __GNUC__ */
# define if_THREADED(x) x
# define if_NONTHREADED(x)
#endif /* FrSINGLE_THREADED */

/************************************************************************/
/************************************************************************/

#ifdef FRAMEPAC_GPL
  // enable BigNum and Rational types using the GNU multi-precision math library.  This will make the program
  //   fall under the terms of the GPL
#  include <gmp.h>
#elif defined(FRAMEPAC_LGPL) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 60))
  // emulate BigNum with 128-bit integers and Rational with 128-bit floats from quadmath.h (dist with GCC but LGPL)
# include <quadmath.h>
  typedef __int128_t mpz_t ;
  typedef __float128 mpq_t ;
# define mpz_zero() 0
# define mpq_zero() 0.0Q
#else
  // emulate BigNum with the biggest integer type available and Rational with double-precision floating point
  #if defined(__GNUC__) && __GNUC__ >= 7
    typedef __int128 mpz_t ;
    typedef long double mpq_t ;
  #else
    typedef intmax_t mpz_t ;
    typedef double mpq_t ;
  #endif
# define mpz_zero() 0
# define mpq_zero() 0.0
#endif /* FRAMEPAC_GPL */

/************************************************************************/
/************************************************************************/

#ifndef lengthof
# define lengthof(x) (sizeof(x)/sizeof((x)[0]))
#endif


#if defined(__GNUC__)
#define ALWAYS_INLINE [[gnu::always_inline]] inline
#else
#define ALWAYS_INLINE inline
#endif

/************************************************************************/
/************************************************************************/

#if __cplusplus < 201700
namespace std {
   // cache line size
   constexpr size_t hardware_destructive_interference_size = 64 ;
}
#else
#  include <cstddef>
#endif /* < C++17 */

/************************************************************************/
/************************************************************************/

namespace Fr
{
// empty class for use in hash tables that don't require an associated value
class NullObject
   {
   public:
      NullObject() {}
      NullObject(int) {}
      operator bool() { return false ; }

      NullObject& operator = (const NullObject) { return *this ; }
      NullObject& operator += (const NullObject) { return *this ; }
      NullObject exchange(NullObject& other) { return other ; }
   } ;
} // end namespace Fr

/************************************************************************/
/*	Some compatibility declarations, will be moved later		*/
/************************************************************************/

namespace Fr
{

template <typename T>
inline T* New(size_t N = 1) { return (T*)std::malloc(sizeof(T)*N) ; }

template <typename T>
inline T* NewC(size_t N = 1) { return (T*)std::calloc(sizeof(T),N) ; }

template <typename T>
inline T* NewR(T* old, size_t N = 1) { return (T*)std::realloc(old,sizeof(T)*N) ; }

inline void Free(void* p) { std::free(p) ; }

#ifndef unlikely
# if defined(__GNUC__)
#  define unlikely(X) __builtin_expect(X,0)
# else
#  define unlikely(X) (X)
# endif
#endif

} // end namespace Fr

#endif /* !__Fr_CONFIG_H_INCLUDED */

// end of file config.h //

