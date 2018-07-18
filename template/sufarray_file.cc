/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-17					*/
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

#include "framepac/file.h"
#include "framepac/message.h"
#include "framepac/sufarray.h"

namespace Fr {

/************************************************************************/
/*	Methods for template class SuffixArray				*/
/************************************************************************/

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::load(CFile& fp)
{
   int version = file_format ;
   if (!fp || !fp.verifySignature(signature,filename,version,min_file_format))
      return false ;
   uint8_t idsize, idxsize ;
   if (!fp.readValue(&idsize) || !fp.readValue(&idxsize))
      return false ;
   if (idsize != sizeof(IdT) || idxsize != sizeof(IdxT))
      {
      SystemMessage::error("wrong data type - sizeof() does not match") ;
      return false ;
      }
      
   //TODO
   return false ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::load(void* mmap_base, size_t mmap_len)
{
   if (!mmap_base || mmap_len == 0)
      return false ;
   //TODO
   return false ;
}

//----------------------------------------------------------------------------

template <typename IdT, typename IdxT>
bool SuffixArray<IdT,IdxT>::save(CFile& fp) const
{
   if (!fp || !fp.writeSignature(signature,file_format))
      return false ;
   uint8_t idsize = sizeof(IdT) ;
   uint8_t idxsize = sizeof(IdxT) ;
   if (!fp.writeValue(idsize) || !fp.writeValue(idxsize))
      return false ;
   //TODO
   return false ;
}

//----------------------------------------------------------------------------


} // end of namespace Fr

// end of file sufarray_file.cc //
