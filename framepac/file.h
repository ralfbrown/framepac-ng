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
	     fail_if_exists = 2
           } ;
   public:
      CFile(const char *filename, bool writing, int options = default_options) ;
      CFile(String *filename, bool writing, int options = default_options) ;
      CFile(const CFile&) = delete ;
      ~CFile() ;
      void operator= (const CFile&) = delete ;
      operator bool () const { return m_file != nullptr ; }

      void writeComplete() { m_complete = true ; }
      bool good() const ;
      int error() const ;
      bool filtered() const { return m_piped ; }
      FILE* operator* () const { return m_file ; }
      size_t read(char *buf, size_t buflen) ;
      size_t write(const char *buf, size_t buflen) ;
      bool close() ;

   protected: // data
      FILE *m_file ;
      char *m_tempname ;	// if safe-overwrite chosen, write to temp file and
      char *m_finalname ;	// move it to the final name on close
      int   m_errcode = 0 ;
      bool  m_piped = false ;
      bool  m_complete = false ; // have we successfully finished writing?

   protected: // methods
      bool openRead(const char *filename, int options) ;
      bool openWrite(const char *filenmae, int options) ;
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
