/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#ifndef _Fr_BITS_H_INCLUDED
#define _Fr_BITS_H_INCLUDED

#include <cstdint>
#include "framepac/init.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

//----------------------------------------------------------------------------

class BitReverser
   {
   public:
      BitReverser() = default ;
      ~BitReverser() = default ;

      static uint8_t reverse(uint8_t N) { return m_reversed[8][N] ; }
      static uint16_t reverse(uint16_t N) { return reverseBits(N,16) ; }
      static uint32_t reverse(uint32_t N, unsigned numbits)
	 { return numbits <= table_bits ? m_reversed[numbits][N] : reverseBits(N,numbits) ; }
      static uint64_t reverse(uint64_t N, unsigned numbits)
	 { return numbits <= table_bits ? m_reversed[numbits][N] : reverseBits(N,numbits) ; }

      template <typename T>
      static T reverseBits(T N, unsigned numbits)
	 {
	    T reversed = 0 ;
	    for (unsigned i = 0 ; i < numbits ; ++i)
	       {
	       reversed = (reversed << 1) | (N & 1) ;
	       N >>= 1 ;
	       }
	    return reversed ;
	 }

      // to be called *only* by the initializer object
      static void StaticInitialization() ;
      static void StaticCleanup() ;
   private:
      static Fr::Initializer<BitReverser> initializer ;
      static uint16_t** m_reversed ;

      // number of bits for which to use a precomputed table; must be >= 8 for reverse(uint8_t) to work;
      //   not allowed to be larger than 16.  For 10 bits, the precomputed tables occupy 4K plus malloc overhead;
      //   for 12 bits, this increases to 16K
      static constexpr unsigned table_bits = 12 ;
   } ;

} // end namespace Fr

#endif /* !_Fr_BITS_H_INCLUDED */

// end of file bits.h //
