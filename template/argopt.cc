/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-09-04					*/
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

#include "framepac/argparser.h"
#include "framepac/as_string.h"
#include "framepac/texttransforms.h"

namespace Fr
{

using namespace std ;

/************************************************************************/
/*	Methods for template class ArgOpt				*/
/************************************************************************/

template <typename T>
bool ArgOpt<T>::setDefaultValue()
{
   if (m_have_defvalue)
      {
      m_value = m_defvalue ;
      m_value_set = true ;
      }
   return m_have_defvalue ;
}

//----------------------------------------------------------------------------

// each user of this template file must explicitly instantiate
//   template <typename T>
//   bool ArgOpt<T>::convert(const char* arg)

//----------------------------------------------------------------------------

template <typename T>
bool ArgOpt<T>::validateValue()
{
   if (m_have_minmax)
      {
      if (m_value < m_minvalue)
	 {
	 m_value = m_minvalue ;
	 return false ;
	 }
      if (m_value > m_maxvalue)
	 {
	 m_value = m_maxvalue ;
	 return false ;
	 }
      }
   return true ;
}

//----------------------------------------------------------------------------

template <typename T>
CharPtr ArgOpt<T>::describeDefault() const
{
   return haveDefault() ? as_string(m_defvalue) : CharPtr(nullptr) ;
}

//----------------------------------------------------------------------------

template <typename T>
CharPtr ArgOpt<T>::describeCurrent() const
{
   return as_string(m_value) ;
}

//----------------------------------------------------------------------------

template <typename T>
CharPtr ArgOpt<T>::describeRange() const
{
   return haveRange() ? aprintf("%s to %s",*as_string(m_minvalue),*as_string(m_maxvalue)) : CharPtr(nullptr) ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file argopt.cc //
