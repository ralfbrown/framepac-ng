/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.14, last edit 2019-02-13					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018,2019 Carnegie Mellon University			*/
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

#include "framepac/argparser.h"
#include "framepac/file.h"
#include "framepac/spelling.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

int main(int argc, char** argv)
{
   bool match_exactly { false } ;
   const char* filename { nullptr } ;

   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(match_exactly,"x","exact","use exact letter matches only")
      .add(filename,"f","cognates","use cognate scores from FILE")
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1;
      }
   auto cd = CognateData::defaultInstance() ;
   if (filename)
      {
      cd = CognateData::load(filename) ;
      }
   CFile in(stdin) ;
   while (auto line = in.getTrimmedLine())
      {
      if (!line || !*line)
	 continue ;
      const char* lineptr { line } ;
      auto src { Object::create(lineptr) } ;
      auto trg { Object::create(lineptr) } ;
      if (src && trg)
	 {
	 auto srcstr { src->stringValue() } ;
	 auto trgstr { trg->stringValue() } ;
	 auto score = cd->score(srcstr,trgstr,match_exactly) ;
	 cout << '"' << srcstr << "\" -> \"" << trgstr << "\" = " << score << endl ;
	 }
      }
   if (cd == CognateData::defaultInstance())
      cd->reset() ;
   else
      delete cd ;
   return 0 ;
}

// end of file cogscore.C //
