/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-04					*/
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

#include "framepac/argparser.h"

using namespace Fr ;
using namespace std ;

int int_option ;
long long_option ;
const char* string_option ;

ArgParser cmdline ;
ArgOpt<int> i_flag(cmdline,int_option,"i","int","set integer variable, default = 5, min = -2, max = 10",
       5,-2,10) ;
ArgHelp h_flag(cmdline,"h","","show brief help") ;

//----------------------------------------------------------------------------

static bool parse_flag(const char* arg)
{
   cerr << "parse_flag("<<arg<<")"<<endl ;
   return true ;
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
   size_t sizet_option = 1 ;
   double double_option = 0.1 ;
   bool bool_option = false ;
   cmdline
      .add(long_option,"l","long","set long variable, min = -6, max = 12",-6L,12L)
      .add(sizet_option,"L","ulong","set size_t variable, default = 9, min = 1, max = 17",9UL,1UL,17UL)
      .add(double_option,"d","double","set double variable, no default, min = -12.8, max = 19.01",
	   -12.8,19.01)
      .add(string_option,"s","string","set char* variable, default=Default","Default")
      .add(bool_option,"b","bool","set boolean flag")
      .addFunc(parse_flag,"p","parse","pass value to function to parse")
      .addHelp("","longhelp", "show detailed help",true) ;
   bool success = cmdline.parseArgs(argc,argv) ;
   cout << "parse status = " << (success ? "OK" : "error") << endl ;
   cout << "bool value = " << (bool_option ? "true" : "false") << endl ;
   cout << "int value = " << int_option << endl ;
   cout << "long value = " << long_option << endl ;
   cout << "size_t value = " << sizet_option << endl ;
   cout << "double value = " << double_option << endl ;
   cout << "char* value = "  << (string_option ? string_option : "(null)") << endl ;
   for (int i = 1 ; i < argc ; ++i)
      {
      cout << "argv[" << i << "] = " << argv[i] << endl ;
      }
   return success ? 0 : 1 ;
}
