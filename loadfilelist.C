/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-13					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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
#include "framepac/list.h"
#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

List* load_file_list(bool use_stdin, const char* listfile, const char* what, bool terminate_on_error)
{
   ListBuilder files ;
   if (use_stdin)
      files += "-" ;
   else
      {
      if (!listfile || !*listfile)
	 listfile = "-" ;
      CInputFile listfp(listfile) ;
      if (!listfp)
	 {
	 if (!what) what = "list of files to process" ;
	 SystemMessage::error("Unable to open '%s' to get %s!",listfile,what) ;
	 if (terminate_on_error)
	    exit(1) ;
	 return nullptr ;
	 }
      char* filename ;
      while (!listfp.eof() && (filename = listfp.getTrimmedLine()) != nullptr)
	 {
	 if (*filename && *filename != '#' && *filename != ';')
	    files += filename ;
	 delete[] filename ;
	 }
      }
   return files.move() ;
}

} //end namespace Fr

// end of file loadfilelist.C //
