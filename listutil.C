/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
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

#include "framepac/list.h"
#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

List* List::createWordList(const char* s)
{
   ListBuilder lb ;
   if (s && *s)
      {
      const char* eos = strchr(s,'\0') ;
      if (eos > s && eos[-1] == '\n')	// strip optional trailing newline
	 --eos ;
      for (const char* blank = s ; blank < eos ; ++blank)
	 {
	 if (*blank == ' ')
	    {
	    lb += String::create(s,blank-s) ;
	    s = blank + 1 ;		// advance past the blank
	    }
	 }
      // add in the final word if the string didn't end with a blank
      if (*s && s < eos)
	 lb += String::create(s,eos-s) ;
      }
   return lb.move() ;
}


} // end namespace Fr

// end of file listutil.C //
