/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-23					*/
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

#ifndef _Fr_MMAPFILE_H_INCLUDED
#define _Fr_MMAPFILE_H_INCLUDED

#include <cstdio>
using namespace std ;

namespace Fr
{

class MemMappedROFile ;

class MemMappedFile
   {
   public:
      MemMappedFile() : m_address(nullptr), m_length(0) {}
      MemMappedFile(const char *filename, off_t start_offset = 0, off_t length = ~0, bool readonly = false) ;
      MemMappedFile(class CFile &, off_t start_offset = 0, off_t length = ~0, bool readonly = false) ;
      MemMappedFile(const MemMappedFile&) = delete ;
      MemMappedFile(MemMappedFile&& orig)
	 {
	 m_address = orig.m_address ;
	 m_length = orig.m_length ;
	 orig.m_address = nullptr ;
	 orig.m_length = 0 ;
	 }
      ~MemMappedFile() ;
      MemMappedFile& operator= (const MemMappedFile& orig) = delete ;
      MemMappedFile& operator= (MemMappedFile&& orig)
	 {
	 m_address = orig.m_address ;
	 m_length = orig.m_length ;
	 orig.m_address = nullptr ;
	 orig.m_length = 0 ;
	 return *this ;
	 }
      // instantiate a file mapping into a default-constructed instance
      void open(const char* filename, off_t start_offset = 0, off_t length = ~0, bool readonly = false) ;

      size_t size() const { return m_length ; }
      explicit operator bool () const { return m_address != nullptr ; }
      char* operator* () const { return m_address ; }
      char& operator[] (size_t N) { return m_address[N] ; }
      const char& operator[] (size_t N) const { return m_address[N] ; }

      static bool willNeed(void *start, size_t len) ;
      static bool doneUsing(void *start, size_t len) ;
      static bool defaultAccess(void *start, size_t len) ;
      bool defaultAccess() { return defaultAccess(m_address,m_length) ; }
      static bool sequentialAccess(void *start, size_t len) ;
      bool sequentialAccess() { return sequentialAccess(m_address,m_length) ; }
      static bool randomAccess(void *start, size_t len) ;
      bool randomAccess() { return randomAccess(m_address,m_length) ; }
      static bool flush(void *start, size_t len, bool synchronous = false) ;
      bool flush(bool synchronous = false) { return flush(m_address,m_length,synchronous) ; }
   protected:
      char      *m_address ;
      size_t     m_length ;
   protected:
      void init(int fd,off_t start_offset, off_t length, bool readonly) ;
   } ;

//----------------------------------------------------------------------------

class MemMappedROFile : public MemMappedFile
   {
   public: // types
      typedef MemMappedFile super ;
   public:
      MemMappedROFile() = default ;
      MemMappedROFile(const char *filename, off_t start_offset = 0,
		    off_t length = ~0)
	 : MemMappedFile(filename,start_offset,length,true) {}
      MemMappedROFile(class CFile &f, off_t start_offset = 0,
		    off_t length = ~0) 
	 : MemMappedFile(f,start_offset,length,true) {}
      ~MemMappedROFile() {}

      const char* operator* () const { return m_address ; }
      const char& operator[] (size_t N) const { return m_address[N] ; }
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_MMAPFILE_H_INCLUDED */

// end of file mmapfile.h //

