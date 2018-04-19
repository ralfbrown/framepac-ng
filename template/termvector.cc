/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-19					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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

#include "framepac/termvector.h"
#include "framepac/charget.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename ValT>
TermVectorT<ValT>* TermVectorT<ValT>::read(CharGetter& getter, size_t size_hint)
{
   TermVectorT* tv = TermVectorT::create(size_hint) ;
   int nextch ;
   for (size_t i = 0 ; ; ++i)
      {
      getter.peekNonWhite() ;		// discard whitespace
      uint32_t index ;
      FramepaC::read_value(getter,index) ;
      nextch = *getter ;		// consume the terminating character
      if (nextch == EOF || nextch == '>') // end of object descriptor?
	 break ;
      ValT value { 0 } ;
      if (nextch == ':')		// verify that we have a value before reading it
	 {
	 FramepaC::read_value(getter,value) ;
	 }
      tv->setElement(i,index,value) ;
      }
   return tv ;
}


} // end namespace Fr

// end of file termvector.cc //
