/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
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

#ifndef Fr_BUFBUILDER_FILE_CC_INCLUDED
#define Fr_BUFBUILDER_FILE_CC_INCLUDED

#include "framepac/builder.h"
#include "framepac/file.h"
#include "framepac/message.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
bool BufferBuilder<T,minsize>::load(CFile& fp, const char* filename)
{
   int version = file_format ;
   if (!fp || !fp.verifySignature(signature,filename,version,min_file_format))
      return false ;
   uint8_t tsize ;
   if (!fp.readValue(&tsize))
      return false ;
   if (tsize != sizeof(T))
      {
      SystemMessage::error("wrong data type - sizeof() does not match") ;
      return false ;
      }
//TODO
   return true ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
bool BufferBuilder<T,minsize>::loadFromMmap(const void* mmap_base, size_t mmap_len)
{
   size_t header_size = CFile::signatureSize(signature) + sizeof(uint8_t) ;
   if (!mmap_base || mmap_len < header_size)
      return false;
//TODO
   return true ;
}

//----------------------------------------------------------------------------

template <typename T, size_t minsize>
bool BufferBuilder<T,minsize>::save(CFile& fp) const
{
   if (!fp || !fp.writeSignature(signature,file_format))
      return false ;
   uint8_t tsize = sizeof(T) ;
   if (!fp.writeValue(tsize))
      return false ;
//TODO
   return true ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !Fr_BUFBUILDER_FILE_CC_INCLUDED */

// end of file bufbuilder_file.cc //
