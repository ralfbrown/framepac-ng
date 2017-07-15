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

#ifndef __Fr_NUMBER_H_INCLUDED
#define __Fr_NUMBER_H_INCLUDED

#include "framepac/object.h"

namespace Fr {

//----------------------------------------------------------------------------

class Number : public Object
   {
   public:
      static Number *create(long) ;
      static Number *create(uint32_t) ;
      static Number *create(double) ;
      static Number *create(const Number *) ;
      static Number *create(const char *) ;

   private:
      // no data members //
   protected: // creation/destruction
      Number() {}
      ~Number() {}

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Number> ;

      // type determination predicates
      static bool isNumber_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "Number" ; }

      // *** copying ***
      using Object::clone_ ;
      using Object::shallowCopy_ ;
      static ObjectPtr subseq_int(const Object *, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object *obj) { delete (Number*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete (Number*)obj ; }

#if 0
      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object *,char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;
#endif

      // *** standard info functions ***
      static size_t size_(const Object *obj) { return obj ? 1 : 0 ; }

      // *** standard access functions ***
      static Object *front_(Object *) ;
      static const Object *front_const(const Object *) ;

      // *** comparison functions ***
      // since Number is an abstract type with no contents, we can just inherit Object's methods
      using Object::hashValue_ ;
      using Object::equal_ ;
      using Object::compare_ ;
      using Object::lessThan_ ;
   } ;

/************************************************************************/
/************************************************************************/

class Integer : public Number
   {
   public:
      static Integer *create(long val = 0) { return new Integer(val) ; }
      static Number *create(uint32_t val) { return new Integer((long)val) ; }
      static Integer *create(double val) { return new Integer((long)val) ; }
      static Integer *create(const Object *obj) { return new Integer(obj->intValue()) ; }
      static Integer *create(const Integer *obj) { return new Integer(obj->m_value) ; }
      static Integer *create(const char *val, int radix = 0) { return new Integer(val,radix) ; }

      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
   private: // static members
      static Allocator s_allocator ;
   private:
      long int m_value ;
   protected: // creation/destruction
      Integer(long value = 0) : m_value(value) {}
      Integer(const char *value, unsigned radix = 0) ;
      ~Integer() {}

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Integer> ;

      // type determination predicates
      static bool isInteger_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "Integer" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *obj)
         { return new Integer(static_cast<const Integer&>(*obj)) ; }
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      using Number::subseq_int ;
      using Number::subseq_iter ;

      // *** destroying ***
      static void free_(Object *obj) { delete (Integer*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete (Integer*)obj ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object *, char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *) { return 1 ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) ;
      static const Object *front_const(const Object *obj) ;
      static long int intValue_(const Object *obj) { return ((Integer*)obj)->m_value ; }
      static double floatValue_(const Object *obj) { return ((Integer*)obj)->m_value ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object *) ;
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static ObjectIter begin_(const Object *) ;
      static ObjectIter cbegin_(const Object *) ;
      static ObjectIter end_(const Object *) ;
      static ObjectIter cend_(const Object *) ;
   } ;

/************************************************************************/
/************************************************************************/

class Float : public Number
   {
   public:
      static Float *create(long val = 0) { return new Float(val) ; }
      static Float *create(double val) { return new Float(val) ; }
      static Float *create(const Object *obj) { return new Float(obj->intValue()) ; }
      static Float *create(const Float *obj) { return new Float(obj->m_value) ; }
      static Float *create(const char *) ;

   private: // static members
      static Allocator s_allocator ;
   private:
      double m_value ;
   protected: // creation/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      Float(double value = 0.0) : m_value(value) {}
      Float(const char *value) ;
      ~Float() {}

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Float> ;

      // type determination predicates
      static bool isInteger_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "Float" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *obj)
         { return new Float(static_cast<const Float&>(*obj)) ; }
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      using Number::subseq_int ;
      using Number::subseq_iter ;

      // *** destroying ***
      static void free_(Object *obj) { delete (Float*)obj ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object *obj) { delete (Float*)obj ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object *, char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *) { return 1 ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) { return obj ; }
      static const Object *front_const(const Object *obj) { return obj ; }
      static long int intValue_(const Object *obj) { return (long)((Float*)obj)->m_value ; }
      static double floatValue_(const Object *obj) { return ((Float*)obj)->m_value ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object *) ;
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static ObjectIter begin_(const Object *) ;
      static ObjectIter cbegin_(const Object *) ;
      static ObjectIter end_(const Object *) ;
      static ObjectIter cend_(const Object *) ;
   } ;

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Number> ;
extern template class FramepaC::Object_VMT<Fr::Integer> ;
extern template class FramepaC::Object_VMT<Fr::Float> ;

} ; // end namespace FramepaC

#endif /* !__Fr_NUMBER_H_INCLUDED */

// end of file number.h //
