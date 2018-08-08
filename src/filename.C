/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-07					*/
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

#include <cstring>
#include "framepac/file.h"
#include "framepac/texttransforms.h"

namespace Fr
{
using namespace std ;

/************************************************************************/
/*	Methods for class FilePath					*/
/************************************************************************/

FilePath::FilePath(const char *pathname)
   : m_directory(nullptr), m_root(nullptr), m_extension(nullptr), m_path(nullptr), m_basename(nullptr)
{
   if (!pathname) pathname = "" ;
   const char *last_slash = strrchr(pathname,'/') ;
   if (last_slash)
      {
      size_t len = last_slash - pathname + 1 ;
      if (len > 1) --len ;  // swallow trailing slash
      m_directory = dup_string(pathname, len) ;
      pathname = last_slash + 1 ;
      }
   const char *last_period = strrchr(pathname,'.') ;
   if (last_period)
      {
      size_t len = last_period - pathname ;
      m_root = dup_string(pathname,len) ;
      m_extension = dup_string(last_period) ;
      }
   else
      {
      m_root = dup_string(pathname) ;
      m_extension = nullptr ;
      }
   return ;
}

//----------------------------------------------------------------------------

bool FilePath::forceDirectory(const char *new_dir)
{
   if (!new_dir)
      new_dir = "." ;
   if (strcmp(new_dir,m_directory) == 0)
      return false ; // no change
   CharPtr dir { dup_string(new_dir) } ;
   if (dir)
      {
      m_directory = dir.move() ;
      m_path = nullptr ;
      return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

bool FilePath::defaultDirectory(const char* dir)
{
   return (m_directory && **m_directory) ? true : forceDirectory(dir) ;
}

//----------------------------------------------------------------------------

bool FilePath::forceExtension(const char *new_ext)
{
   if (!new_ext)
      new_ext = "" ;
   if (new_ext[0] == '.')
      new_ext++ ;
   if (strcmp(new_ext,m_extension) == 0)
      return false ; // no change
   CharPtr ext { dup_string(new_ext) } ;
   if (ext)
      {
      m_extension = ext.move() ;
      m_basename = nullptr ;
      m_path = nullptr ;
      return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

bool FilePath::defaultExtension(const char* ext)
{
   return (m_extension && **m_extension) ? true : forceExtension(ext) ;
}

//----------------------------------------------------------------------------

const char *FilePath::generatePath() const
{
   m_path = nullptr ; //FIXME   
   return m_path ;
}

//----------------------------------------------------------------------------

const char *FilePath::generateBasename() const
{
   m_basename = nullptr ;
   size_t len_r = strlen(m_root) ;
   size_t len_e = strlen(m_extension) ;
   size_t len = len_r + len_e + (len_e?1:0) ;
   char *base = new char[len+1] ;
   if (base)
      {
      memcpy(base,*m_root,len_r) ;
      if (len_e)
	 {
	 base[len_r++] = '.' ;
	 memcpy(base+len_r,*m_extension,len_e+1) ;
	 }
      m_basename = base ;
      }
   return m_basename ;
}

//----------------------------------------------------------------------------


} // end of namespace Fr

// end of file filename.C //
