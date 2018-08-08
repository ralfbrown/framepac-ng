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

#ifndef _Fr_FILE_H_INCLUDED
#define _Fr_FILE_H_INCLUDED

#include <cstdarg>
#include <cstdio>
#include "framepac/config.h"
#include "framepac/cstring.h"

namespace Fr
{

// forward declaration
class CString ;
class List ;
class String ;

//----------------------------------------------------------------------------

class LineBatch
   {
   public:
      typedef char* LineEditFunc(char* line, std::va_list args) ;
   public:
      LineBatch(size_t init_capacity = 0) ;
      ~LineBatch() ;

      size_t capacity() const { return m_capacity ; }
      size_t size() const { return m_count ; }

      // iterator support
      char** begin() const { return m_lines ; }
      const char** cbegin() const { return const_cast<const char**>(m_lines) ; }
      char** end() const { return m_lines + m_count ; }
      const char** cend() const { return const_cast<const char**>(m_lines + m_count) ; }

      void clear() ;
      bool append(char* ln) ;
      bool append(CharPtr&& ln) ;
      LineBatch& operator+= (char* ln) { (void)append(ln) ; return *this ; }

      const char* line(size_t N) const { return (N < size()) ?  m_lines[N] : nullptr ; }
      const char* operator[] (size_t N) const { return m_lines[N] ; }

      bool applyVA(LineEditFunc* fn, std::va_list args) ;
      bool apply(LineEditFunc* fn, ...)
	 {
	    std::va_list args ;
	    va_start(args,fn) ;
	    bool status = applyVA(fn,args) ;
	    va_end(args) ;
	    return status ;
	 }

   protected:
      bool expandTo(size_t newsize) ;

   protected:
      size_t m_capacity ;
      size_t m_count ;
      char** m_lines ;
   } ;

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
      explicit operator bool () const { return m_file != nullptr ; }
      bool operator! () const { return m_file == nullptr ; }

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
      void skipWS() ;   	// skip whitespace within a single line
      void skipAllWS() ;	// skip until a non-whitespace char is seen, may skip multiple lines
      size_t skipLines(size_t maxskip = 1) ;
      size_t skipBlankLines(size_t maxskip = (size_t)~0) ;
      class Fr::String* getline(size_t maxline = (size_t)~0) ; // result must be freed
      CharPtr getCLine(size_t maxline = (size_t)~0) ; // result must be freed
      CharPtr getTrimmedLine(size_t maxline = (size_t)~0) ; // result must be freed
      LineBatch* getLines(size_t batchsize = 0) ;
      LineBatch* getLines(size_t batchsize, int mono_skip) ;
      void putc(char c) { fputc(c,m_file) ; }
      void puts(const char* s) { fputs(s,m_file) ; }
      void putlines(const LineBatch* batch) ;
      [[gnu::format(gnu_printf,2,0)]] bool printf(const char* fmt, ...) const ;
      off_t tell() const { return ftell(m_file) ; }
      bool seek(off_t loc, int whence = SEEK_SET) { return fseek(m_file,loc,whence) == 0 ; }
      void flush() { fflush(m_file) ; }
      bool close() ;

      void writeJSON(const class List*, int indent, bool recursive) ;

      static size_t signatureSize(const char* sigstring) ;
      int verifySignature(const char* sigstring) ;
      // returns format version stored in header, -1 on read error, -2 if wrong signature, -3 if wrong endianness
      bool verifySignature(const char* sigstring, const char* filename, int& currver, int minver) ;
      bool writeSignature(const char* sigstring, int version) ;

      template <typename T>
      bool readValue(T* val)
	 {
	    if (!val) return true ; // trivially successful
	    return read(val,sizeof(T),1) == 1 ;
	 }

      // on success, updated first argument to point at allocated array of values read; array must be
      //   released with delete[]
      template <typename T>
      bool readValues(T** val, size_t count)
	 {
	    if (!val || count == 0) return true ; // trivially successful
	    *val = new T[count] ;
	    if (!*val) return false ;
	    if (read(*val,sizeof(T),count) == count)
	       return true ;
	    else
	       {
	       delete[] *val ;
	       *val = nullptr ;
	       return false ;
	       }
	 }

      template <typename T>
      bool readVarsAt(uint64_t offset, T** val, size_t count = 1)
	 {
	    if (!val || count == 0) return true ; // trivially successful
	    if (!seek(offset,SEEK_SET))
	       return false ;
	    return readValues(val,count) ;
	 }

      bool readStringArray(CString*& strings, size_t& count) ;

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

      bool writeStringArray(const CString* strings, size_t count) ;

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

      CFile& operator<< (char c) { putc(c) ; return *this ; }
      CFile& operator<< (const char* s) { puts(s) ; return *this ; }
      CFile& operator<< (long i) { printf("%ld",i) ; return *this ; }
      CFile& operator<< (size_t u) { printf("%lu",u) ; return *this ; }
      CFile& operator<< (double d) { printf("%g",d) ; return *this ; }

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
      typedef CFile super ;
   public:
      CInputFile(const char *filename, int options = default_options) : super(filename,false,options) {}
      CInputFile(String *filename, int options = default_options) : super(filename,false,options) {}
      ~CInputFile() = default ;
   } ;

//----------------------------------------------------------------------------

class COutputFile : public CFile
   {
   public:
      typedef CFile super ;
   public:
      COutputFile(const char *filename, int options = default_options) : super(filename,true,options) {}
      COutputFile(String *filename, int options = default_options) : super(filename,true,options) {}
      ~COutputFile() = default ;
   } ;

//----------------------------------------------------------------------------

class FilePath
   {
   public:
      FilePath(const char *pathname) ;
      FilePath(const FilePath&) = delete ; // no copying (for now)
      ~FilePath() = default ;

      FilePath& operator= (const FilePath&) = delete ; // no copying (for now)

      const char *path() const { return m_path ? *m_path : generatePath() ; }
      const char *directory() const { return m_directory ; }
      const char *basename() const { return m_basename ? *m_basename : generateBasename() ; }
      const char *root() const { return m_root ; }
      const char *extension() const { return m_extension ; }

      bool pathOnly() const { return *m_root == nullptr && *m_extension == nullptr ; }

      bool defaultDirectory(const char *path) ;
      bool forceDirectory(const char *new_path) ;
      bool defaultExtension(const char *extension) ;
      bool forceExtension(const char *new_extension) ;

   protected:
      CharPtr m_directory ;
      CharPtr m_root ;
      CharPtr m_extension ;
      mutable CharPtr m_path ;
      mutable CharPtr m_basename ;
   protected:
      const char *generatePath() const ;
      const char *generateBasename() const ;
   } ;

//----------------------------------------------------------------------------

List* load_file_list(bool use_stdin, const char* listfile, const char* what = nullptr,
   bool terminate_on_error = true) ;

bool copy_file(const char* srcname, const char* destname) ;
bool copy_file(const char* srcname, FILE* dest) ;

} // end namespace Fr

#endif /* !_Fr_FILE_H_INCLUDED */

// end file file.h //
