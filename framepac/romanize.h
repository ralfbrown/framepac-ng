/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-07-29					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#ifndef _Fr_ROMANIZE_H_INCLUDED
#define _Fr_ROMANIZE_H_INCLUDED

#include <cstdint>
#include "framepac/init.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

class Romanizer
   {
   public:
      static unsigned utf8codepoint(const char* buf, wchar_t& codepoint) ;
      static bool romanizable(wchar_t codepoint) ;
      static int romanize(wchar_t codepoint, char* buffer) ;
      static unsigned romanize(wchar_t codepoint, wchar_t& romanized1, wchar_t& romanized2) ;

   public: // to be called only by the initializer
      static void StaticInitialization() ;

   private:
      static Initializer<Romanizer> initializer ;
      static std::uint16_t s_mapping[0xFF20] ;
   } ;

/************************************************************************/
/************************************************************************/

} // end namespace Fr

#endif /* !_Fr_ROMANIZE_H_INCLUDED */

// end of file romanize.h //
