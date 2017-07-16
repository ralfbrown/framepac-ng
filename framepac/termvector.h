/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.02, last edit 2017-07-16					*/
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

#ifndef __FrTERMVECTOR_H_INCLUDED
#define __FrTERMVECTOR_H_INCLUDED

#include "framepac/vector.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

class TermCountVector : public SparseVector<uint32_t,uint32_t>
   {
   public:
      static TermCountVector* create() { return new TermCountVector ; }

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      TermCountVector() : SparseVector<uint32_t,uint32_t>()
	 {
	 }

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<TermCountVector> ;

      // type determination predicates
      static bool isTermVector_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "TermCountVector" ; }

   private: // static members
      static Allocator s_allocator ;
   } ;

//----------------------------------------------------------------------------

class TermVector : public SparseVector<uint32_t,float>
   {
   public:
      static TermVector* create() { return new TermVector ; }

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      TermVector() : SparseVector<uint32_t,float>()
	 {
	 }

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<TermVector> ;

      // type determination predicates
      static bool isTermVector_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "TermVector" ; }

   private: // static members
      static Allocator s_allocator ;
   } ;

} ; // end of namespace Fr

#endif /* !__FrVECTOR_H_INCLUDED */

// end of termvector.h //
