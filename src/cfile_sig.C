/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-15					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018,2019 Carnegie Mellon University		*/
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
#include "framepac/memory.h"
#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class CFile						*/
/************************************************************************/

size_t CFile::signatureSize(const char* sigstring)
{
   if (!sigstring) return 0 ;
   return strlen(sigstring) + 1 + sizeof(uint16_t) + sizeof(uint32_t) ;
}

//----------------------------------------------------------------------------

int CFile::verifySignature(const char* sigstring)
{
   if (!sigstring || !*sigstring) return -1 ;
   size_t len = strlen(sigstring) + 1 ;
   LocalAlloc<char> buf(len) ;
   uint16_t version ;
   uint32_t byteorder ;
   if (read(buf,len) < len || !readValue(&version) || !readValue(&byteorder))
      return -1 ;
   if (memcmp(sigstring,buf,len) != 0)
      return -2 ;
   if (byteorder != 0x12345678)
      return -3 ;
   return version ;
}

//----------------------------------------------------------------------------

bool CFile::verifySignature(const char* sigstring, const char* filename, int &currver, int minver, bool silent)
{
   int ver = verifySignature(sigstring) ;
   if (ver == -1)
      {
      if (!silent)
	 SystemMessage::error("read error on %s",filename) ;
      return false ;
      }
   else if (ver == -2)
      {
      if (!silent)
	 SystemMessage::error("wrong file type for %s",filename) ;
      return false ;
      }
   else if (ver == -3)
      {
      if (!silent)
	 SystemMessage::error("file '%s' was written by a system with a different byte order",filename) ;
      return false ;
      }
   else if (ver < minver)
      {
      if (!silent)
	 SystemMessage::error("file '%s' is in an obsolete format",filename) ;
      return false ;
      }
   else if (ver > currver)
      {
      if (!silent)
	 SystemMessage::error("file '%s' is from a newer version of the program",filename) ;
      return false ;
      }
   currver = ver ;
   return true ;
}

//----------------------------------------------------------------------------

bool CFile::writeSignature(const char* sigstring, int version)
{
   if (!sigstring || !*sigstring || version < 1)
      return false ;
   size_t len = strlen(sigstring) + 1 ;
   if (write(sigstring,len) < len)
      return false ;
   uint32_t byteorder { 0x12345678 } ;
   uint16_t ver { (uint16_t)version } ;
   if (!writeValue(ver) || !writeValue(byteorder))
      return false ;
   return true ;
}

} // end namespace Fr

// end of file cfile_sig.C //
