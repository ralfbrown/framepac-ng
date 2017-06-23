/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
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

#include "framepac/nonobject.h"

using namespace FramepaC ;
using namespace Fr ;

namespace Fr
{

// /*static*/ const FramepaC::Object_VMT<NonObject> nonobject_vmt ;

/************************************************************************/
/*	Methods for class NonObject					*/
/************************************************************************/

size_t NonObject::cStringLength_(const Object *obj, size_t /*wrap_at*/, size_t indent)
{
   return snprintf(nullptr,0,"%*s#NonObject<%lu>",(int)indent,"",(unsigned long)obj) ;
}

//----------------------------------------------------------------------------

bool NonObject::toCstring_(const Object *obj, char *buffer, size_t buflen, size_t /*wrap_at*/, size_t indent)
{
   if (!buffer)
      return false ;
   size_t count = snprintf(buffer,buflen,"%*s#NonObject<%lu>%c",(int)indent,"",(unsigned long)obj,'\0') ;
   return count <= buflen ;
}

//----------------------------------------------------------------------------

size_t NonObject::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool NonObject::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool wrap, size_t indent)
{
   (void)obj; (void)buflen; (void)wrap; (void)indent; //FIXME
   if (!buffer)
      return false ;

   return false ; //FIXME
}



} // end namespace Fr

// end of file nonobject.C //
