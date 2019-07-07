/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-01					*/
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

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "framepac/cstring.h"
#include "framepac/file.h"
#include "framepac/message.h"
#include "framepac/texttransforms.h"

/************************************************************************/
/*	Manifest Constants						*/
/************************************************************************/

#ifndef O_BINARY
#  define O_BINARY 0
#endif

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/************************************************************************/

//----------------------------------------------------------------------

bool is_writeable_directory(const char *filename)
{
   return (filename && *filename && access(filename,X_OK | W_OK) == 0) ;
}

//----------------------------------------------------------------------

bool file_exists(const char* filename)
{
   return (filename && *filename && access(filename,F_OK) == 0) ;
}

//----------------------------------------------------------------------

off_t file_size(int fd)
{
   if (fd == EOF)
      return 0 ;
   off_t pos = lseek(fd,0L,SEEK_CUR) ;
   off_t end = lseek(fd,0L,SEEK_END) ;	// find file's size
   lseek(fd,pos,SEEK_SET) ;		// return to original position
   return end ;
}

//----------------------------------------------------------------------

off_t file_size(FILE* fp)
{
   // note: while fstat() could be used here, calling the lseek-based
   //   file_size(fd) will reflect the correct size even if the file
   //   has been written to without the directory entry being updated
   //   (as happens, e.g. under Windows)
   return fp ? file_size(fileno(fp)) : 0 ;
}

//----------------------------------------------------------------------

off_t file_size(const char* filename)
{
   if (!filename || !*filename)
      return 0 ;
   int fd = open(filename,O_RDONLY|O_BINARY) ;
   off_t size = file_size(fd) ;
   if (fd != EOF)
      close(fd) ;
   return size ;
}

//----------------------------------------------------------------------

static const char *find_last_component(const char *filepath)
{
   if (!filepath)
      return nullptr ;
   const char *end = strchr(filepath,'\0')-1 ;
   const char *start = filepath ;
#ifdef FrMSDOS_PATHNAMES
   if (*filepath && filepath[1] == ':')
      start = filepath + 2 ;
#endif /* FrMSDOS_PATHNAMES */
   while (end > start && *end != '/'
#ifdef FrMSDOS_PATHNAMES
	  && *end != '\\'
#endif /* FrMSDOS_PATHNAMES */
	 )
      end-- ;
   if (*end == '/' || *end == '\\')
      end++ ;
   return end ;
}

//----------------------------------------------------------------------

static CharPtr strip_last_component(const char *filepath,
				  bool keep_slash = false)
{
   const char *end = find_last_component(filepath) ;
   if (!keep_slash && end > filepath && (end[-1] == '/' || end[-1] == '\\'))
      end-- ;
   int len = end-filepath ;
   if (len == 0)
      {
      len = 1 ;
      filepath = "." ;
      }
   return Fr::dup_string(filepath,len) ;
}

//----------------------------------------------------------------------

bool create_path(const char* path)
{
   if (is_writeable_directory(path)) // does directory already exist?
      return true ;			 // if yes, we were successful
   auto parent = strip_last_component(path) ;
   if (parent && *parent)
      {
      // does parent directory exist?  if not, try to create it
      if (!is_writeable_directory(parent) && !create_path(parent))
	 {
	 return false ;			// bail out if unable to create parent
	 }
      }
   if (*find_last_component(path) == '\0') // trailing slash?
      return true ;
   return mkdir(path,0755) != -1 ;
}

//----------------------------------------------------------------------

CharPtr FileDirectory(const char *filename)
{
   (void)filename;

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------

CharPtr force_filename_ext(const char *filename, const char *ext)
{
   if (!filename)
      filename = "" ;
   size_t extlen = ext ? strlen(ext) : 0 ;
   size_t namelen = strlen(filename) ;
   CharPtr fullname = Fr::New<char>(namelen+extlen+2) ;
   memcpy(*fullname,filename,namelen+1) ;
   // strip off any existing extension
   char *slash = strrchr(*fullname,'/') ;
#ifdef FrMSDOS_PATHNAMES
   char *backslash = strrchr(*fullname,'\\') ;
   if (backslash && (!slash || backslash > slash))
      slash = backslash ;
#endif /* FrMSDOS_PATHNAMES */
   if (!slash)
      slash = *fullname ;
   char *period = strrchr(slash,'.') ;
   if (period)
      *period = '\0' ;
   // and add the new extension
   if (ext)
      {
      strcat(*fullname,".") ;
      strcat(*fullname,ext) ;
      }
   return fullname ;
}

//----------------------------------------------------------------------

bool safely_replace_file(const char *tempname, const char *filename, bool keep_backup, bool print_errmsg)
{
   if (!filename || !tempname || !*filename || !*tempname)
      {
      errno = EINVAL ;
      return false ;
      }
   auto bakname = force_filename_ext(filename,"bak") ;
   bool success = false ;
   if (bakname)
      {
      (void)unlink(*bakname) ;
      errno = 0 ;
      if (file_exists(filename) && rename(filename,*bakname) != 0)
	 {
	 if (print_errmsg)
	    {
	    SystemMessage::error("renaming %s to %s%s",filename,*bakname,errno==EACCES?"  (access denied)":"") ;
	    }
	 }
      else if (rename(tempname,filename) != 0)
	 {
	 if (print_errmsg)
	    {
	    SystemMessage::error("renaming temporary file to %s%s",filename,errno==EACCES?"  (access denied)":"") ;
	    }
	 }
      else
	 success = (keep_backup || !file_exists(*bakname) || unlink(*bakname) == 0) ;
      }
   else
      {
      SystemMessage::no_memory("replacing file with a new version") ;
      errno = ENOMEM ;
      }
   return success ;
}

} // end namespace Fr


// end of file filemanip.C //
