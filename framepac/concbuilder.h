/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-15					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#ifndef _Fr_CONCBUILDER_H_INCLUDED
#define _Fr_CONCBUILDER_H_INCLUDED

#include <mutex>
#include "framepac/builder.h"

/************************************************************************/
/************************************************************************/

namespace Fr {

// a buffer builder that can be used concurrently by multiple threads
//   this has the same interface as BufferBuilder, but adds
//   serialization to the update functions (which naturally slows
//   things down, so use only when multiple threads are needed)
template <typename T, size_t minsize = 200>
class ConcurrentBufferBuilder : public BufferBuilder<T,minsize>
   {
   public:
      typedef BufferBuilder<T,minsize> super ;
   public:
      ConcurrentBufferBuilder() : super() {}
      ConcurrentBufferBuilder(const ConcurrentBufferBuilder&) = delete ;
      ~ConcurrentBufferBuilder() {}
      void operator= (const ConcurrentBufferBuilder&) = delete ;

      bool read(const char*&) ;
      bool read(char*& input) { return read(const_cast<char*&>(input)) ; }
      void append(T value) ;
      void append(const super& value) ;
      void remove() ;

      // operator overloads
      T *operator * () const { return this->m_buffer ; }
      ConcurrentBufferBuilder &operator += (T value) { append(value) ; return *this ; }
      ConcurrentBufferBuilder &operator += (const super& buf) { append(buf) ; return *this ; }

   protected:
      std::mutex m_mutex ;
   } ;

extern template class BufferBuilder<char> ;

} // end namespace Fr

#endif /* !_Fr_BUILDER_H_INCLUDED */

// end of file builder.h //
