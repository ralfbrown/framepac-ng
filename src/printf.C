/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-07					*/
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

#include <cstdarg>
#include <cstdio>
#include "framepac/cstring.h"
#include "framepac/texttransforms.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

CharPtr vaprintf(const char *fmt, va_list args)
{
   // we need to make a copy of the arglist, because va_lists are passed by
   //   reference, so the first call to vsprintf would clobber 'args' for
   //   the second call!
   va_list argcopy ;
   va_copy(argcopy,args) ;
   size_t len = vsnprintf(nullptr,0,fmt,argcopy) ;
   va_end(argcopy) ;
   CharPtr buf(len+1) ;
   if (buf)
      {
      va_copy(argcopy,args) ;
      vsnprintf((char*)buf,len+1,fmt,args) ;
      va_end(argcopy) ;
      buf[len] = '\0' ;
      }
   return buf ;
}

//----------------------------------------------------------------------------

CharPtr aprintf(const char *fmt, ...)
{
   va_list args ;
   va_start(args,fmt) ;
   auto s = vaprintf(fmt,args) ;
   va_end(args) ;
   return s ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file printf.C //
