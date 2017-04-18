/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-18					*/
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

#if 0
static char* duplicate_string(const char* s)
{
   if (!s) s = "" ;
   size_t len = strlen(s) ;
   char* dup = new char[len+1] ;
   memcpy(dup,s,len+1) ;
   return dup ;
}
#endif

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

void ArgParser::addOpt(ArgOptBase* opt, bool must_delete)
{
   init() ;
   opt->next(m_options) ;
   opt->mustDelete(must_delete) ;
   m_options = opt ;
   return ;
}

//----------------------------------------------------------------------------

ArgOptBase* ArgParser::matchingArg(const char* arg) const
{
   if (!arg)
      return nullptr ;
   // is this a long or a short flag name?
   if (arg[1] != '-')
      {
      arg++ ; // drop the leading hyphen
      // do we have an exact match of a short flag name? (implies that the value is in the next argv[] element)
      for (ArgOptBase* opt = m_options ; opt ; opt = opt->next())
	 {
	 if (strcmp(opt->shortName(),arg) == 0)
	    return opt ;
	 }
      // otherwise, find the longest matching short name
      size_t longest = 0 ;
      ArgOptBase* match = nullptr ;
      for (ArgOptBase* opt = m_options ; opt ; opt = opt->next())
	 {
	 const char* name = opt->shortName() ;
	 size_t matchlen = 0 ;
	 for ( ; name[matchlen] && arg[matchlen] ; ++matchlen)
	    {
	    if (name[matchlen] != arg[matchlen]) break ;
	    }
	 if (name[matchlen] == '\0' && matchlen > longest)
	    {
	    longest = matchlen ;
	    match = opt ;
	    }
	 }
      return match ;
      }
   else
      {
      // it's a long flag name, so check whether the name matches minus the optional "=value"
      const char* equal = strchr(arg,'=') ;
      size_t len = strlen(arg+2) ;
      if (equal) len = (equal - arg - 2) ;
      for (ArgOptBase* opt = m_options ; opt ; opt = opt->next())
	 {
	 if (strncmp(opt->fullName(),arg+2,len) == 0)
	    return opt ;
	 }
      }
   return nullptr ;
}

//----------------------------------------------------------------------------

bool ArgParser::parseArgs(int& argc, char**& argv)
{
   init() ;
   bool success = true ;
   while (success && argc > 1 && argv[1] != nullptr && argv[1][0] == '-')
      {
      if (strcmp(argv[1],"--") == 0)
	 {
	 // special flag which indicates that everything after it is a
	 //   non-flag option even if it starts with a dash
	 argc-- ;
	 argv++ ;
	 success = false ;
	 break  ;
	 }
      ArgOptBase* opt = matchingArg(argv[1]) ;
      if (!opt)
	 {
	 unknownOption(argv[1]) ;
	 success = false ;
	 }
      else if (!opt->parse(argc, argv))
	 {
	 // parse function reported invalid value for option, so we can just return
	 success = false ;
	 break ;
	 }
      // consume the commandline argument
      argc-- ;
      argv++ ;
      }
   return success ;
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

ArgOptBase::ArgOptBase(const char* shortname, const char* fullname, const char* desc)
{
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

//----------------------------------------------------------------------------

template <>
bool ArgOpt<bool>::convert(const char* arg, bool& value)
{
   if (!arg || !*arg)
      return false ;
   if (strcmp(arg,"0") == 0 || strcasecmp(arg,"n") == 0 || strcasecmp(arg,"no") == 0
       || strcasecmp(arg,"f") == 0 || strcasecmp(arg,"false") == 0)
      {
      value = false ;
      return true ;
      }
   if (strcmp(arg,"1") == 0 || strcasecmp(arg,"y") == 0 || strcasecmp(arg,"yes") == 0
       || strcasecmp(arg,"t") == 0 || strcasecmp(arg,"true") == 0)
      {
      value = true ;
      return true ;
      }
   return false ;
}

template <>
bool ArgOpt<int>::convert(const char* arg, int& value)
{
   char* endptr = const_cast<char*>(arg) ;
   value = (int)strtol(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<unsigned>::convert(const char* arg, unsigned& value)
{
   char* endptr = const_cast<char*>(arg) ;
   value = (unsigned)strtoul(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<long>::convert(const char* arg, long& value)
{
   char* endptr = const_cast<char*>(arg) ;
   value = strtol(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<size_t>::convert(const char* arg, size_t& value)
{
   char* endptr = const_cast<char*>(arg) ;
   value = strtoul(arg,&endptr,0) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<float>::convert(const char* arg, float& value)
{
   char* endptr = const_cast<char*>(arg) ;
   value = (float)strtod(arg,&endptr) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<double>::convert(const char* arg, double& value)
{
   char* endptr = const_cast<char*>(arg) ;
   value = strtod(arg,&endptr) ;
   return endptr != arg ;
}

template <>
bool ArgOpt<const char*>::convert(const char* arg, const char*& value)
{
   value = arg ;
   return true ;
}

//----------------------------------------------------------------------------

template <>
bool ArgOpt<bool>::parseValue(const char* arg) const
{
   if (arg == nullptr)
      {
      // toggle the flag if no value given
      m_value = !m_value ;
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
   else if (*arg)
      {
      bool value ;
      if (convert(arg,value))
	 {
	 m_value = value ;
	 return true ;
	 }
      }
   //TODO: error message: invalid value
   return false ;
}

//----------------------------------------------------------------------------

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
   T value ;
   if (!convert(arg,value))
      {
      //TODO: error message
      return false ;
      }
   if (m_have_minmax)
      {
      if (value < m_minvalue || value > m_maxvalue)
	 {
	 //TODO: error messsage
	 return false ;
	 }
      }
   m_value = value ;
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
/*	Methods for class ArgHelp					*/
/************************************************************************/

ArgHelp::ArgHelp(ArgParser& parser, const char* shortname, const char* fullname, const char* desc, bool longhelp)
   : ArgOptBase(parser,shortname,fullname,desc), m_flag(nullptr), m_long(longhelp), m_defer(false)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::ArgHelp(const char* shortname, const char* fullname, const char* desc, bool longhelp)
   : ArgOptBase(shortname,fullname,desc), m_flag(nullptr), m_long(longhelp), m_defer(false)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::ArgHelp(bool& var, const char* shortname, const char* fullname, const char* desc,
		 bool longhelp)
   : ArgOptBase(shortname,fullname,desc), m_flag(&var), m_long(longhelp), m_defer(true)
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
