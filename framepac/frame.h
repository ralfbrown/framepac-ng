/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-06-22					*/
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

#include "framepac/object.h"

namespace Fr
{

class Frame : public Object
   {
   private: // static members
      static Allocator s_allocator ;
   private:

   protected:
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      Frame() ;
      Frame(const Frame *) ;
      Frame(const Frame &) ;
      ~Frame() ;

   public:
      static Frame *create() ;
      static Frame *create(const char *) ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Frame> ;

      // type determination predicates
      static bool isFrame_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "Frame" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *) ;
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object *,char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *) ;
      static bool empty_(const Object *) ;

      // *** standard access functions ***
      static Object *front_(Object *) ;
      static const Object *front_(const Object *) ;
      static const char *stringValue_(const Object *obj) ;

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static ObjectIter begin_(const Object *obj) ;
      static ObjectIter cbegin_(const Object *obj) ;
      static ObjectIter end_(const Object *obj) ;
      static ObjectIter cend_(const Object *obj) ;
   } ;

//----------------------------------------------------------------------------

class Slot
   {


   } ;

//----------------------------------------------------------------------------

class Facet
   {

   } ;

} // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Frame> ;

} ; // end namespace FramepaC

// end of file frame.h //
