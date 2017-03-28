/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
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

#include <cstring>
#include "framepac/file.h"

namespace Fr
{
using namespace std ;

/************************************************************************/
/*	Methods for class FilePath					*/
/************************************************************************/

FilePath::FilePath(const char *pathname)
   : m_directory(nullptr), m_root(nullptr), m_extension(nullptr), m_path(nullptr), m_basename(nullptr)
{
   (void)pathname; //FIXME

   return ;
}

//----------------------------------------------------------------------------

FilePath::~FilePath()
{
   delete [] m_directory ;
   delete [] m_root ;
   delete [] m_extension ;
   delete [] m_path ;
   delete [] m_basename ;
   return ;
}

//----------------------------------------------------------------------------

bool FilePath::forceDirectory(const char *new_dir)
{
   if (!new_dir)
      new_dir = "." ;
   if (strcmp(new_dir,m_directory) == 0)
      return false ; // no change
   size_t len = strlen(new_dir) ;
   char *dir = new char[len+1] ;
   if (dir)
      {
      delete [] m_directory ;
      m_directory = dir ;
      strcpy(dir,new_dir) ;
      delete [] m_path ;
      m_path = nullptr ;
      return true ;
      }
   return false ;
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
   size_t len = strlen(new_ext) ;
   char *ext = new char[len+1] ;
   if (ext)
      {
      delete [] m_extension ;
      m_extension = ext ;
      strcpy(ext,new_ext) ;
      delete [] m_basename ;
      m_basename = nullptr ;
      delete [] m_path ;
      m_path = nullptr ;
      return true ;
      }
   return false ;
}

//----------------------------------------------------------------------------

const char *FilePath::generatePath() const
{
   delete [] m_path ;
   m_path = nullptr ; //FIXME   
   return m_path ;
}

//----------------------------------------------------------------------------

const char *FilePath::generateBasename() const
{
   delete [] m_basename ;
   m_basename = nullptr ;
   size_t len_r = strlen(m_root) ;
   size_t len_e = strlen(m_extension) ;
   size_t len = len_r + len_e + (len_e?1:0) ;
   char *base = new char[len+1] ;
   if (base)
      {
      memcpy(base,m_root,len_r) ;
      if (len_e)
	 {
	 base[len_r++] = '.' ;
	 memcpy(base+len_r,m_extension,len_e+1) ;
	 }
      m_basename = base ;
      }
   return m_basename ;
}

//----------------------------------------------------------------------------


} // end of namespace Fr

// end of file filename.C //
