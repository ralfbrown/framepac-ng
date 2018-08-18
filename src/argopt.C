/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-17					*/
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

#include <cstring>
#include "framepac/texttransforms.h"
#include "template/argopt.cc"

namespace Fr
{

using namespace std ;

/************************************************************************/
/*	Methods for template class ArgOpt<bool>				*/
/************************************************************************/

template <>
bool ArgOpt<bool>::setDefaultValue()
{
   m_value = !m_value ;
   m_value_set = true ;
   return true ;
}

//----------------------------------------------------------------------------

template <>
bool ArgOpt<bool>::convert(const char* arg, const char* /*delim*/)
{
   if (!arg || !*arg)
      return false ;
   if (*arg == '=' || strcasecmp(arg,"default") == 0)
      {
      if (haveDefault())
	 {
	 m_value = m_defvalue ;
	 return true ;
	 }
      }
   if (*arg == '-'
       || strcmp(arg,"0") == 0 || strcasecmp(arg,"n") == 0 || strcasecmp(arg,"no") == 0
       || strcasecmp(arg,"f") == 0 || strcasecmp(arg,"false") == 0)
      {
      m_value = false ;
      return true ;
      }
   if (*arg == '+'
       || strcmp(arg,"1") == 0 || strcasecmp(arg,"y") == 0 || strcasecmp(arg,"yes") == 0
       || strcasecmp(arg,"t") == 0 || strcasecmp(arg,"true") == 0)
      {
      m_value = true ;
      return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

template <>
bool ArgOpt<bool>::validateValue()
{
   return true ;
}

//----------------------------------------------------------------------------

template <>
bool ArgOpt<bool>::optional() const
{
   return true ;
}

/************************************************************************/
/*	Methods for template class ArgOpt<char*>			*/
/************************************************************************/

template <>
bool ArgOpt<char*>::convert(const char* arg, const char* delim)
{
   bool success ;
   const char* new_val = string_as<const char*>(arg,success) ;
   if (!success)
      return false ;
   CharPtr concat = delim ? aprintf("%s%s%s",m_value,delim,new_val) : dup_string(new_val) ;
   delete[] m_value ;
   m_value = concat.move() ;
   return true ;
}

/************************************************************************/
/************************************************************************/

// explicit instantiations of the common types
template class ArgOpt<bool> ;
template class ArgOpt<int> ;
template class ArgOpt<unsigned> ;
template class ArgOpt<long> ;
template class ArgOpt<size_t> ;
template class ArgOpt<const char*> ;
template class ArgOpt<char*> ;

} // end namespace Fr

// end of file argopt.C //
