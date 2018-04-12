/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-25					*/
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

#include "framepac/vecsim.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

VectorSimilarityOptions::VectorSimilarityOptions()
{
   power = 1.0 ;
   alpha = 0.0 ;
   beta = 0.0 ;
   smoothing = 0.0 ;
   normalize = 0 ;
   use_similarity = false ;
   return ;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool VectorSimilarityOptions::parseOptions(const char* options)
{
   //TODO
   (void)options ;
   return true ;
}

} // end namespace Fr

// end of file vecsimopt.C //
