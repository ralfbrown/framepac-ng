/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-14					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#ifndef Fr_CONVERT_H_INCLUDED
#define Fr_CONVERT_H_INCLUDED

#include <cstdint>

namespace Fr
{

   template <typename T>
   bool convert_string(const char*& input, T& value) ;

   // predefined specializations
   template <> bool convert_string(const char*& input, std::uint32_t& value) ;
   template <> bool convert_string(const char*& input, std::int32_t& value) ;
   template <> bool convert_string(const char*& input, std::size_t& value) ;
   template <> bool convert_string(const char*& input, char& value) ;
   template <> bool convert_string(const char*& input, int& value) ;
   template <> bool convert_string(const char*& input, long& value) ;
   template <> bool convert_string(const char*& input, float& value) ;
   template <> bool convert_string(const char*& input, double& value) ;
   template <> bool convert_string(const char*& input, long double& value) ;
   
} // end namespace Fr

#endif /* !Fr_CONVERT_H_INCLUDED */

// end of file convert.h //
