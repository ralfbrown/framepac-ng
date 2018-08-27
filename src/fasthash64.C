/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-09-18					*/
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

#include "framepac/fasthash64.h"
using namespace std ;
using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

#if __cplusplus__ >= 201700
#define FALLTHROUGH [[fallthrough]]
#elif defined(__GNUC__) && __GNUC__ > 5
#define FALLTHROUGH [[gnu::fallthrough]]
#else
#define FALLTHROUGH
#endif

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

uint64_t fasthash64(const void* data, size_t len, uint64_t seed)
{
   const uint64_t *ptr = reinterpret_cast<const uint64_t*>(data) ;
   const uint64_t *last = ptr + (len / 8) ;
   uint64_t state = fasthash64_init(len,seed) ;
   while (ptr < last)
      {
      state = fasthash64_add(state,*ptr++) ;
      }
   const unsigned char* ptr2 = reinterpret_cast<const unsigned char*>(ptr) ;
   // grab the remaining zero to seven bytes of data
   uint64_t value = 0 ;
   switch (len & 7)
      {
      case 7:
	 value = ((uint64_t)ptr2[6]) << 48 ;
	 FALLTHROUGH ;
      case 6:
	 value |= ((uint64_t)*((uint16_t*)(ptr2+4))) << 40 ;
	 value |= *((uint32_t*)ptr2) ;
	 state = fasthash64_add(state,value) ;
	 break ;
      case 5:
	 value |= ((uint64_t)ptr2[4]) << 32 ;
	 FALLTHROUGH ;
      case 4:
	 value |= *((uint32_t*)ptr2) ;
	 state = fasthash64_add(state,value) ;
	 break ;
      case 3:
	 value = ((uint64_t)ptr2[2]) << 16 ;
	 FALLTHROUGH ;
      case 2:
	 value |= ((uint64_t)ptr2[1]) << 8 ;
	 FALLTHROUGH ;
      case 1:
	 value |= ((uint64_t)ptr2[0]) ;
	 state = fasthash64_add(state,value) ;
	 FALLTHROUGH ;
      case 0:
	 break ;
      }
   return fasthash64_finalize(state) ;
}

} // end namespace FramepaC

// end of file fasthash64.C
