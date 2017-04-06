/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-05					*/
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

#include <cstring>
#include "framepac/cstring.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class CString					*/
/************************************************************************/

// code from frsymtab.C, removing string-length return //
#if defined(__GNUC__) && defined(__386__)
inline unsigned long hashvalue(const char *name)
{
   unsigned long hashvalue ;
   unsigned long t ;
   __asm__("xor %0,%0\n\t"
	   "xor %1,%1\n\t"
     ".LBL%=:\n\t"
           "ror $5,%0\n\t"
	   "movb (%2),%b1\n\t"
	   "inc %2\n\t"
	   "add %1,%0\n\t"
	   "test %b1,%b1\n\t"
	   "jne .LBL%=\n\t"
	   : "=&r" (hashvalue), "=&q" (t), "=r" (name)  // we modify 'name', so it must be in the output list
	   : "2" (name)
	   : "cc" ) ;
   return hashvalue ;
}

#elif defined(_MSC_VER) && _MSC_VER >= 800
inline unsigned long hashvalue(const char *name)
{
   unsigned long hash ;
   _asm {
          mov ebx,name ;
	  xor eax,eax ;
	  xor edx,edx ;
        } ;
     l1:
   _asm {
          ror eax,5
	  mov dl,[ebx]
	  inc ebx
	  add eax,edx
	  test dl,dl
	  jne l1
        } ;
   return hash ;
}

#else // neither Watcom C++ nor Visual C++ nor GCC on x86

#if __BITS__==64
#define rotr5(X) ((sum << 59) | ((sum >> 5) & 0x07FFFFFFFFFFFFFFL))
#else
#define rotr5(X) ((sum << 27) | ((sum >> 5) & 0x07FFFFFF))
#endif

// BorlandC++ won't inline functions containing while loops
#ifdef __BORLANDC__
static
#else
inline
#endif /* __BORLANDC__ */
unsigned long hashvalue(const char *name)
{
   register unsigned long sum = 0 ;
   int c ;
   do
      {
      c = *(unsigned char*)name++ ;
      sum = rotr5(sum) + c ;
      } while (c) ;
   return sum ;
}
#endif /* __WATCOMC__ && __386__ */

//----------------------------------------------------------------------

unsigned long CString::hashValue() const
{
   return hashvalue(_s) ;
}

//----------------------------------------------------------------------

bool CString::equal(const CString* s) const
{
   return s->_s == _s || strcmp(s->_s,_s) == 0 ; 
}

//----------------------------------------------------------------------

bool CString::equal(const CString& s) const
{
   return s._s == _s || strcmp(s._s,_s) == 0 ; 
}

//----------------------------------------------------------------------

int CString::compare(const CString* s) const
{
   if (s->_s == _s) return 0 ;
   else return strcmp(s->_s,_s) ;
}

//----------------------------------------------------------------------

int CString::compare(const CString& s) const
{
   if (s._s == _s) return 0 ;
   else return strcmp(s._s,_s) ;
}



} // end namespace Fr

// end of file cstring.C //
