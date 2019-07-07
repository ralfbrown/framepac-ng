/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-07					*/
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

#include "framepac/utility.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) && (defined(__SSE4_2__) || !defined(FrFAST_MULTIPLY))
#else
unsigned popcount(uint64_t val)
{
#if defined(FrFAST_MULTIPLY)
   // from graphics.stanford.edu/~seander/bithacks.html
   val = val - ((val >> 1) & 0x5555555555555555) ;
   val = (val & 0x3333333333333333) + ((val >> 2) & 0x3333333333333333) ;
   val = (((val + (val >> 4)) & 0x0F0F0F0F0F0F0F0F) * 0x0101010101010101) >> 56 ;
   return val ;
#else
   //optimized version as shown at http://en.wikipedia.org/wiki/Hamming_weight
   val = ((val >> 1) & 0x5555555555555555) + (val & 0x5555555555555555) ;
   val = ((val >> 2) & 0x3333333333333333) + (val & 0x3333333333333333) ;
   val = (val + (val >> 4)) & 0x0F0F0F0F0F0F0F0F ;
   val += (val >> 8) ;
   val += (val >> 16) ;
   val += (val >> 32) ;
   return val & 0x7F ;
#endif
}
#endif /* GCC v3.4 or higher */

//----------------------------------------------------------------------

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) && (defined(__SSE4_2__) || !defined(FrFAST_MULTIPLY))
#else
unsigned popcount(uint32_t val)
{
#if defined(FrFAST_MULTIPLY)
   // from graphics.stanford.edu/~seander/bithacks.html
   val = val - ((val >> 1) & 0x55555555) ;
   val = (val & 0x33333333) + ((val >> 2) & 0x33333333) ;
   val = (((val + (val >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24 ;
   return val ;
#else
   val = ((val >> 1) & 0x55555555) + (val & 0x55555555) ;
   val = ((val >> 2) & 0x33333333) + (val & 0x33333333) ;
   val = ((val + (val >> 4)) & 0x0F0F0F0F) ;
   val = val + (val >> 8) ;
   val = val + (val >> 16) ;
   return val & 0x3F ;
#endif
}
#endif /* GCC v3.4 or higher */

//----------------------------------------------------------------------

unsigned popcount(uint16_t val)
{
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
   return __builtin_popcount(val) ;
#else
   val = ((val & 0xAAAA) >> 1) + (val & 0x5555) ;
   val = ((val & 0xCCCC) >> 2) + (val & 0x3333) ;
   val = ((val + (val >> 4)) & 0x0F0F) ;
   val = val + (val >> 8) ;
   return val & 0x1F ;
#endif
}

} // end namespace Fr

// end of file popcount.C //
