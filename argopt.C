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

#include <cstring>
#include "template/argopt.cc"

namespace Fr
{

using namespace std ;

/************************************************************************/
/*	Methods for template class ArgOpt				*/
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
bool ArgOpt<bool>::convert(const char* arg)
{
   if (!arg || !*arg)
      return false ;
   if (*arg == '=' || strcasecmp(arg,"default") == 0)
      {
      if (m_have_defvalue)
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

template <>
bool ArgOpt<int>::convert(const char* arg)
{
   char* endptr = const_cast<char*>(arg) ;
   m_value = (int)strtol(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<unsigned>::convert(const char* arg)
{
   char* endptr = const_cast<char*>(arg) ;
   m_value = (unsigned)strtoul(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<long>::convert(const char* arg)
{
   char* endptr = const_cast<char*>(arg) ;
   m_value = strtol(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<size_t>::convert(const char* arg)
{
   char* endptr = const_cast<char*>(arg) ;
   m_value = strtoul(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<const char*>::convert(const char* arg)
{
   m_value = arg ;
   return true ;
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

//----------------------------------------------------------------------------

// explicit instantiations of the common types
template class ArgOpt<bool> ;
template class ArgOpt<int> ;
template class ArgOpt<unsigned> ;
template class ArgOpt<long> ;
template class ArgOpt<size_t> ;
template class ArgOpt<const char*> ;

} // end namespace Fr

// end of file argopt.C //
