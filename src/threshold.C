/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.02, last edit 2017-07-28					*/
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

#include "framepac/threshold.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

ThresholdList::ThresholdList(const char* threshold_filename, double def_threshold)
   : m_defthresh(def_threshold)
{
   if (threshold_filename && *threshold_filename)
      {
      //TODO

      }
   return ;
}

//----------------------------------------------------------------------------

ThresholdList::~ThresholdList()
{
//TODO
   return  ;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file threshold.h //
