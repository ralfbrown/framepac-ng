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

#ifndef _Fr_THRESHOLD_H_INCLUDED
#define _Fr_THRESHOLD_H_INCLUDED

namespace Fr
{

class ThresholdList
   {
   public:
      ThresholdList(double def_threshold = 0.0) : m_defthresh(def_threshold) {}
      ThresholdList(const char* threshold_filename, double def_threshold) ;
      ~ThresholdList() ;

      double defaultThreshold() const { return m_defthresh ; }

   protected:
      double m_defthresh ;
   } ;

} // end namespace Fr

#endif /* !_Fr_THRESHOLD_H_INCLUDED */

// end of file threshold.h //
