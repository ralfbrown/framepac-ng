/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-17					*/
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

#include <cstdlib>
#include <iostream>
#include "framepac/argparser.h"

using namespace Fr ;
using namespace std ;

int int_option ;
long long_option ;
size_t sizet_option ;
double double_option ;
const char* string_option ;

ArgParser cmdline ;
ArgOpt<int> i_flag(cmdline,int_option,"i","int","set integer variable, default = 5, min = -2, max = 10",
       5,-2,10) ;
ArgOpt<long> l_flag(cmdline,long_option,"l","long","set long variable, default = 7, min = -6, max = 12",
       7L,-6L,12L) ;
ArgOpt<size_t> L_flag(cmdline,sizet_option,"L","ulong","set size_t variable, default = 9, min = 1, max = 17",
       9UL,1UL,17UL) ;
ArgOpt<double> d_flag(cmdline,double_option,"d","double","set double variable, default = 1.23, min = -12.8, max = 19.01",
       1.23,-12.8,19.01) ;
ArgOpt<const char*> s_flag(cmdline,string_option,"s","string","set char* variable, default=Default","Default") ;
ArgHelp h_flag(cmdline,"h","","show brief help") ;
ArgHelp longhelp_flag(cmdline,"","longhelp", "show detailed help") ;


int main(int argc, char** argv)
{
   bool success = cmdline.parseArgs(argc,argv) ;
   cout << "parse status = " << success << endl ;
   cout << "int value = " << int_option << endl ;
   cout << "long value = " << long_option << endl ;
   cout << "size_t value = " << sizet_option << endl ;
   cout << "double value = " << double_option << endl ;
   cout << "char* value = "  << string_option << endl ;
   return success ? 0 : 1 ;
}
