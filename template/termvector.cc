/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.02, last edit 2017-07-18					*/
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

#include "framepac/termvector.h"
#include "framepac/charget.h"
#include "framepac/fasthash64.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename ValT>
size_t TermVectorT<ValT>::hashValue_(const Object* obj)
{
   auto tv = static_cast<const TermVectorT*>(obj) ;
   size_t numelts = tv->numElements() ;
   FastHash64 hash(numelts) ;
   for (size_t i = 0 ; i < numelts ; ++i)
      {
      hash += tv->elementIndex(i) ;
      hash += tv->elementValue(i) ;
      }
   return *hash ;
}

//----------------------------------------------------------------------------

template <typename ValT>
TermVectorT<ValT>* TermVectorT<ValT>::read(CharGetter& getter, size_t size_hint)
{
   TermVectorT* tv = TermVectorT::create(size_hint) ;
   int nextch ;
   for (size_t i = 0 ; ; ++i)
      {
      uint32_t index { 0 } ;
      while ((nextch = *getter) != EOF && nextch != ':' && nextch != '>')
	 {
	 if (isdigit(nextch))
	    index = 10*index + (nextch - '0') ;
	 }
      if (nextch == EOF || nextch == '>')
	 break ;
      ValT value { 0 } ;
      if (nextch == ':')
	 {
//FIXME: read value
	 }
      tv->setElement(i,index,value) ;
      getter.peekNonWhite() ;
      }
   return tv ;
}


} // end namespace Fr

// end of file termvector.cc //
