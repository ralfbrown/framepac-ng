/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-07-12					*/
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

#include "framepac/file.h"
#include "framepac/stringbuilder.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class CFile which use String			*/
/************************************************************************/

//----------------------------------------------------------------------------

CFile::CFile(String *filename, bool writing, int options)
{
   if (writing)
      openWrite(filename->c_str(),options) ;
   else
      openRead(filename->c_str(),options) ;
   return ;
}

//----------------------------------------------------------------------------

String* CFile::getline(size_t maxline)
{
   if (!m_file || feof(m_file))
      return nullptr ;
   StringBuilder str(this,maxline) ;
   return str.string() ;
}

} // end namespace Fr

// end of file cfile_str.C //
