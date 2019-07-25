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

#ifndef _Fr_FILE_H_INCLUDED
#define _Fr_FILE_H_INCLUDED

#include <cstdarg>
#include <cstdio>
#include "framepac/config.h"
#include "framepac/objectvmt.h"
#include "framepac/smartptr.h"

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
      size_t inputBytes() const { return m_inputbytes ; }

      // iterator support
      char** begin() const { return m_lines ; }
      const char** cbegin() const { return const_cast<const char**>(m_lines) ; }
      char** end() const { return m_lines + m_count ; }
      const char** cend() const { return const_cast<const char**>(m_lines + m_count) ; }

      void clear() ;
      bool append(char* ln) ;
      bool append(CharPtr&& ln) ;
      LineBatch& operator+= (char* ln) { (void)append(ln) ; return *this ; }
      void inputBytes(size_t b) { m_inputbytes = b ; }
      void addInput(size_t b) { m_inputbytes += b ; }

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
      size_t m_inputbytes ;
      char** m_lines ;
   } ;

//----------------------------------------------------------------------------

class CFile
   {
   public:
      enum { default_options = 0,
	     safe_rewrite = 1,
	     fail_if_exists = 2,
	     binary = 4,
	     no_truncate = 8
           } ;
   public:
      CFile() = default ;
      CFile(const char *filename, bool writing, int options = default_options) ;
      CFile(String *filename, bool writing, int options = default_options) ;
      CFile(const CFile&) = delete ;
      CFile(CFile&& orig) ;
      CFile(FILE*) ;
      ~CFile() ;
      void operator= (const CFile&) = delete ;
      CFile& operator= (CFile*) ;  // move semantics!
      CFile& operator= (CFile&) ;  // move semantics!
      explicit operator bool () const { return m_file != nullptr ; }
      bool operator! () const { return m_file == nullptr ; }

      void writeComplete() { m_complete = true ; }	// indicate that write of new file was successful
      void discardBackup() { m_keep_backup = false ; } // delete original version instead of moving to *.bak
      FILE* fp() const { return m_file ; }
      bool eof() const { return m_file ? feof(m_file) : true ; }
      bool good() const ;
      int error() const ;
      bool filtered() const { return m_piped ; }
      FILE* operator* () const { return m_file ; }
      size_t read(void* buf, size_t buflen) ;
      size_t read(void* buf, size_t itemcount, size_t itemsize) ;
      size_t write(const char* buf, size_t buflen) ;
      size_t write(const void* buf, size_t itemcount, size_t itemsize) ;
      int getc() { return fgetc(m_file) ; }
      int getc_nonws() ;
      int ungetc(int c) { return std::ungetc(c,m_file) ; }
      bool gets(char* buf, size_t buflen) { return m_file ? fgets(buf,buflen,m_file) != nullptr : false ; }
      void skipWS() ;   	// skip whitespace within a single line
      void skipAllWS() ;	// skip until a non-whitespace char is seen, may skip multiple lines
      size_t skipLines(size_t maxskip = 1) ;
      size_t skipBlankLines(size_t maxskip = (size_t)~0) ;
      Fr::Ptr<Fr::String> getline(size_t maxline = (size_t)~0) ; // result must be freed
      CharPtr getCLine(size_t maxline = (size_t)~0) ; // result must be freed
      CharPtr getTrimmedLine(size_t maxline = (size_t)~0) ; // result must be freed
      LineBatch* getLines(size_t batchsize = 0) ;
      LineBatch* getLines(size_t batchsize, int mono_skip) ;
      bool putc(char c) { return fputc(c,m_file) != EOF ; }
      void puts(const char* s) { fputs(s,m_file) ; }
      void putlines(const LineBatch* batch) ;
      bool putNulls(size_t count) ;
      [[gnu::format(gnu_printf,2,0)]] bool printf(const char* fmt, ...) const ;
      off_t tell() const { return ftell(m_file) ; }
      bool seek(off_t loc, int whence = SEEK_SET) { return fseek(m_file,loc,whence) == 0 ; }
      void flush() { fflush(m_file) ; }
      bool close() ;

      void writeJSON(const class List*, int indent, bool recursive) ;

      static size_t signatureSize(const char* sigstring) ;
      int verifySignature(const char* sigstring) ;
      // returns format version stored in header, -1 on read error, -2 if wrong signature, -3 if wrong endianness
      bool verifySignature(const char* sigstring, const char* filename, int& currver, int minver, bool silent = false);
      bool writeSignature(const char* sigstring, int version) ;

      // byte-oriented I/O
      uint64_t read64LE() ;
      uint64_t read48LE() ;
      uint32_t read32LE() ;
      uint32_t read24LE() ;
      uint16_t read16LE() ;
      uint8_t read8() ;
      bool read64LE(uint64_t&) ;
      bool read48LE(uint64_t&) ;
      bool read32LE(uint32_t&) ;
      bool read24LE(uint32_t&) ;
      bool read16LE(uint16_t&) ;
      bool read8(uint8_t&) ;
      bool read64BE(uint64_t&) ;
      bool read48BE(uint64_t&) ;
      bool read32BE(uint32_t&) ;
      bool read24BE(uint32_t&) ;
      bool read16BE(uint16_t&) ;
      bool write64LE(uint64_t) ;
      bool write48LE(uint64_t) ;
      bool write32LE(uint32_t) ;
      bool write24LE(uint32_t) ;
      bool write16LE(uint16_t) ;
      bool write8(uint8_t) ;
      bool write64BE(uint64_t) ;
      bool write48BE(uint64_t) ;
      bool write32BE(uint32_t) ;
      bool write24BE(uint32_t) ;
      bool write16BE(uint16_t) ;

      template <typename T>
      bool readValue(T* val)
	 {
	    if (!val) return true ; // trivially successful
	    return read(val,1,sizeof(T)) == 1 ;
	 }

      // on success, updated first argument to point at allocated array of values read; array must be
      //   released with delete[]
      template <typename T>
      bool readValues(T** val, size_t count)
	 {
	    if (!val || count == 0) return true ; // trivially successful
	    *val = new T[count] ;
	    if (!*val) return false ;
	    if (read(*val,count,sizeof(T)) == count)
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
	    return write(&val,1,sizeof(T)) == 1 ;
	 }

      template <typename T>
      bool writeValues(T* val, size_t count = 1)
	 {
	    if (!val || count == 0) return true ; // trivially successful
	    return write(val,count,sizeof(T)) == count ;
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
      FILE* m_file        { nullptr } ;
      CharPtr m_tempname  { nullptr } ;	// if safe-overwrite chosen, write to temp file and
      CharPtr m_finalname { nullptr } ; // move it to the final name on close
      int   m_errcode     { 0 } ;
      bool  m_piped       { false } ;	// are we piping through an external process?
      bool  m_complete    { false } ;   // have we successfully finished writing?
      bool  m_keep_backup { true } ;	// should we retain original file as a backup?
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

bool file_exists(const char *filename) ;
bool is_writeable_directory(const char *filename) ;

off_t file_size(FILE*) ;
off_t file_size(const char* filename) ;

bool create_path(const char* path) ;

bool copy_file(const char* srcname, const char* destname) ;
bool copy_file(const char* srcname, FILE* dest) ;

bool safely_replace_file(const char *tempname, const char *filename, bool keep_backup = true,
   bool print_errmsg = false) ;

} // end namespace Fr

#endif /* !_Fr_FILE_H_INCLUDED */

// end file file.h //
