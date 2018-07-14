/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-13					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

namespace Fr
{

//----------------------------------------------------------------------------

bool copy_file(const char* srcname, FILE* dest)
{
   CInputFile src(srcname,CFile::binary) ;
   if (!src)
      return false ;
   size_t count ;
   char buffer[8*FrMAX_LINE] ;
   bool success = true ;
   while ((count = src.read(buffer,sizeof(buffer))) > 0)
      {
      if (fwrite(buffer,sizeof(char),count,dest) < count)
	 {
	 success = false ;
	 break ;
	 }
      }
   return success ;
}

//----------------------------------------------------------------------------

bool copy_file(const char* srcname, const char* destname)
{
   CInputFile src(srcname,CFile::binary) ;
   if (!src)
      return false ;
   COutputFile dest(destname,CFile::binary | CFile::safe_rewrite) ;
   if (!dest)
      return false ;
   size_t count ;
   char buffer[8*FrMAX_LINE] ;
   bool success = true ;
   while ((count = src.read(buffer,sizeof(buffer))) > 0)
      {
      if (dest.write(buffer,count) < count)
	 {
	 success = false ;
	 break ;
	 }
      }
   if (success)
      dest.writeComplete() ;
   return success && dest.close() ;
}


} // end namespace Fr

// end of file copyfile.C //
