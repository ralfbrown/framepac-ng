/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-09					*/
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

#include "framepac/argparser.h"
#include "framepac/cluster.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

int main(int argc, char** argv)
{
   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
   //TODO

   return 0 ;
}

// end of file clustertest.C //
