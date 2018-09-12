/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.10, last edit 2018-09-04					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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

#include <cstring>
#include <sstream>
#include "framepac/argparser.h"
#include "framepac/cstring.h"
#include "framepac/texttransforms.h"

namespace Fr
{

using namespace std ;

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

bool ArgOptBase::setDefaultValue()
{
   return false ;
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
   if (!arg || !*arg)
      {
      if (setDefaultValue())
	 {
	 return true ;
	 }
      if (!m_value_set)
	 {
	 //FIXME: error--no default available
	 return false ;
	 }
      }
   bool success = convert(arg) ;
   if (!success)
      {
      invalidValue(arg) ;
      }
   if (!validateValue())
      {
      invalidValue(arg) ;
      success = false ;
      }
   return success ;
}

//----------------------------------------------------------------------------

void ArgOptBase::invalidValue(const char* opt) const
{
   cerr << "Invalid value '" << opt << "' for " ;
   if (shortName())
      {
      cerr << "-" << shortName() ;
      if (fullName())
	 {
	 cerr << " (--" << fullName() << ")" ;
	 }
      }
   else if (fullName())
      cerr << "--" << fullName() ;
   cerr << endl ;
   CharPtr range { describeRange() } ;
   if (range)
      {
      cerr << "valid range is " << *range << endl ;
      }
   return ;
}

/************************************************************************/
/*	Methods for class ArgHelp					*/
/************************************************************************/

ArgHelp::ArgHelp(ArgParser& parser, const char* shortname, const char* fullname, const char* desc, bool longhelp)
   : ArgOptBase(parser,shortname,fullname,desc), m_flag(nullptr), m_long(longhelp)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::ArgHelp(const char* shortname, const char* fullname, const char* desc, bool longhelp)
   : ArgOptBase(shortname,fullname,desc), m_flag(nullptr), m_long(longhelp)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::ArgHelp(bool& var, const char* shortname, const char* fullname, const char* desc,
		 bool longhelp)
   : ArgOptBase(shortname,fullname,desc), m_flag(&var), m_long(longhelp)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::ArgHelp(ArgParser& parser, bool& var, const char* shortname, const char* fullname, const char* desc,
		 bool longhelp)
   : ArgOptBase(parser,shortname,fullname,desc), m_flag(&var), m_long(longhelp)
{
   return ;
}

//----------------------------------------------------------------------------

ArgHelp::~ArgHelp()
{
   return ;
}

//----------------------------------------------------------------------------

bool ArgHelp::showHelp()
{
   if (m_parser)
      {
      return m_parser->showHelp(isLongHelp()) ;
      }
   return false ;
}

//----------------------------------------------------------------------------

bool ArgHelp::setDefaultValue()
{
   if (m_flag)
      {
      *m_flag = true ;
      m_value_set = true ;
      return true ;
      }
   showHelp() ;
   return false ;
}

//----------------------------------------------------------------------------

bool ArgHelp::convert(const char* arg, const char* /*delim*/)
{
   (void)arg ;
//TODO
   if (m_flag) *m_flag = true ;
   return true ;
}

//----------------------------------------------------------------------------

bool ArgHelp::validateValue()
{
   return true ;
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
      auto tmp = m_options ;
      m_options = m_options->next() ;
      delete tmp ;
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

ArgParser& ArgParser::repeatable(const char* delim)
{
   if (!delim) delim = "," ;
   m_options->repeatable(delim) ;
   return *this ;
}

//----------------------------------------------------------------------------

void ArgParser::addOpt(ArgOptBase* opt, bool must_delete)
{
   init() ;
   opt->next(m_options) ;
   opt->mustDelete(must_delete) ;
   opt->setParser(this) ;
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
	 if (opt->shortName() && strcmp(opt->shortName(),arg) == 0)
	    return opt ;
	 }
      // otherwise, find the longest matching short name
      size_t longest = 0 ;
      ArgOptBase* match = nullptr ;
      for (ArgOptBase* opt = m_options ; opt ; opt = opt->next())
	 {
	 const char* name = opt->shortName() ;
	 if (!name) continue ;
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
	 if (opt->fullName() && strncmp(opt->fullName(),arg+2,len) == 0)
	    return opt ;
	 }
      }
   return nullptr ;
}

//----------------------------------------------------------------------------

void ArgParser::reverseArgList()
{
   if (!m_finalized)
      {
      // adding args with .add() etc left the chain in the reverse order from which we want to use it,
      //   so reverse the list
      ArgOptBase* prev = nullptr ;
      ArgOptBase* curr = m_options ;
      while (curr)
	 {
	 ArgOptBase* next = curr->next() ;
	 curr->next(prev) ;
	 prev = curr ;
	 curr = next ;
	 }
      m_options = prev ;
      m_finalized = true ;
      }
   return ;
}

//----------------------------------------------------------------------------

bool ArgParser::parseArgs(int& argc, char**& argv, bool show_help_on_error)
{
   init() ;
   reverseArgList() ;
   bool success = true ;
   while (success && argc > 1 && argv[1] != nullptr && argv[1][0] == '-')
      {
      if (strcmp(argv[1],"--") == 0)
	 {
	 // special flag which indicates that everything after it is a
	 //   non-flag option even if it starts with a dash
	 argc-- ;
	 argv++ ;
	 return true ;
	 }
      if (strcmp(argv[1],"-\t") == 0)
	 {
	 // special value which indicates an option which has already
	 //   been processed in a previous pass
	 argc-- ;
	 argv++ ;
	 continue ;
	 }
      ArgOptBase* opt = matchingArg(argv[1]) ;
      if (!opt)
	 {
	 unknownOption(argv[1]) ;
	 if (show_help_on_error)
	    showHelp() ;
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

void ArgParser::showBanner(ostream& out) const
{
   if (m_banner && *m_banner)
      {
      out << m_banner << endl ;
      }
   return  ;
}

//----------------------------------------------------------------------------

static size_t widest_flag(const ArgOptBase* options)
{
   size_t width = 1 ;
   for ( ; options ; options = options->next())
      {
      size_t meta = 0 ;
      const char* desc = options->description() ;
      if (desc)
	 {
	 const char* meta_end = strchr(desc,'\v') ;
	 if (meta_end)
	    {
	    meta = meta_end - desc ;
	    //desc = meta_end + 1 ;
	    }
	 }
      if (options->shortName() && *options->shortName())
	 {
	 size_t w = strlen(options->shortName()) + 2 + meta ;
	 if (meta && options->haveDefault())
	    w++ ;
	 width = std::max(w,width) ;
	 }
      if (options->fullName() && *options->fullName())
	 {
	 if (meta) ++meta ;
	 size_t w = strlen(options->fullName()) + 2 + meta ;
	 width = std::max(w,width) ;
	 }
      }
   return width ;
}

//----------------------------------------------------------------------------

static void show_arg_name(const char* arg, bool full, bool have_def, const char* meta, size_t pad_to)
{
   if (!arg || !*arg)
      return  ;
   cout << "  -" ;
   size_t width = 0 ;
   if (full)
      {
      cout << "-" ;
      ++width ;
      }
   cout << arg ;
   width += strlen(arg) ;
   if (meta)
      {
      const char* meta_end = strchr(meta,'\v') ;
      if (meta_end)
	 {
	 const char* closing = "" ;
	 ++width ;
	 if (full)
	    {
	    cout << '=' ;
	    }
	 else if (have_def)
	    {
	    cout << '[' ;
	    closing = "]" ;
	    ++width ;
	    }
	 else
	    {
	    cout << ' ' ;
	    }
	 width += meta_end - meta ;
	 for ( ; meta < meta_end ; ++meta)
	    {
	    cout << *meta ;
	    }
	 cout << closing ;
	 }
      }
   while (width++ < pad_to)
      {
      cout << ' ' ;
      }
   return ;
}

//----------------------------------------------------------------------------

static bool show_arg(const char* arg, bool full, bool have_def, size_t column_width,
   const char* desc, const char* meta)
{
   if (!arg || !*arg)
      return false ;
   show_arg_name(arg,full,have_def,meta,column_width) ;
   if (desc && *desc)
      {
      const char* meta_end = strchr(desc,'\v') ;
      if (meta_end)
	 desc = meta_end + 1 ;
      cout << desc ;
      }
   return true ;
}

//----------------------------------------------------------------------------

bool ArgParser::showHelp(bool longhelp) const
{
   if (m_showed_help)
      return false ;
   m_showed_help = true ;
   if (m_banner)
      cout << m_banner << endl ;
   const char* argv0 = m_argv0 ? m_argv0 : "program" ;
   cout << "Usage: " << argv0 <<  " {options} " ;
   if (m_usage)
      cout << m_usage ;
   cout << endl  ;
   cout << "\nAvailable option flags:" << endl ;
   size_t col_width = widest_flag(m_options) ;
   for (auto opt = m_options ; opt ; opt = opt->next())
      {
      const char* desc = opt->description() ;
      if (!desc) desc = "(no description available)" ;
      bool have_def = opt->haveDefault() ;
      if (longhelp)
	 {
	 show_arg_name(opt->shortName(),false,have_def,desc,0) ;
	 if (opt->shortName() && opt->fullName())
	    cout <<  "," ;
	 show_arg_name(opt->fullName(),true,have_def,desc,0) ;
	 if (strchr(desc,'\v'))
	    desc = strchr(desc,'\v')+1 ;
	 cout << "\n\t" << desc << '\n' ;
	 auto def_desc = opt->describeDefault() ;
	 auto cur_desc = opt->describeCurrent() ;
	 auto range_desc = opt->describeRange() ;
	 if (def_desc)
	    {
	    cout << "\tDefault: " << def_desc ;
	    }
	 if (cur_desc)
	    {
	    cout << "\tCurrent: " << cur_desc ;
	    }
	 if (range_desc)
	    {
	    cout << "\tRange: " << range_desc << endl ;
	    }
	 else if (def_desc || cur_desc)
	    cout << endl ;
	 }
      else if (opt->shortName() && opt->fullName())
	 {
#if 0
	 if (show_arg(opt->shortName(),false,have_def,col_width,nullptr,desc))
	    cout << endl ;
	 if (show_arg(opt->fullName(),true,have_def,col_width,desc,desc))
	    cout << endl ;
#else
	 show_arg(opt->shortName(),false,have_def,col_width,desc,desc) ;
	 cout << endl ;
#endif
	 }
      else
	 {
	 if (show_arg(opt->shortName(),false,have_def,col_width,desc,desc))
	    cout << endl ;
	 if (show_arg(opt->fullName(),true,have_def,col_width,desc,desc))
	    cout << endl ;
	 }
      }
   return true ;
}


} // end namespace Fr

// end of file argparser.C //
