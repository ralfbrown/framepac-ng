/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-25					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#include "framepac/byteorder.h"
#include "framepac/file.h"

namespace Fr
{

/************************************************************************/
/*	Helper functions			       			*/
/************************************************************************/

static uint64_t readN(CFile& f, unsigned N, bool& OK)
{
   unsigned char buf[8] ;
   uint64_t value = 0 ;
   OK = false ;
   if (f && f.read(buf,N) == N)
      {
      OK = true ;
      for (unsigned i = 0 ; i < N ; ++i)
	 {
	 value = (value << 8) | buf[i] ;
	 }
      }
   return value ;
}

/************************************************************************/
/*	Methods for class CFile which do byte-oriented I/O		*/
/************************************************************************/

uint64_t CFile::read64LE()
{
   UInt64 val ;
   return readValue(&val) ? val.load() : 0 ;
}

//----------------------------------------------------------------------

bool CFile::read64BE(uint64_t& value)
{
   bool success ;
   value = readN(*this,8,success) ;
   return success ;
}

//----------------------------------------------------------------------

bool CFile::read64LE(uint64_t& value)
{
   UInt64 val ;
   bool success = readValue(&val) ;
   if (success) value = val.load() ;
   return success ;
}

//----------------------------------------------------------------------

uint64_t CFile::read48LE()
{
   UInt48 val ;
   return readValue(&val) ? val.load() : 0 ;
}

//----------------------------------------------------------------------

bool CFile::read48BE(uint64_t& value)
{
   bool success ;
   value = readN(*this,6,success) ;
   return success ;
}

//----------------------------------------------------------------------

bool CFile::read48LE(uint64_t& value)
{
   UInt48 val ;
   bool success = readValue(&val) ;
   if (success) value = val.load() ;
   return success ;
}

//----------------------------------------------------------------------

uint32_t CFile::read32LE()
{
   UInt32 val ;
   return readValue(&val) ? val.load() : 0 ;
}

//----------------------------------------------------------------------

bool CFile::read32BE(uint32_t& value)
{
   bool success ;
   value = readN(*this,4,success) ;
   return success ;
}

//----------------------------------------------------------------------

bool CFile::read32LE(uint32_t& value)
{
   UInt32 val ;
   bool success = readValue(&val) ;
   if (success) value = val.load() ;
   return success ;
}

//----------------------------------------------------------------------

uint32_t CFile::read24LE()
{
   UInt24 val ;
   return readValue(&val) ? val.load() : 0 ;
}

//----------------------------------------------------------------------

bool CFile::read24BE(uint32_t& value)
{
   bool success ;
   value = readN(*this,3,success) ;
   return success ;
}

//----------------------------------------------------------------------

bool CFile::read24LE(uint32_t& value)
{
   UInt32 val ;
   bool success = readValue(&val) ;
   if (success) value = val.load() ;
   return success ;
}

//----------------------------------------------------------------------

uint16_t CFile::read16LE()
{
   UInt16 val ;
   return readValue(&val) ? val.load() : 0 ;
}

//----------------------------------------------------------------------

bool CFile::read16BE(uint16_t& value)
{
   bool success ;
   value = readN(*this,2,success) ;
   return success ;
}

//----------------------------------------------------------------------

bool CFile::read16LE(uint16_t& value)
{
   UInt16 val ;
   bool success = readValue(&val) ;
   if (success) value = val.load() ;
   return success ;
}

//----------------------------------------------------------------------

uint8_t CFile::read8()
{
   uint8_t val ;
   return readValue(&val) ? val : 0 ;
}

//----------------------------------------------------------------------

bool CFile::read8(uint8_t& value)
{
   return readValue(&value) ;
}

//----------------------------------------------------------------------

bool CFile::write64BE(uint64_t value)
{
   return putc((value>>56)&0xFF) && putc((value>>48)&0xFF) && putc((value>>40)&0xFF)
      && putc((value>>32)&0xFF) && putc((value>>24)&0xFF)
      && putc((value>>16)&0xFF) && putc((value>>8)&0xFF) && putc(value&0xFF) ;
}

//----------------------------------------------------------------------

bool CFile::write64LE(uint64_t value)
{
   UInt64 val(value) ;
   return writeValue(val) ;
}

//----------------------------------------------------------------------

bool CFile::write48BE(uint64_t value)
{
   return putc((value>>40)&0xFF) && putc((value>>32)&0xFF) && putc((value>>24)&0xFF)
      && putc((value>>16)&0xFF) && putc((value>>8)&0xFF) && putc(value&0xFF) ;
}

//----------------------------------------------------------------------

bool CFile::write48LE(uint64_t value)
{
   UInt48 val(value) ;
   return writeValue(val) ;
}

//----------------------------------------------------------------------

bool CFile::write32BE(uint32_t value)
{
   return putc((value>>24)&0xFF) && putc((value>>16)&0xFF) && putc((value>>8)&0xFF) && putc(value&0xFF) ;
}

//----------------------------------------------------------------------

bool CFile::write32LE(uint32_t value)
{
   UInt32 val(value) ;
   return writeValue(val) ;
}

//----------------------------------------------------------------------

bool CFile::write24BE(uint32_t value)
{
   return putc((value>>16)&0xFF) && putc((value>>8)&0xFF) && putc(value&0xFF) ;
}

//----------------------------------------------------------------------

bool CFile::write24LE(uint32_t value)
{
   UInt24 val(value) ;
   return writeValue(val) ;
}

//----------------------------------------------------------------------

bool CFile::write16BE(uint16_t value)
{
   return putc((value>>8)&0xFF) && putc(value&0xFF) ;
}

//----------------------------------------------------------------------

bool CFile::write16LE(uint16_t value)
{
   UInt16 val(value) ;
   return writeValue(val) ;
}

//----------------------------------------------------------------------

bool CFile::write8(uint8_t value)
{
   return writeValue(value) ;
}

//----------------------------------------------------------------------

} // end namespace Fr

// end of file cfile_byte.C //


