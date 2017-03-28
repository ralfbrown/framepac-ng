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

#ifndef _Fr_PERTHREAD_H_INCLUDED
#define _Fr_PERTHREAD_H_INCLUDED

#include "framepac/config.h"

class PerThreadData
   {
   public:
      PerThreadData() ;
      ~PerThreadData() ;

      // insert a per-thread instance of the data being tracked
      PerThreadData& operator += (void *data) ;
      // remove a per-thread instance of the data being tracked
      PerThreadData& operator -= (void *data) ;

      //begin()
      //end()
   private:

   } ;


#endif /* !_Fr_PERTHREAD_H_INCLUDED */

// end of perthread.h //

