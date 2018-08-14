/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-27					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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

#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>
#include "framepac/file.h"
#include "framepac/mmapfile.h"
using namespace std ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

MemMappedFile::MemMappedFile(const char *filename, off_t start_offset,
			     off_t length, bool readonly)
   : m_address(nullptr), m_length(0)
{
   if (!filename || !*filename)
      return ;
   CInputFile file(filename) ;
   if (file)
      init(fileno(*file),start_offset,length,readonly) ;
   return ;
}

//----------------------------------------------------------------------------

MemMappedFile::MemMappedFile(CFile &file, off_t start_offset,
			     off_t length, bool readonly)
   : m_address(nullptr), m_length(0)
{
   FILE *fp = *file ;
   if (fp)
      init(fileno(fp),start_offset,length,readonly) ;
   return ;
}

//----------------------------------------------------------------------------

void MemMappedFile::init(int fd, off_t start_offset, off_t length, bool readonly)
{
   if (length == ~0)
      {
      off_t currpos = lseek(fd,0,SEEK_CUR) ;
      off_t filelen = lseek(fd,0,SEEK_END) ;
      lseek(fd,currpos,SEEK_SET) ;
      length = filelen - start_offset ;
      }
   int prot = readonly ? PROT_READ : PROT_READ|PROT_WRITE ;
   m_address = (char*)mmap(nullptr,length,prot,MAP_SHARED,fd,start_offset) ;
   if (m_address == MAP_FAILED)
      {
      m_address = nullptr ;
      m_length = 0 ;
      }
   else
      {
      m_length = length ;
      }
   return ;
}

//----------------------------------------------------------------------------

MemMappedFile::~MemMappedFile()
{
   if (m_address)
      (void)munmap(m_address,m_length) ;
   return ;
}

//----------------------------------------------------------------------------

bool MemMappedFile::willNeed(void *start, size_t len)
{
#ifdef __linux__
   return madvise(start,len,MADV_WILLNEED) == 0 ;
#else
   return posix_madvise(start,len,POSIX_MADV_WILLNEED) == 0 ;
#endif
}

//----------------------------------------------------------------------------

bool MemMappedFile::doneUsing(void *start, size_t len)
{
#ifdef __linux__
   return madvise(start,len,MADV_DONTNEED) == 0 ;
#else
   return posix_madvise(start,len,POSIX_MADV_DONTNEED) == 0 ;
#endif
}

//----------------------------------------------------------------------------

bool MemMappedFile::defaultAccess(void *start, size_t len)
{
#ifdef __linux__
   return madvise(start,len,MADV_NORMAL) == 0 ;
#else
   return posix_madvise(start,len,POSIX_MADV_NORMAL) == 0 ;
#endif
}

//----------------------------------------------------------------------------

bool MemMappedFile::sequentialAccess(void *start, size_t len)
{
#ifdef __linux__
   return madvise(start,len,MADV_SEQUENTIAL) == 0 ;
#else
   return posix_madvise(start,len,POSIX_MADV_SEQUENTIAL) == 0 ;
#endif
}

//----------------------------------------------------------------------------

bool MemMappedFile::randomAccess(void *start, size_t len)
{
#ifdef __linux__
   return madvise(start,len,MADV_RANDOM) == 0 ;
#else
   return posix_madvise(start,len,POSIX_MADV_RANDOM) == 0 ;
#endif
}

//----------------------------------------------------------------------------

bool MemMappedFile::flush(void *start, size_t len, bool sync)
{
   return msync(start,len,sync ? MS_SYNC : MS_ASYNC) == 0 ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file mmapfile.C //
