/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-18					*/
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

#ifndef _Fr_NONOBJECT_H_INCLUDED
#define _Fr_NONOBJECT_H_INCLUDED

#include "framepac/object.h"

/************************************************************************/
/************************************************************************/

namespace Fr {


class NonObject : public Object
   {
   public: // types
      typedef Object super ;
   public:
      ~NonObject() {}

   protected: // construction/destruction
      NonObject() {}

   protected: //implementation functions for virtual methods
      friend class FramepaC::Object_VMT<NonObject> ;

      // *** copying ***
      static ObjectPtr clone_(const Object*) { return ObjectPtr(nullptr) ; }
      static Object* shallowCopy_(const Object*) { return nullptr ; }
      static ObjectPtr subseq_int(const Object */*obj*/, size_t /*start*/, size_t /*stop*/) { return ObjectPtr(nullptr) ; }
      static ObjectPtr subseq_iter(const Object */*obj*/, ObjectIter /*start*/, ObjectIter /*stop*/) { return ObjectPtr(nullptr) ; }

      // type determination predicates
      static bool isObject_(const Object*) { return false ; }
      
      // *** standard info functions ***
      static size_t size_(const Object* o) { return FramepaC::Slab::slab(o)->objectSize() ; }
      static bool empty_(const Object*) { return false ; }

      // generate printed representation into a buffer
      static char* toCstring_(const Object* obj, char* buffer,size_t buflen,size_t wrap_at,size_t indent,
	 size_t wrapped_indent) ;
      static bool toJSONString_(const Object* obj,char* buffer,size_t buflen,bool wrap,size_t indent) ;
      // determine length of buffer required for string representation of object
      static size_t cStringLength_(const Object* obj,size_t wrap_at,size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object* obj,bool wrap,size_t indent) ;

   private:
      static const char s_typename[] ;
} ;


} // end namespace Fr

#endif /* !_Fr_NONOBJECT_H_INCLUDED */

// end of file nonobject.h //
