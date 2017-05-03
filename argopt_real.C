/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-03					*/
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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include "template/argopt.cc"

namespace Fr
{

using namespace std ;

/************************************************************************/
/*	Methods for template class ArgOpt				*/
/************************************************************************/

template <>
bool ArgOpt<float>::convert(const char* arg)
{
   char* endptr = const_cast<char*>(arg) ;
   m_value = (float)strtod(arg,&endptr) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<double>::convert(const char* arg)
{
   char* endptr = const_cast<char*>(arg) ;
   m_value = strtod(arg,&endptr) ;
   return endptr != arg ;
}

//----------------------------------------------------------------------------

// explicit instantiations of the common types
template class ArgOpt<float> ;
template class ArgOpt<double> ;

} // end namespace Fr

// end of file argopt_real.C //
