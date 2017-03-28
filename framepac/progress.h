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

#ifndef _Fr_PROGRESS_H_INCLUDED
#define _Fr_PROGRESS_H_INCLUDED

#include "framepac/config.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

class ProgressIndicator
   {
   public:
      ProgressIndicator(size_t interval, size_t limit) ;
      ~ProgressIndicator() ;

      ProgressIndicator& operator += (size_t add) ;
      ProgressIndicator& operator++ () ;

   protected:
      size_t   m_limit ;
      size_t   m_interval ;
      size_t   m_count ;
      size_t   m_since_update ;
   } ;

//----------------------------------------------------------------------------

class NullProgressIndicator : public ProgressIndicator
   {
   public:
      NullProgressIndicator() : ProgressIndicator(0,0) {}
      ~NullProgressIndicator() {}

      NullProgressIndicator& operator += (size_t) { return *this ; }
      NullProgressIndicator& operator++ () { return *this ; }
   protected:

   } ;

//----------------------------------------------------------------------------

class ConsoleProgressIndicator
   {
   public:
      ConsoleProgressIndicator(size_t interval, size_t limit, size_t per_line,
			       const char *first_prefix, const char *rest_prefix) ;
      ~ConsoleProgressIndicator() ;

      ConsoleProgressIndicator& operator += (size_t) ;
      ConsoleProgressIndicator& operator++ () ;
   protected:

   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_PROGRESS_H_INCLUDED */

// end of file progress.h //
