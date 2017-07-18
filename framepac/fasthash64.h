/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-08					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

#ifndef __FrFASTHASH64_H_INCLUDED
#define __FrFASTHASH64_H_INCLUDED

#include <cstdint>

namespace FramepaC
{

// implementation of fasthash64 hash function patterned after that by Zilong Tam (2012, MIT License), available at
//    https://github.com/rurban/smhasher/blob/master/fasthash.cpp
// limitations: requires unaligned memory access, gives different results depending on endianness

constexpr std::uint64_t FH64_MULTIPLIER = 0x880355f21e6d1965ULL ;

inline std::uint64_t fasthash64_mix(std::uint64_t state)
{
   state ^= (state >> 23) ;
   state *= 0x2127599bf4325c37ULL ;
   state ^= (state >> 47) ;
   return state ;
}

//----------------------------------------------------------------------------

// start an incremental hashing (e.g. of the hash values of subobjects of an object)
inline std::uint64_t fasthash64_init(std::size_t total_len, std::uint64_t seed = 0)
{
   return seed ^ (total_len * FH64_MULTIPLIER) ;
}

//----------------------------------------------------------------------------

// add a 64-bit item to the accumulating hash value
inline std::uint64_t fasthash64_add(std::uint64_t state, std::uint64_t newvalue)
{
   state ^= fasthash64_mix(newvalue) ;
   return state * FH64_MULTIPLIER ;
}

//----------------------------------------------------------------------------

// apply the final step of the hash value computation
inline std::uint64_t fasthash64_finalize(std::uint64_t state)
{
   return fasthash64_mix(state) ;
}

//----------------------------------------------------------------------------

// hash the contents of a buffer
std::uint64_t fasthash64(const void* data, std::size_t len, std::uint64_t seed = 0) ;

//----------------------------------------------------------------------------

// hash the actual pointer, not what it points at
inline std::uint64_t fasthash64_ptr(const void* value, std::uint64_t seed = 0)
{
   return fasthash64_finalize(fasthash64_add(fasthash64_init(sizeof(void*),seed),(std::uint64_t)value)) ;
}

//----------------------------------------------------------------------------

// hash an integral value
inline std::uint64_t fasthash64_int(std::uint64_t value, std::uint64_t seed = 0)
{
   return fasthash64_finalize(fasthash64_add(fasthash64_init(sizeof(std::uint64_t),seed),value)) ;
}

//----------------------------------------------------------------------------

// hash a floating-point value
inline std::uint64_t fasthash64_float(double value, std::uint64_t seed = 0)
{
   static_assert(sizeof(double) == sizeof(std::uint64_t),"double must take the same space as std::uint64_t") ;
   std::uint64_t* val64 = reinterpret_cast<std::uint64_t*>(&value) ;
   return fasthash64_mix(fasthash64_add(fasthash64_init(sizeof(double),seed),*val64)) ;
}

//----------------------------------------------------------------------------

} // end namespace FramepaC

/************************************************************************/
/************************************************************************/

namespace Fr
{

class FastHash64
   {
   public:
      FastHash64() { m_hash = 0 ; }
      //  initialize with just the data length
      FastHash64(std::uint64_t len, std::uint64_t seed)
	 {
	 m_hash = FramepaC::fasthash64_init(len,seed) ;
	 }
      FastHash64(double value, std::uint64_t seed = 0) : FastHash64(sizeof(double),seed)
	 {
	 std::uint64_t* val64 = reinterpret_cast<std::uint64_t*>(&value) ;
	 m_hash = FramepaC::fasthash64_add(m_hash,*val64) ;
	 }
      FastHash64(const void* data, std::size_t len, std::uint64_t seed = 0)
	 {
	 m_hash = FramepaC::fasthash64(data,len,seed) ;
	 }

      // update the hash value with additional data
      FastHash64& operator += (unsigned val)
	 {
	 m_hash = FramepaC::fasthash64_add(m_hash,val) ;
	 return *this ;
	 }
      FastHash64& operator += (std::uint64_t val)
	 {
	 m_hash = FramepaC::fasthash64_add(m_hash,val) ;
	 return *this ;
	 }
      FastHash64& operator += (double val)
	 {
	 std::uint64_t* val64 = reinterpret_cast<std::uint64_t*>(&val) ;
	 m_hash = FramepaC::fasthash64_add(m_hash,*val64) ;
	 return *this ;
	 }
      FastHash64& add(const void* data, std::size_t len)
	 {
	 m_hash = FramepaC::fasthash64(data,len,m_hash) ;
	 return *this ;
	 }

      std::uint64_t operator* () const { return FramepaC::fasthash64_finalize(m_hash) ; }

   protected:
      std::uint64_t m_hash ;
   } ;

} // end namespace Fr

#endif /* !__FrFASTHASH64_H_INCLUDED */

// end of file fasthash64.h
