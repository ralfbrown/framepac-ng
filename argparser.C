/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-04-14					*/
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

#include "framepac/argparser.h"


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
      ArgOptBase* opt = matchingArg(argv[1]) ;
      if (!opt)
	 {
	 // TODO: report unknown option

	 return false ;
	 }
      if (!opt->parse(argc, argv))
	 {
	 // TODO: report invalid value for option

	 return false ;
	 }
      // consume the commandline argument
      argc-- ;
      argv++ ;
      }
   return true ;
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
   size_t len = strlen(shortname) ;
   m_shortname = new char[len+1] ;
   memcpy(m_shortname,shortname,len+1) ;
   len = strlen(fullname) ;
   m_fullname = new char[len+1] ;
   memcpy(m_fullname,fullname,len+1) ;
   len = strlen(desc) ;
   m_description = new char[len+1] ;
   memcpy(m_description,desc,len+1) ;
   return ;
}

//----------------------------------------------------------------------------

ArgOptBase::~ArgOptBase()
{
   delete[] m_shortname ;  m_shortname = nullptr ;
   delete[] m_fullname ;   m_fullname = nullptr ;
   delete[] m_description ; m_description = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

bool ArgOptBase::parse(int& argc, char**& argv)
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

template <T>
bool ArgOpt<T>::parseValue(const char* arg)
{
//FIXME
   (void)arg;
   return false ;
}

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

bool ArgFlag::parseValue(const char* arg)
{
//FIXME
   (void)arg;

   return false ;
}

/************************************************************************/
/*	Methods for class ArgHelp					*/
/************************************************************************/

ArgHelp::ArgHelp(ArgParser& parser, const char* shortname, const char* fullname, const char* desc, bool long)
   : ArgOptBase(parser,shortname,fullname,desc), m_flag(nullptr), m_long(long), m_defer(false)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::ArgHelp(ArgParser& parser, bool& var, const char* shortname, const char* fullname, const char* desc,
		 bool long)
   : ArgOptBase(parser,shortname,fullname,desc), m_flag(&var), m_long(long), m_defer(true)
{
   return ;
}

// end of file argparser.C //
