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

#include "framepac/bits.h"

namespace Fr
{

/************************************************************************/
/*	Static variables for class BitReverser				*/
/************************************************************************/

Initializer<BitReverser> BitReverser::initializer ;
uint16_t** BitReverser::m_reversed ;

/************************************************************************/
/*	Methods for class BitReverser					*/
/************************************************************************/

void BitReverser::StaticInitialization()
{
   m_reversed = new uint16_t*[table_bits+1] ;
   m_reversed[0] = new uint16_t[1] ;
   m_reversed[0][0] = 0 ;
   for (unsigned i = 1 ; i <= table_bits ; ++i)
      {
      m_reversed[i] = new uint16_t[1<<i] ;
      // recursively build up the reversed k-bit numbers from the reversed (k-1)-bit numbers
      unsigned half = 1U << (i-1) ;
      for (unsigned j = 0 ; j < half ; ++j)
	 {
	 // get the reversed shorter number and shift it to make room for the extra bit
	 uint16_t rev = m_reversed[i-1][j] << 1 ;
	 // first half of table has a zero as the lowest reversed bit
	 m_reversed[i][j] = rev ;
	 // second half of table has a one as the lowest reversed bit
	 m_reversed[i][j+half] = rev + 1 ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------

void BitReverser::StaticCleanup()
{
   for (unsigned i = 0 ; i <= table_bits ; ++i)
      delete[] m_reversed[i] ;
   delete[] m_reversed ;
   return ;
}


} // end namespace Fr

// end of file bitreverser.C //
