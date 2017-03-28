/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
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

#ifndef _Fr_BWT_H_INCLUDED
#define _Fr_BWT_H_INCLUDED

#include "framepac/config.h"


/*  Compressed BWT
     use 64-entry chunks
        each consists of a 32-bit index into a pointer array for the first pointer
	plus 64 bitflags for whether an explicit pointer is in use (if not, add 1
	to the value of the previous entry).
     for speed on machines without bit-scan instructions, store the
       bitflags in reverse order, with the lowest-indexed flag in the
       MSB, since we can extract the lowest set bit with (X&(-X))
       while the highest set bit has no such simple expression
     extracting pointer and adjustment:
       idx %= 64   // index within chunk
       unwanted = (1<<(64-idx))-1 ;
       wanted = flags & ~unwanted ;   // mask off flag bits past idx'th position
       pointernum = popcnt(wanted) ;  // get index into pointer array
       ptrflag = wanted & -wanted ;   // isolate the lowest set bit
       add = (ptrflag-1) & ~(wanted|unwanted) ;  // bits between lowest set bit and idx
       offset = popcnt(add) ;         // how many such bits

       addr = ptrs[first+ptrnum]+offset ;
*/


#endif /* !_Fr_BWT_H_INCLUDED */

// end of bwt.h //
