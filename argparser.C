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
#include <cstring>
#include <iostream>
#include "framepac/argparser.h"

namespace Fr
{

using namespace std ;

static char* duplicate_string(const char* s)
{
   if (!s) s = "" ;
   size_t len = strlen(s) ;
   char* dup = new char[len+1] ;
   memcpy(dup,s,len+1) ;
   return dup ;
}

/************************************************************************/
/*	Methods for class ArgParser					*/
/************************************************************************/

ArgParser::ArgParser()
{
   init() ;
   return ;
}

//----------------------------------------------------------------------------

ArgParser::~ArgParser()
{
   while (m_options)
      {
//FIXME      
      m_options = m_options->next() ;
      }
   return ;
}

//----------------------------------------------------------------------------

void ArgParser::init()
{
   if (m_self != this)
      {
      // we haven't been initialized yet!
      m_options = nullptr ;
      m_self = this ;
      }
   return ;
}

//----------------------------------------------------------------------------

void ArgParser::addOpt(ArgOptBase* opt)
{
   init() ;
   opt->next(m_options) ;
   m_options = opt ;
   return ;
}

//----------------------------------------------------------------------------

ArgOptBase* ArgParser::matchingArg(const char* arg) const
{
   if (!arg)
      return nullptr ;
//FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

bool ArgParser::parseArgs(int& argc, char**& argv)
{
   init() ;
   while (argc > 1 && argv[1] != nullptr && argv[1][0] == '-')
      {
      if (strcmp(argv[1],"--") == 0)
	 {
	 // special flag which indicates that everything after it is a
	 //   non-flag option even if it starts with a dash
	 argc-- ;
	 argv++ ;
	 break  ;
	 }
      ArgOptBase* opt = matchingArg(argv[1]) ;
      if (!opt)
	 {
	 unknownOption(argv[1]) ;
	 return false ;
	 }
      if (!opt->parse(argc, argv))
	 {
	 // parse function reported invalid value for option, so we can just return
	 return false ;
	 }
      // consume the commandline argument
      argc-- ;
      argv++ ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ArgParser::unknownOption(const char*name) const
{
   cerr << "unknown option " << name << endl ; //TODO: better message
   return false ;
}

//----------------------------------------------------------------------------

bool ArgParser::showHelp(bool longhelp) const
{
   if (longhelp)
      {
      }
   else
      {
      }
   return false ;
}

/************************************************************************/
/*	Methods for class ArgOptBase					*/
/************************************************************************/

ArgOptBase::ArgOptBase(ArgParser& parser, const char* shortname, const char* fullname, const char* desc)
{
   parser.addOpt(this) ;
   if (!shortname) shortname = "" ;
   if (!fullname) fullname = "" ;
   if (!desc) desc = "" ;
   m_shortname = shortname ;
   m_fullname = fullname ;
   m_description = desc ;
   return ;
}

//----------------------------------------------------------------------------

ArgOptBase::~ArgOptBase()
{
   return ;
}

//----------------------------------------------------------------------------

bool ArgOptBase::parse(int& argc, char**& argv) const
{
   if (argv[1][0] != '-')
      return false ;
   char* arg ;
   if (argv[1][1] == '-')
      {
      // we have a long name
      arg = strchr(argv[1],'=') ;  // is it --longflag=value ?
      if (arg)
	 arg++ ;
      else if (optional())
	 {
	 // optional values can't be separated by blanks, since we
	 //   can't tell whether the next element of argv[] is the
	 //   value or a non-flag argument
	 arg = nullptr ;
	 }
      else if (argc > 1)
	 {
	 argc-- ;
	 argv++ ;
	 arg = argv[1] ;
	 }
      else
	 return false ;
      }
   else
      {
      // process a short name
      size_t len = strlen(m_shortname) ;
      if (len == 0 || strncmp(m_shortname,argv[1]+1,len) != 0)
	 return false ;
      arg = argv[1]+len+1 ;
      if (*arg)
	 {
	 // got the value, nothing to do
	 }
      else if (optional())
	 {
	 // optional values can't be separated by blanks, since we
	 //   can't tell whether the next element of argv[] is the
	 //   value or a non-flag argument
	 arg = nullptr ;
	 }
      else if (argc > 1)
	 {
	 argc-- ;
	 argv++ ;
	 arg = argv[1] ;
	 }
      else
	 return false ;
      }
   return parseValue(arg) ;
}

/************************************************************************/
/*	Methods for class ArgOpt					*/
/************************************************************************/

template <typename T>
bool ArgOpt<T>::parseValue(const char* arg) const
{
   if (arg == nullptr)
      {
      if (m_have_defvalue)
	 {
	 m_value = m_defvalue ;
	 return true ;
	 }
      else
	 {
	 //TODO: error message: no default value
	 return false ;
	 }
      }
//FIXME

   return false ;
}

//----------------------------------------------------------------------------

template <>
bool ArgOpt<char*>::parseValue(const char* arg) const
{
   if (arg == nullptr)
      {
      if (m_have_defvalue)
	 {
	 m_value = duplicate_string(m_defvalue) ;
	 return true ;
	 }
      else
	 {
	 //TODO: error message: no default value
	 return false ;
	 }
      }
   m_value = duplicate_string(arg) ;
   return true ;
}

//----------------------------------------------------------------------------

// explicit instantiations of the common types
template class ArgOpt<int> ;
template class ArgOpt<unsigned> ;
template class ArgOpt<long> ;
template class ArgOpt<size_t> ;
template class ArgOpt<float> ;
template class ArgOpt<double> ;
template class ArgOpt<const char*> ;

/************************************************************************/
/*	Methods for class ArgFlag					*/
/************************************************************************/

ArgFlag::ArgFlag(ArgParser& parser, bool& var, const char* shortname, const char* fullname, const char* desc)
   : ArgOptBase(parser,shortname,fullname,desc), m_value(var)
{
   return ;
}

//----------------------------------------------------------------------------

ArgFlag::ArgFlag(ArgParser& parser, bool& var, const char* shortname, const char* fullname,
		 const char* desc, bool def_value)
   : ArgOptBase(parser,shortname,fullname,desc), m_value(var), m_defvalue(def_value), m_have_defvalue(true)
{
   return ;
}

//----------------------------------------------------------------------------

bool ArgFlag::parseValue(const char* arg) const
{
   if (arg == nullptr)
      {
      // toggle the flag if no value given
      m_value = !m_value ;
      return true ;
      }
   else if (*arg == '+' || strcasecmp(arg,"yes") == 0 || strcasecmp(arg,"true") == 0 || *arg == '1')
      {
      m_value = true ;
      return true ;
      }
   else if (*arg == '-' || strcasecmp(arg,"no") == 0 || strcasecmp(arg,"false") == 0 || *arg == '0')
      {
      m_value = false ;
      return true ;
      }
   else if (*arg == '=' || strcasecmp(arg,"default") == 0)
      {
      if (m_have_defvalue)
	 {
	 m_value = m_defvalue ;
	 return true ;
	 }
      }
   //TODO: error message: invalid value
   return false ;
}

/************************************************************************/
/*	Methods for class ArgHelp					*/
/************************************************************************/

ArgHelp::ArgHelp(ArgParser& parser, const char* shortname, const char* fullname, const char* desc, bool longhelp)
   : ArgOptBase(parser,shortname,fullname,desc), m_flag(nullptr), m_long(longhelp), m_defer(false)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::ArgHelp(ArgParser& parser, bool& var, const char* shortname, const char* fullname, const char* desc,
		 bool longhelp)
   : ArgOptBase(parser,shortname,fullname,desc), m_flag(&var), m_long(longhelp), m_defer(true)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::~ArgHelp()
{
   return ;
}

//----------------------------------------------------------------------------

bool ArgHelp::parseValue(const char* arg) const
{
   if (arg != nullptr)
      {
      }
//TODO
   return true ;
}


} // end namespace Fr

// end of file argparser.C //
