/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-29					*/
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

#ifndef _Fr_BYTEORDER_H_INCLUDED
#define _Fr_BYTEORDER_H_INCLUDED

#include <cstdint>
#include "framepac/config.h"

// check for #defines indicating little-endian target architectures
#if (defined(__GNUC__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) \
   || defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) || defined(__MIPSEL) \
   || defined(__MIPSEL__) || defined(_M_IX86) || defined(_M_X86) || defined(_M_IA64) || defined(_M_ARM) \
   || defined(__LITTLE_ENDIAN__)
#  define FrLITTLE_ENDIAN

// check for #defines indicating big-endian target architectures
#elif (defined(__GNUC__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) \
   || defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || defined(_MIPSEB) || defined(__MIPSEB) \
   || defined(__MIPSEB__) || defined(_M_PPC) \
   || defined(__BIG_ENDIAN__)
#  define FrBIG_ENDIAN
#else

// try to convince the preprocessor to determine endianness for us
# if '1234' == 0x31323334
#  define FrBIG_ENDIAN
# else
#  define FrLITTLE_ENDIAN
# endif
#endif

// final fallback if none of the above worked: manual configuration
#if !defined(FrLITTLE_ENDIAN) && !defined(FrBIG_ENDIAN)
// uncomment the appropriate line below for the endianness of the platform if it is not automatically detected above
#define FrLITTLE_ENDIAN
//#define FrBIG_ENDIAN
#endif /* !FrLITTLE_ENDIAN && !FrBIG_ENDIAN */

namespace Fr
{

#if defined(FrLITTLE_ENDIAN)
# define if_LITTLE_ENDIAN(x) x
# define if_BIG_ENDIAN(x)
#elif defined(FrBIG_ENDIAN)
# define if_LITTLE_ENDIAN(x)
# define if_BIG_ENDIAN(x) x
#else
#  error Neither FrLITTLE_ENDIAN nor FrBIG_ENDIAN is defined
#endif /* FrLITTLE_ENDIAN */

/************************************************************************/
/************************************************************************/

template <int bytes, typename numtype>
class ByteOrdered
   {
   private:
      typedef numtype DataType ;
      unsigned char m_value[bytes] ;
#ifdef FrLITTLE_ENDIAN
      constexpr unsigned byteswap(unsigned index) { return index ; }
#else
      constexpr unsigned byteswap(unsigned index) { return bytes - 1 - index ; }
#endif
   public:
      ByteOrdered() = default ;
      ByteOrdered(numtype v) { store(v) ; }
      ByteOrdered(const ByteOrdered &old) = default ;
      ~ByteOrdered() = default ;

      numtype load() const ;
      void store(const void *v) ;
      void store(numtype v) { store(&v) ; }

      bool operator == (const ByteOrdered &other) const { return load() == other.load() ; }
      bool operator == (numtype x) const { return load() == x ; }
      numtype operator * () const { return load() ; }
      operator numtype () const { return load() ; }
      ByteOrdered& operator = (numtype v) { store(&v) ; return *this ; }
      ByteOrdered& operator += (numtype v) { store(load()+v) ; return *this ; }
      ByteOrdered& operator -= (numtype v) { store(load()-v) ; return *this ; }
      ByteOrdered& operator *= (numtype v) { store(load()*v) ; return *this ; }
      ByteOrdered& operator /= (numtype v) { store(load()/v) ; return *this ; }
      ByteOrdered& operator -- () { store(load()-1) ; return *this ; }
      ByteOrdered& operator ++ () { store(load()+1) ; return *this ; }
   } ;

// the sizes corresponding to a native type can be optimized in the little-endian case
#if defined(FrLITTLE_ENDIAN)
template <>
inline void ByteOrdered<2,uint16_t>::store(const void *v)
{
   DataType val = *((const DataType*)v) ;
   DataType *vp = (DataType*)m_value ;
   *vp = val ;
   return  ;
}

template <>
inline uint16_t ByteOrdered<2,uint16_t>::load() const
{
   DataType *vp = (DataType*)m_value ;
   return *vp ;
}

template <>
inline void ByteOrdered<4,uint32_t>::store(const void *v)
{
   DataType val = *((const DataType*)v) ;
   DataType *vp = (DataType*)m_value ;
   *vp = val ;
   return  ;
}

template <>
inline uint32_t ByteOrdered<4,uint32_t>::load() const
{
   DataType *vp = (DataType*)m_value ;
   return *vp ;
}

template <>
inline void ByteOrdered<8,uint64_t>::store(const void *v)
{
   DataType val = *((const DataType*)v) ;
   DataType *vp = (DataType*)m_value ;
   *vp = val ;
   return  ;
}

template <>
inline uint64_t ByteOrdered<8,uint64_t>::load() const
{
   DataType *vp = (DataType*)m_value ;
   return *vp ;
}

template <>
inline void ByteOrdered<4,float>::store(const void *v)
{
   DataType val = *((const DataType*)v) ;
   DataType *vp = (DataType*)m_value ;
   *vp = val ;
   return  ;
}

template <>
inline float ByteOrdered<4,float>::load() const
{
   DataType *vp = (DataType*)m_value ;
   return *vp ;
}

template <>
inline void ByteOrdered<8,double>::store(const void *v)
{
   DataType val = *((const DataType*)v) ;
   DataType *vp = (DataType*)m_value ;
   *vp = val ;
   return  ;
}

template <>
inline double ByteOrdered<8,double>::load() const
{
   DataType *vp = (DataType*)m_value ;
   return *vp ;
}
#endif /* FrLITTLE_ENDIAN */

// sizes not corresponding to a native type currently just use a naive implementation
template <int bytes, typename numtype>
void ByteOrdered<bytes,numtype>::store(const void *v)
{
   for (unsigned i = 0 ; i < bytes ; i++)
      {
      m_value[i] = ((unsigned char*)v)[byteswap(i)] ;
      }
   return ;
}

template <int bytes, typename numtype>
numtype ByteOrdered<bytes,numtype>::load() const
{
   numtype value = m_value[0] ;
   for (unsigned i = 1 ; i < bytes ; ++i)
      {
      value |= (m_value[i] << (8*i)) ;
      }
   return value ;
}

#ifndef FrLITTLE_ENDIAN
template <>
float ByteOrdered<4,float>::load() const
{
   union
      {
      DataType f ;
      unsigned char b[sizeof(DataType)] ;
      } float_or_bytes ;
   for (unsigned i = 0 ; i < sizeof(DataType) ; ++i)
      {
      float_or_bytes.b[byteswap(i)] = m_value[i] ;
      }
   return float_or_bytes.f ;
}
#endif /* !FrLITTLE_ENDIAN */

#ifndef FrLITTLE_ENDIAN
template <>
double ByteOrdered<8,double>::load() const
{
   union
      {
      DataType d ;
      unsigned char b[sizeof(DataType)] ;
      } double_or_bytes ;
   for (unsigned i = 0 ; i < sizeof(DataType) ; ++i)
      {
      double_or_bytes.b[byteswap(i)] = m_value[i] ;
      }
   return double_or_bytes.d ;
}
#endif /* !FrLITTLE_ENDIAN */

//----------------------------------------------------------------------------

typedef ByteOrdered<2,int16_t> Int16 ;
typedef ByteOrdered<3,int32_t> Int24 ;
typedef ByteOrdered<4,int32_t> Int32 ;
typedef ByteOrdered<5,int64_t> Int40 ;
typedef ByteOrdered<6,int64_t> Int48 ;
typedef ByteOrdered<7,int64_t> Int56 ;
typedef ByteOrdered<8,int64_t> Int64 ;
typedef ByteOrdered<2,uint16_t> UInt16 ;
typedef ByteOrdered<3,uint32_t> UInt24 ;
typedef ByteOrdered<4,uint32_t> UInt32 ;
typedef ByteOrdered<5,uint64_t> UInt40 ;
typedef ByteOrdered<6,uint64_t> UInt48 ;
typedef ByteOrdered<7,uint64_t> UInt56 ;
typedef ByteOrdered<8,uint64_t> UInt64 ;
typedef ByteOrdered<4,float> Float32 ;
typedef ByteOrdered<8,double> Float64 ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_BYTEORDER_H_INCLUDED */

// end of file byteorder.h //
