/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-05					*/
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
#include "framepac/cstring.h"
#include "framepac/fasthash64.h"

using namespace FramepaC ;

namespace Fr
{

/************************************************************************/
/*	Methods for class CString					*/
/************************************************************************/

unsigned long CString::hashValue() const
{
   return fasthash64(_s,_s?strlen(_s):0) ;
}

//----------------------------------------------------------------------

bool CString::equal(const CString* s) const
{
   return s->_s == _s || strcmp(s->_s,_s) == 0 ; 
}

//----------------------------------------------------------------------

bool CString::equal(const CString& s) const
{
   return s._s == _s || strcmp(s._s,_s) == 0 ; 
}

//----------------------------------------------------------------------

int CString::compare(const CString* s) const
{
   if (s->_s == _s) return 0 ;
   else return strcmp(s->_s,_s) ;
}

//----------------------------------------------------------------------

int CString::compare(const CString& s) const
{
   if (s._s == _s) return 0 ;
   else return strcmp(s._s,_s) ;
}



} // end namespace Fr

// end of file cstring.C //
