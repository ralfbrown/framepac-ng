/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-08-22					*/
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

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <string>
#include <unistd.h>
#include "framepac/file.h"
#include "framepac/message.h"
#include "framepac/stringbuilder.h"
#include "framepac/texttransforms.h"

namespace Fr
{

/************************************************************************/
/*	Types								*/
/************************************************************************/

enum CompressionType
   {
      unknown,
      xz,
      gzip,
      bzip2,
      lzma,
      lzo,
      lzip,
      lrzip,
      zstd
   } ;

/************************************************************************/
/*	Helper Functions						*/
/************************************************************************/

static FILE *open_piped_input(CompressionType comp, const char *filename,
			      const char *suffix = nullptr)
{
   if (!filename || !*filename)
      return nullptr ;
   std::string fn(filename) ;
   if (suffix && *suffix)
      fn += suffix ;
   if (access(fn.c_str(),F_OK) != 0)
      return nullptr ;
   const char *pipe_command = nullptr ;
   switch (comp)
      {
      case CompressionType::xz:		pipe_command = "xz -dcq %s" ; break ;
      case CompressionType::gzip:	pipe_command = "gzip -dcq %s" ; break ;
      case CompressionType::bzip2:	pipe_command = "bzip2 -dcq %s" ; break ;
      case CompressionType::lzma:	pipe_command = "lzma -dcq %s" ; break ;
      case CompressionType::lzo:	pipe_command = "lzop -dcq %s" ; break ;
      case CompressionType::lzip:	pipe_command = "lzip -dcq %s" ; break ;
      case CompressionType::lrzip:	pipe_command = "lrzip -dq <%s" ; break ;
      case CompressionType::zstd:	pipe_command = "zstd -dcq %s" ; break ;
      default:				return nullptr ;
      }
   char *cmd = nullptr ;
   int cmdlen = asprintf(&cmd,pipe_command,fn.c_str()) ;
   (void)cmdlen ;
   FILE *fp = nullptr ;
#if defined(unix) || defined(__linux__)
   fp = popen(cmd,"r") ;
#endif
   std::free(cmd) ;
   return fp ;
}

//----------------------------------------------------------------------------

static FILE *open_piped_output(const char *pipe_command, const char *filename,
			       const char *suffix = nullptr)
{
   if (!filename || !*filename)
      return nullptr ;
   std::string fn(filename) ;
   if (suffix && *suffix)
      fn += suffix ;
   if (access(fn.c_str(),F_OK) != 0)
      return nullptr ;
   char *cmd = nullptr ;
   int cmdlen = asprintf(&cmd,pipe_command,fn.c_str()) ;
   (void)cmdlen ;
   FILE *fp = nullptr ;
#if defined(unix) || defined(__linux__)
   fp = popen(cmd,"w") ;
#endif
   std::free(cmd) ;
   return fp ;
}

//----------------------------------------------------------------------------

static bool tail_is(const char *string, size_t len, const char *tail)
{
   if (!tail || !*tail)
      return false ;
   size_t tail_len = strlen(tail) ;
   if (len > tail_len && strcmp(string+len-tail_len,tail) == 0)
      return true ;
   return false ;
}

//----------------------------------------------------------------------------

static CompressionType check_file_signature(const char *filename)
{
   FILE *fp = fopen(filename,"rb") ;
   char sigbuf[8] ;
   if (!fp)
      return CompressionType::unknown ;
   size_t siglen = fread(sigbuf,sizeof(char),sizeof(sigbuf),fp) ;
   fclose(fp) ;
   if (siglen >= 4 && memcmp(sigbuf,"\x89LZO",4) == 0)
      return CompressionType::lzo ;
   else if (siglen >= 5 && memcmp(sigbuf,"\xFD""7zXZ",5) == 0)
      return CompressionType::xz ;
   else if (siglen >= 5 && memcmp(sigbuf,"LRZI\0",5) == 0)
      return CompressionType::lrzip ;
   else if (siglen >= 5 && memcmp(sigbuf,"LZIP\1",5) == 0)
      return CompressionType::lzip ;
   else if (siglen >= 2 && memcmp(sigbuf,"\x1F\x8B",2) == 0)
      return CompressionType::lzip ;
   else if (siglen >= 4 && memcmp(sigbuf,"BZh",3) == 0 && sigbuf[3] >= '1' && sigbuf[3] <= '9')
      return CompressionType::bzip2 ;
   else if (siglen >= 4 && memcmp(sigbuf,"(\xB5/\xFD",4) == 0)
      return CompressionType::zstd ;
   // unfortunately, .lzma files don't have an official signature, so check for the most common
   //   starting bytes accompanied by the usual file extension
   else if (siglen >= 3 && memcmp(sigbuf,"\0x5D\0\0",3) == 0 && tail_is(filename,strlen(filename),".lzma"))
      return CompressionType::lzma ;
   return CompressionType::unknown ;
}

/************************************************************************/
/*	Methods for class CFile						*/
/************************************************************************/

CFile::CFile(const char *filename, bool writing, int options, CFile::OverwriteFn* prompter)
{
   if (writing)
      openWrite(filename,options,prompter) ;
   else
      openRead(filename,options) ;
   return ;
}

//----------------------------------------------------------------------------

CFile::CFile(CFile&& orig)
{
   m_file = orig.m_file ;    orig.m_file = nullptr ;
   m_tempname = orig.m_tempname ;
   m_finalname = orig.m_finalname ;
   m_errcode = orig.m_errcode ;
   m_piped = orig.m_piped ;
   m_complete = orig.m_complete ;
   m_keep_backup = orig.m_keep_backup ;
   return ;
}

//----------------------------------------------------------------------------

CFile::CFile(FILE* fileptr)
   : m_file(fileptr)
{
   return ;
}

//----------------------------------------------------------------------------

CFile::~CFile()
{
   close() ;
   return ;
}

//----------------------------------------------------------------------------

CFile& CFile::operator= (CFile* orig)
{
   if (orig)
      {
      m_file = orig->m_file ;    orig->m_file = nullptr ;
      m_tempname = orig->m_tempname ;
      m_finalname = orig->m_finalname ;
      m_errcode = orig->m_errcode ;
      m_piped = orig->m_piped ;
      m_complete = orig->m_complete ;
      m_keep_backup = orig->m_keep_backup ;
      }
   else
      {
      m_file = nullptr ;
      m_tempname = nullptr ;
      m_finalname = nullptr ;
      }
   return *this ;
}

//----------------------------------------------------------------------------

CFile& CFile::operator= (CFile& orig)
{
   m_file = orig.m_file ;    orig.m_file = nullptr ;
   m_tempname = orig.m_tempname ;
   m_finalname = orig.m_finalname ;
   m_errcode = orig.m_errcode ;
   m_piped = orig.m_piped ;
   m_complete = orig.m_complete ;
   m_keep_backup = orig.m_keep_backup ;
   return *this ;
}

//----------------------------------------------------------------------------

bool CFile::openRead(const char *filename, int options)
{
   m_piped = false ;
   if (!filename || !*filename)
      {
      m_file = nullptr ;
      return false ;
      }
   if (strcmp(filename,"-") == 0)
      {
      m_file = stdin ;
      return true ;
      }
   if (access(filename,F_OK) != 0)
      {
      // file does not exist: look for filename.xz, filename.bz2, filename.gz
      //   and open the first of those that exists
      m_file = open_piped_input(xz,filename,".xz") ;
      if (!m_file)
	 m_file = open_piped_input(bzip2,filename,".bz2") ;
      if (!m_file)
	 m_file = open_piped_input(gzip,filename,".gz") ;
      if (!m_file)
	 m_file = open_piped_input(lzma,filename,".lzma") ;
      if (!m_file)
	 m_file = open_piped_input(zstd,filename,".zstd") ;
      if (!m_file)
	 m_file = open_piped_input(lrzip,filename,".lrz") ;
      if (!m_file)
	 m_file = open_piped_input(lzo,filename,".lzo") ;
      if (!m_file)
	 m_file = open_piped_input(lzip,filename,".lz") ;
      if (m_file)
	 {
	 m_piped = true ;
	 return true ;
	 }
      return false ;
      }
   // check signature in file header
   CompressionType comp(check_file_signature(filename)) ;
   if (comp == CompressionType::unknown)
      {
      // on Unix-like systems, the 'b' modifier is a no-op, but on DOS/Windows, it tells the lib
      //   not to drop CRs
      m_file = fopen(filename,(options&binary) ? "rb" : "r") ;
      }
   else
      {
      m_file = open_piped_input(comp,filename) ;
      if (m_file)
	 m_piped = true ;
      }
   return m_file != nullptr ;
}

//----------------------------------------------------------------------------

bool CFile::openWrite(const char *filename, int options, CFile::OverwriteFn* prompter)
{
   if ((options & fail_if_exists) != 0)
      {
      // check whether the output file exists
      if (file_exists(filename))
	 {
	 // if the file exists and we were given a callback function, invoke it to determine
	 //   whether to clobber the file; if no function or the result is negative, set error code
	 //   and signal failure
	 if (!prompter || !prompter(filename))
	    {
	    m_errcode = -1 ; //FIXME
	    return false ;
	    }
	 }
      }
   if ((options & safe_rewrite) != 0)
      {
      //TODO: set up m_tempname

      }
   m_piped = false ;
   if (!filename || !*filename)
      {
      m_file = nullptr ;
      return false ;
      }
   if (strcmp(filename,"-") == 0)
      {
      m_file = stdout ;
      return true ;
      }
   size_t namelen = strlen(filename) ;
   m_file = nullptr ;
   if (tail_is(filename,namelen,".xz"))
      m_file = open_piped_output("xz -cqz >%s",filename) ;
   else if (tail_is(filename,namelen,".gz"))
      m_file = open_piped_output("gzip -c9qf >%s",filename) ;
   else if (tail_is(filename,namelen,".bz2"))
      m_file = open_piped_output("bzip2 -cq9 >%s",filename) ;
   else if (tail_is(filename,namelen,".lzma"))
      m_file = open_piped_output("lzma -cqz >%s",filename) ;
   else if (tail_is(filename,namelen,".lzo"))
      m_file = open_piped_output("lzop -qfo %s",filename) ;
   else if (tail_is(filename,namelen,".lz"))
      m_file = open_piped_output("lzip -qfo %s",filename) ;
   else if (tail_is(filename,namelen,".lrz"))
      m_file = open_piped_output("lrzip -qfo %s",filename) ;
   else if (tail_is(filename,namelen,".zstd"))
      m_file = open_piped_output("zstd -9q -o %s",filename) ;
   if (m_file)
      {
      m_piped = true ;
      }
   else
      {
      // on Unix-like systems, the 'b' modifier is a no-op, but on DOS/Windows, it tells the lib
      //   not to convert NL to CRLF
      const char* filemode = (options&binary) ? "wb" : "w" ;
      if (options & no_truncate)
	 {
	 filemode = (options & binary) ? "rb+" : "r+" ;
	 }
      m_file = fopen(filename,filemode) ;
      }
   return m_file != nullptr ;
}

//----------------------------------------------------------------------------

bool CFile::askOverwrite(const char* filename)
{
   return SystemMessage::confirmation("File %s exists.  Overwrite (Y/N)? ",filename) ;
}

//----------------------------------------------------------------------------

bool CFile::close()
{
   errno = 0 ;
   bool success = true ;
   if (m_file)
      {
      errno = 0 ;
      while (fflush(m_file) == EOF && errno == EINTR)
	 ;
      (void)fdatasync(fileno(m_file)) ;	// flush kernel buffers for file
      }
   if (!m_file || m_file == stdin || m_file == stdout)
      {
      // nothing to be done
      }
   else if (m_piped)
      {
#if defined(unix) || defined(__linux__)
      success = pclose(m_file) != -1 ;
#endif
      }
   else
      {
      success = fclose(m_file) == 0 ;
      }
   if (success)
      m_file = nullptr ;
   if (m_tempname && m_finalname)
      {
      if (m_complete)
	 {
	 // we can now rename the temporary file we've been using to the final name,
	 //   removing the previous version in the process
	 if (!safely_replace_file(m_tempname,m_finalname,m_keep_backup,false))
	    success = false ;
	 }
      else
	 {
	 // uh oh, the write got interrupted!  "revert" the output by deleting the temporary
	 //   file we used for the output and leaving the original unchanged
	 unlink(m_tempname) ;
	 }
      }
   m_tempname = nullptr ;
   m_finalname = nullptr ;
   return success && errno == 0 ;
}

//----------------------------------------------------------------------------

bool CFile::good() const
{
   return m_errcode != 0 && m_file != nullptr ;
}

//----------------------------------------------------------------------------

int CFile::error() const
{
   return m_errcode ;
}

//----------------------------------------------------------------------------

off_t CFile::filesize() const
{
   if (m_file)
      {
      off_t currpos = ftell(m_file) ;
      fseek(m_file,0,SEEK_END) ;
      off_t endpos = ftell(m_file) ;
      fseek(m_file,currpos,SEEK_SET) ;
      return endpos ;
      }
   return 0 ;
}

//----------------------------------------------------------------------------

size_t CFile::read(void *buf, size_t buflen)
{
   if (m_file)
      return fread(buf,sizeof(char),buflen,m_file) ;
   else
      return (size_t)EOF ;
}

//----------------------------------------------------------------------------

size_t CFile::read(void *buf, size_t itemcount, size_t itemsize)
{
   if (m_file)
      return fread(buf,itemsize,itemcount,m_file) ;
   else
      return (size_t)EOF ;
}

//----------------------------------------------------------------------------

int CFile::getc_nonws()
{
   int c ;
   while ((c = getc()) != EOF && isspace(c))
      {
      // discard the char
      }
   return c;
}

//----------------------------------------------------------------------------

void CFile::skipAllWS()
{
   int c ;
   while (!eof())
      {
      c = getc() ;
      if (c == EOF)
	 return;
      if (!isspace(c))
	 {
	 ungetc(c) ;
	 return ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void CFile::skipWS()
{
   int c ;
   while (!eof())
      {
      c = getc() ;
      if (c == EOF)
	 return;
      if (c == '\n' || !isspace(c))
	 {
	 ungetc(c) ;
	 return ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

size_t CFile::skipLines(size_t maxskip)
{
   size_t skipped = 0 ;
   while (!eof() && skipped < maxskip)
      {
      // skip a line
      int c ;
      for (c = getc() ; c != EOF && c != '\r' && c != '\n' ; c = getc())
	 {
	 // ignore the character
	 }
      if (c == '\r')
	 {
	 // check for CRLF sequence
	 c = getc() ;
	 if (c != EOF && c != '\n')
	    ungetc(c) ;
	 }
      skipped++ ;
      }
   return skipped ;
}

//----------------------------------------------------------------------------

size_t CFile::skipBlankLines(size_t maxskip)
{
   size_t skipped = 0 ;
   while (!eof() && skipped < maxskip)
      {
      // skip a line
      int c ;
      for (c = getc() ; c != EOF && c != '\r' && c != '\n' ; c = getc())
	 {
	 if (!isspace(c))
	    {
	    ungetc(c) ;
	    return skipped ;
	    }
	 }
      if (c == '\r')
	 {
	 // check for CRLF sequence
	 c = getc() ;
	 if (c != EOF && c != '\n')
	    ungetc(c) ;
	 }
      skipped++ ;
      }
   return skipped ;
}

//----------------------------------------------------------------------------

CharPtr CFile::getCLine(size_t maxline)
{
   if (!m_file || feof(m_file))
      return nullptr ;
   StringBuilder str(this,maxline) ;
   return str.c_str() ;
}

//----------------------------------------------------------------------------

CharPtr CFile::getTrimmedLine(size_t maxline)
{
   if (!m_file || feof(m_file))
      return nullptr ;
   skipWS() ;
   StringBuilder str(this,maxline) ;
   return trim_whitespace(str.c_str().move()) ;
}

//----------------------------------------------------------------------------

size_t CFile::write(const char *buf, size_t buflen)
{
   if (m_file)
      return fwrite(buf,sizeof(char),buflen,m_file) ;
   else
      return 0 ;
}

//----------------------------------------------------------------------------

size_t CFile::write(const void *buf, size_t itemcount, size_t itemsize)
{
   if (m_file)
      return fwrite(buf,itemsize,itemcount,m_file) ;
   else
      return 0 ;
}

//----------------------------------------------------------------------------

bool CFile::putNulls(size_t count)
{
   if (!m_file)
      return false ;

   for (size_t i = 0 ; i < count ; ++i)
      {
      if (fputc('\0',m_file) == EOF)
	 return false ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool CFile::printf(const char* fmt, ...) const
{
   if (!m_file)
      return false ;
   va_list args ;
   va_start(args,fmt) ;
   vfprintf(m_file,fmt,args) ;
   va_end(args) ;
   return true ;
}

//----------------------------------------------------------------------------

/************************************************************************/
/*	Methods for class COutputFile					*/
/************************************************************************/

COutputFile& COutputFile::operator= (COutputFile& orig)
{
   m_file = orig.m_file ;    orig.m_file = nullptr ;
   m_tempname = orig.m_tempname ;
   m_finalname = orig.m_finalname ;
   m_errcode = orig.m_errcode ;
   m_piped = orig.m_piped ;
   m_complete = orig.m_complete ;
   m_keep_backup = orig.m_keep_backup ;
   return *this ;
}

//----------------------------------------------------------------------------


} // end namespace Fr

// end of file cfile.C //
