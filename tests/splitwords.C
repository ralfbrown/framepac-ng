/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-30					*/
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
#include "framepac/charget.h"
#include "framepac/file.h"
#include "framepac/words.h"

using namespace Fr ;
using namespace std ;

typedef void output_function(WordSplitter&, char delimiter) ;
typedef void process_function(output_function*, CFile&, char in_delimiter, char out_delimiter) ;

//----------------------------------------------------------------------------

static void line_by_line(WordSplitter& splitter, char delim)
{
   StringPtr words = splitter.delimitedWords(delim) ;
   cout << words->stringValue() << endl ;
   return ;
}

//----------------------------------------------------------------------------

static void word_per_line(WordSplitter& splitter, char)
{
   while (!splitter.eof())
      {
      StringPtr word = splitter.nextWord() ;
      cout << word->stringValue() << endl ;
      }
   return  ;
}

//----------------------------------------------------------------------------

static void list_per_line(WordSplitter& splitter, char)
{
   List* words = splitter.allWords() ;
   cout << words << endl ;
   return ;
}

//----------------------------------------------------------------------------

static void process_English(output_function* fn, CFile& file, char, char out_delimiter)
{
   while (!file.eof())
      {
      char* line = file.getCLine() ;
      CharGetterCString getter(line) ;
      WordSplitterEnglish splitter(getter) ;
      fn(splitter,out_delimiter) ;
      delete[] line ;
      }
   return ;
}

//----------------------------------------------------------------------------

static void process_CSV(output_function* fn, CFile& file, char in_delimiter, char out_delimiter)
{
   while (!file.eof())
      {
      char* line = file.getCLine() ;
      CharGetterCString getter(line) ;
      WordSplitterCSV splitter(getter,in_delimiter) ;
      fn(splitter,out_delimiter) ;
      delete[] line ;
      }
   return ;
}

//----------------------------------------------------------------------------

static void process_delimited(output_function* fn, CFile& file, char in_delimiter, char out_delimiter)
{
   while (!file.eof())
      {
      char* line = file.getCLine() ;
      CharGetterCString getter(line) ;
      WordSplitterDelimiter splitter(getter,in_delimiter) ;
      fn(splitter,out_delimiter) ;
      delete[] line ;
      }
   return ;
}

//----------------------------------------------------------------------------

static void process_whitespace(output_function* fn, CFile& file, char, char out_delimiter)
{
   CharGetterFILE getter(file) ;
   WordSplitterWhitespace splitter(getter) ;
   fn(splitter,out_delimiter) ;
   return ;
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
   const char* in_CSV { nullptr } ;
   const char* in_delimited { nullptr } ;
   bool in_whitespace { false } ;
   char in_delimiter { ' ' } ;
   const char* out_normalized { nullptr } ;
   bool out_list { false } ;
   bool out_separated { false } ;
   char out_delimiter { ' ' } ;
   
   Fr::Initialize() ;
   ArgParser cmdline ;
   cmdline
      .add(in_CSV,"c","csv","DELIM\vprocess input as CSV file using DELIM as delimiter character (default is comma)",",")
      .add(in_delimited,"d","delimited","DELIM\vprocess input as a sequence of tokens delimited by DELIM (default blank)"," ")
      .add(in_whitespace,"w","whitespace","process input as a sequence of tokens delimited by arbitrary whitespace")
      .add(out_normalized,"n","normalized","DELIM\voutput results normalized to one DELIM between tokens (default blank)"," ")
      .add(out_list,"l","list","output results as a list of words for each line of input")
      .add(out_separated,"s","separate","output each token on a separate line")
      .addHelp("","longhelp","show detailed help",true) ;
   if (!cmdline.parseArgs(argc,argv))
      {
      cmdline.showHelp() ;
      return 1 ;
      }

   output_function* out_fn { line_by_line } ;
   process_function* in_fn { process_English } ;
   if (in_CSV)
      {
      in_fn = process_CSV ;
      if (*in_CSV)
	 in_delimiter = *in_CSV ;
      }
   else if (in_delimited)
      {
      in_fn = process_delimited ;
      if (*in_delimited)
	 in_delimiter = *in_delimited ;
      }
   else if (in_whitespace)
      {
      in_fn = process_whitespace ;
      }
   if (out_normalized)
      {
      if (out_normalized)
	 out_delimiter = *out_normalized ;
      }
   else if (out_list)
      {
      out_fn = list_per_line ;
      }
   else if (out_separated)
      {
      out_fn = word_per_line ;
      }
   if (argc == 1)
      {
      // get input from stdin
      CFile file(stdin) ;
      if (file)
	 in_fn(out_fn,file,in_delimiter,out_delimiter) ;
      }
   while (argc > 1)
      {
      CInputFile file(argv[1]) ;
      argv++ ;
      argc-- ;
      if (file)
	 {
	 in_fn(out_fn,file,in_delimiter,out_delimiter) ;
	 }
      }
   return 0 ;
}

// end of file splitwords.C //

