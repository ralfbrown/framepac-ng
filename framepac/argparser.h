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


#ifndef _Fr_ARGPARSER_H_INCLUDED
#define _Fr_ARGPARSER_H_INCLUDED

namespace Fr
{

//----------------------------------------------------------------------------

class ArgOptBase ;

class ArgParser
   {
   public:
      ArgParser() ;
      ~ArgParser() ;

      void addOpt(ArgOptBase* opt) ;
      bool parseArgs(int& argc, char**& argv) ;

   protected:
      void init() ;
      ArgOptBase* matchingArg(const char* arg) const ;

   protected:
      ArgOptBase* m_options ;
      ArgParser*  m_self ;
   } ;

//----------------------------------------------------------------------------

class ArgOptBase
   {
   public;
      ArgOptBase(ArgParser&, const char* shortname const char* fullname) ;
      ~ArgOptBase() ;

      bool parse(int& argc, char**& argv) = 0 ;

   protected:
      virtual bool parseValue(const char* arg) = 0 ;
   protected:
      char* m_shortname ;
      char* m_fullname ;
   } ;

//----------------------------------------------------------------------------

template <T>
class ArgOpt : public ArgOptBase
   {
   public:
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname)
	 : ArgOptBase(parser,shortname,fullname), m_value(var)
	 {}
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, T def_value)
	 : ArgOptBase(parser,shortname,fullname), m_value(var), m_defvalue(def_value), m_have_defvalue(true)
	 {}
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, T def_value,
	     T min_value, T max_value)
	 : ArgOptBase(parser,shortname,fullname), m_value(var), m_defvalue(def_value), m_minvalue(min_value),
	   m_maxvalue(max_value), m_have_defvalue(true), m_have_minmax(true)
	 {}
      ~ArgOpt()
	 {
	    m_have_defvalue = m_have_minmax = false ;
	 }

   protected:
      virtual bool parseValue(const char* arg) ;
   protected:
      T& m_value ;
      T m_defvalue { } ;
      T m_minvalue { } ;
      T m_maxvalue { } ;
      bool m_have_defvalue { false } ;
      bool m_have_minmax { false } ;
   } ;

//----------------------------------------------------------------------------

class ArgFlag : public ArgOptBase
   {
   public:
      ArgFlag(ArgParser&, bool& flag, const char* shortname, const char*fullname) ;
      ArgFlag(ArgParser&, bool& flag, const char* shortname, const char*fullname, bool def_value) ;
      ~ArgFlag() ;

   protected:
      virtual bool parseValue(const char* argv) ;
   protected:
      bool& m_value ;
      bool  m_defvalue { false } ;
      bool  m_have_defvalue { false } ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_ARGPARSER_H_INCLUDED */

// end of file argparser.h //
