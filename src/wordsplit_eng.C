/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-11					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2018 Carnegie Mellon University			*/
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

#include "framepac/words.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class WordSplitterEnglish				*/
/************************************************************************/

WordSplitter::boundary WordSplitterEnglish::boundaryType(const char* window_start, const char* currpos,
   const char* window_end) const
{
   (void)window_start; (void)currpos; (void)window_end;
   return no_boundary ; //FIXME
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file wordsplit_eng.C //
