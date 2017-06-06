/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-05					*/
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

#ifndef _Fr_NONOBJECT_H_INCLUDED
#define _Fr_NONOBJECT_H_INCLUDED

#include "framepac/object.h"

/************************************************************************/
/************************************************************************/

namespace Fr {


class NonObject : public Object
   {
   private:
      // no data members
   protected:
      NonObject() {}
   public:
      ~NonObject() {}

      // *** copying ***
      static ObjectPtr clone_(const Object*) { return ObjectPtr(nullptr) ; }
      static Object* shallowCopy_(const Object*) { return nullptr ; }
      static ObjectPtr subseq_int(const Object */*obj*/, size_t /*start*/, size_t /*stop*/) { return ObjectPtr(nullptr) ; }
      static ObjectPtr subseq_iter(const Object */*obj*/, ObjectIter /*start*/, ObjectIter /*stop*/) { return ObjectPtr(nullptr) ; }

      // *** reclamation for non-Object items
      static void releaseSlab_(FramepaC::Slab*) ;

      // name of the actual type of the current object
      static const char* typeName_(const Object *) { return "NonObject" ; }

      // *** standard info functions ***
      static size_t size_(const Object* o) { return FramepaC::Slab::slab(o)->objectSize() ; }
      static bool empty_(const Object*) { return false ; }

      // generate printed representation into a buffer
      static bool toCstring_(const Object* obj, char* buffer,size_t buflen,size_t wrap_at,size_t indent) ;
      static bool toJSONString_(const Object* obj,char* buffer,size_t buflen,bool wrap,size_t indent) ;
      // determine length of buffer required for string representation of object
      static size_t cStringLength_(const Object* obj,size_t wrap_at,size_t indent) ;
      static size_t jsonStringLength_(const Object* obj,bool wrap,size_t indent) ;
} ;


} // end namespace Fr

#endif /* !_Fr_NONOBJECT_H_INCLUDED */

// end of file nonobject.h //
