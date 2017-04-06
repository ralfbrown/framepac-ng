/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-05					*/
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

#ifndef _Fr_FILE_H_INCLUDED
#define _Fr_FILE_H_INCLUDED

#include <cstdio>
#include "framepac/config.h"

namespace Fr
{

// forward declaration
class String ;

//----------------------------------------------------------------------------

class CFile
   {
   public:
      enum { default_options = 0,
	     safe_rewrite = 1,
	     fail_if_exists = 2,
	     binary = 4
           } ;
   public:
      CFile(const char *filename, bool writing, int options = default_options) ;
      CFile(String *filename, bool writing, int options = default_options) ;
      CFile(const CFile&) = delete ;
      CFile(FILE*) ;
      ~CFile() ;
      void operator= (const CFile&) = delete ;
      operator bool () const { return m_file != nullptr ; }

      void writeComplete() { m_complete = true ; }
      FILE* fp() const { return m_file ; }
      bool eof() const { return m_file ? feof(m_file) : true ; }
      bool good() const ;
      int error() const ;
      bool filtered() const { return m_piped ; }
      FILE* operator* () const { return m_file ; }
      size_t read(char* buf, size_t buflen) ;
      size_t read(void* buf, size_t itemsize, size_t itemcount) ;
      size_t write(const char* buf, size_t buflen) ;
      size_t write(const void* buf, size_t itemsize, size_t itemcount) ;
      int getc() { return fgetc(m_file) ; }
      int getc_nonws() ;
      int ungetc(int c) { return std::ungetc(c,m_file) ; }
      bool gets(char* buf, size_t buflen) { return m_file ? fgets(buf,buflen,m_file) != nullptr : false ; }
      size_t skipLines(size_t maxskip = 1) ;
      class Fr::String* getline(size_t maxline = (size_t)~0) ; // result must be freed
      char* getCLine(size_t maxline = (size_t)~0) ; // result must be freed
      void putc(char c) { fputc(c,m_file) ; }
      void puts(const char* s) { fputs(s,m_file) ; }
      [[gnu::format(gnu_printf,2,0)]] bool printf(const char* fmt, ...) const ;
      off_t tell() const { return ftell(m_file) ; }
      off_t seek(off_t loc, int whence) { return fseek(m_file,loc,whence) ; }
      void flush() { fflush(m_file) ; }
      bool close() ;

      template <typename T>
      bool readValue(T* val, size_t count = 1)
	 {
	    if (!val || count == 0) return true ; // trivially successful
	    val = Fr::New<T>(count) ;
	    if (!val) return false ;
	    return read(val,sizeof(T),count) == count ;
	 }

      template <typename T>
      bool readVarsAt(T* val, size_t count = 1)
	 {
	    if (!val || count == 0) return true ; // trivially successful
	    uint64_t offset = (uint64_t)val ;
	    val = Fr::New<T>(count) ;
	    if (!val) return false ;
	    seek(offset,SEEK_SET) ;
	    return read(val,sizeof(T),count) == count ;
	 }

      template <typename T>
      bool readOffset(T** var)
	 {
	    uint64_t offset ;
	    bool status ;
	    *var = (status = readValue(&offset)) ? reinterpret_cast<T*>(offset) : nullptr ;
	    return status ;
	 }

      template <typename T>
      bool writeValue(T& val)
	 {
	    return write(&val,sizeof(T),1) == 1 ;
	 }

      template <typename T>
      bool writeValues(T* val, size_t count = 1)
	 {
	    if (!val || count == 0) return true ; // trivially successful
	    return write(val,sizeof(T),count) == count ;
	 }

      template <typename T>
      bool writeOffset(T* array, size_t count, uint64_t& offset)
	 {
	    uint64_t ofs(0) ;
	    if (array)
	       {
	       ofs = offset ;
	       offset += count * sizeof(T) ;
	       }
	    return writeValue(ofs) ;
	 }

   protected: // methods
      bool openRead(const char *filename, int options) ;
      bool openWrite(const char *filenmae, int options) ;

   protected: // data
      FILE *m_file ;
      char *m_tempname ;	// if safe-overwrite chosen, write to temp file and
      char *m_finalname ;	// move it to the final name on close
      int   m_errcode = 0 ;
      bool  m_piped = false ;
      bool  m_complete = false ; // have we successfully finished writing?
   } ;

//----------------------------------------------------------------------------

class CInputFile : public CFile
   {
   public:
      CInputFile(const char *filename, int options = default_options) : CFile(filename,false,options) {}
      CInputFile(String *filename, int options = default_options) : CFile(filename,false,options) {}
      ~CInputFile() = default ;
   } ;

//----------------------------------------------------------------------------

class COutputFile : public CFile
   {
   public:
      COutputFile(const char *filename, int options = default_options) : CFile(filename,true,options) {}
      COutputFile(String *filename, int options = default_options) : CFile(filename,true,options) {}
      ~COutputFile() = default ;
   } ;

//----------------------------------------------------------------------------

class FilePath
   {
   public:
      FilePath(const char *pathname) ;
      FilePath(const FilePath&) = delete ; // no copying (for now)
      ~FilePath() ;

      FilePath& operator= (const FilePath&) = delete ; // no copying (for now)

      const char *path() const { return m_path ? m_path : generatePath() ; }
      const char *directory() const { return m_directory ; }
      const char *basename() const { return m_basename ? m_basename : generateBasename() ; }
      const char *root() const { return m_root ; }
      const char *extension() const { return m_extension ; }

      bool forceDirectory(const char *new_path) ;
      bool forceExtension(const char *new_extension) ;

   protected:
      char *m_directory ;
      char *m_root ;
      char *m_extension ;
      mutable char *m_path ;
      mutable char *m_basename ;
   protected:
      const char *generatePath() const ;
      const char *generateBasename() const ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_FILE_H_INCLUDED */

// end file file.h //
