/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-01					*/
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

#include <iostream>

namespace Fr
{

//----------------------------------------------------------------------------

class ArgParser ;

class ArgOptBase
   {
   public:
      ArgOptBase(const char* shortname, const char* fullname, const char* desc) ;
      ArgOptBase(ArgParser&, const char* shortname, const char* fullname, const char* desc) ;
      ~ArgOptBase() ;

      bool parse(int& argc, char**& argv) ;

      void invalidValue(const char* opt) const ;

      ArgOptBase* next() const { return m_next ; }
      void next(ArgOptBase* nxt) { m_next = nxt ; }

      void mustDelete(bool del) { m_must_delete = del ; }
      bool mustDelete() const { return m_must_delete ; }

      const char* shortName() const { return m_shortname ; }
      const char* fullName() const { return m_fullname ; }
      const char* description() const { return m_description ; }
      char* describeDefault() const ;
      char* describeRange() const ;

      void setParser(ArgParser* p) { m_parser = p ; }
      ArgParser* getParser() const { return m_parser ; }

   protected:
      virtual bool convert(const char*) = 0 ;
      virtual bool setDefaultValue() ;
      virtual bool validateValue() = 0 ;
      virtual bool optional() const { return false ; } ;
      virtual void describeDefault(std::ostream&) const { return ; }
      virtual void describeRange(std::ostream&) const { return ; }
   protected:
      ArgParser*  m_parser { nullptr } ;
      ArgOptBase* m_next { nullptr } ;
      const char* m_shortname ;
      const char* m_fullname ;
      const char* m_description ;
      bool        m_must_delete ;
      bool        m_have_defvalue { false } ;
      bool        m_have_minmax { false } ;
      bool	  m_value_set { false } ;
   } ;

//----------------------------------------------------------------------------

template <typename Callable>
class ArgOptFunc : public ArgOptBase
   {
   public:
      ArgOptFunc(ArgParser& parser, Callable& fn, const char* shortname, const char* fullname, const char* desc)
	 : ArgOptBase(parser,shortname,fullname,desc), m_func(fn)
	 {
	 m_func = &fn ; 
	 }
      ArgOptFunc(Callable& fn, const char* shortname, const char* fullname, const char* desc)
	 : ArgOptBase(shortname,fullname,desc), m_func(fn)
	 {
	 m_func = &fn ; 
	 }
      ~ArgOptFunc() {}

   protected:
      virtual bool convert(const char* arg) { return m_func(arg) ; }
      virtual bool setDefaultValue() { return false ; }
      virtual bool validateValue() { return true ; }
   protected:
      Callable* m_func ;
   } ;

//----------------------------------------------------------------------------

template <typename T>
class ArgOpt : public ArgOptBase
   {
   public:
      ArgOpt(T& var, const char* shortname, const char* fullname, const char* desc)
	 : ArgOptBase(shortname,fullname,desc), m_value(var)
	 {}
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var)
	 {}
      ArgOpt(T& var, const char* shortname, const char* fullname, const char* desc, T def_value)
	 : ArgOptBase(shortname,fullname,desc), m_value(var), m_defvalue(def_value)
	 { m_have_defvalue = true ; }
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc, T def_value)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var), m_defvalue(def_value)
	 { m_have_defvalue = true ; }
      ArgOpt(T& var, const char* shortname, const char* fullname, const char* desc,
	     T min_value, T max_value)
	 : ArgOptBase(shortname,fullname,desc), m_value(var), m_minvalue(min_value),
	   m_maxvalue(max_value)
	 { m_have_minmax = true ; }
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc,
	     T min_value, T max_value)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var), m_minvalue(min_value),
	   m_maxvalue(max_value)
	 { m_have_minmax = true ; }
      ArgOpt(T& var, const char* shortname, const char* fullname, const char* desc, T def_value,
	     T min_value, T max_value)
	 : ArgOptBase(shortname,fullname,desc), m_value(var), m_defvalue(def_value), m_minvalue(min_value),
	   m_maxvalue(max_value)
	 { m_have_defvalue = m_have_minmax = true ; }
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc, T def_value,
	     T min_value, T max_value)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var), m_defvalue(def_value), m_minvalue(min_value),
	   m_maxvalue(max_value)
	 { m_have_defvalue = m_have_minmax = true ; }
      ~ArgOpt()
	 {
	    m_have_defvalue = m_have_minmax = false ;
	 }

   protected:
      virtual bool convert(const char* arg) ;
      virtual bool setDefaultValue() ;
      virtual bool validateValue() ;
      virtual bool optional() const { return m_have_defvalue ; }
      virtual void describeDefault(std::ostream& s) const ;
      virtual void describeRange(std::ostream& s) const ;
   protected:
      T&         m_value ;
      T          m_defvalue { } ;
      T          m_minvalue { } ;
      T          m_maxvalue { } ;
   } ;


// library provides instantiations for the common variable types
template<> bool ArgOpt<bool>::convert(const char*) ;
template<> bool ArgOpt<bool>::optional() const ;
template<> bool ArgOpt<bool>::setDefaultValue() ;
template<> bool ArgOpt<bool>::validateValue() ;
extern template class ArgOpt<bool> ;
template<> bool ArgOpt<int>::convert(const char*) ;
extern template class ArgOpt<int> ;
template<> bool ArgOpt<long>::convert(const char*) ;
extern template class ArgOpt<long> ;
template<> bool ArgOpt<unsigned>::convert(const char*) ;
extern template class ArgOpt<unsigned> ;
template<> bool ArgOpt<size_t>::convert(const char*) ;
extern template class ArgOpt<size_t> ;
template<> bool ArgOpt<float>::convert(const char*) ;
extern template class ArgOpt<float> ;
template<> bool ArgOpt<double>::convert(const char*) ;
extern template class ArgOpt<double> ;
template<> bool ArgOpt<const char*>::convert(const char*) ;
extern template class ArgOpt<const char*> ;

//----------------------------------------------------------------------------

class ArgHelp : public ArgOptBase
   {
   public:
      ArgHelp(const char* shortname, const char* fullname, const char* desc, bool longhelp = false) ;
      ArgHelp(ArgParser&, const char* shortname, const char* fullname, const char* desc, bool longhelp = false) ;
      ArgHelp(bool& flag, const char* shortname, const char* fullname, const char* desc, bool longhelp = false) ;
      ArgHelp(ArgParser&, bool& flag, const char* shortname, const char* fullname, const char* desc, bool longhelp = false) ;
      ~ArgHelp() ;

      bool isLongHelp() const { return m_long ; }
      bool showHelp() ;

   protected:
      virtual bool convert(const char*) ;
      virtual bool setDefaultValue() ;
      virtual bool validateValue() ;
      virtual bool optional() const { return true ; }
   protected:
      bool* m_flag ;
      bool  m_long  ;
   } ;

//----------------------------------------------------------------------------

class ArgParser
   {
   public:
      ArgParser() ;
      ~ArgParser() ;

      ArgParser& add(bool& var, const char* shortname, const char* fullname, const char* desc)
	 { addOpt(new ArgOpt<bool>(var,shortname,fullname,desc),true) ; return *this ; }
      ArgParser& add(bool& var, const char* shortname, const char* fullname, const char* desc, bool def_value)
	 { addOpt(new ArgOpt<bool>(var,shortname,fullname,desc,def_value),true) ; return *this ; }
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc)
	 { addOpt(new ArgOpt<T>(var,shortname,fullname,desc),true) ; return *this ; }
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc, T def_value)
	 { addOpt(new ArgOpt<T>(var,shortname,fullname,desc,def_value),true) ; return *this ; }
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc,
		     T min_value, T max_value)
	 { addOpt(new ArgOpt<T>(var,shortname,fullname,desc,min_value,max_value),true) ; return *this ; }
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc, T def_value,
		     T min_value, T max_value)
	 { addOpt(new ArgOpt<T>(var,shortname,fullname,desc,def_value,min_value,max_value),true) ; return *this ; }
      ArgParser& addHelp(const char* shortname, const char* fullname, const char* desc, bool longhelp = false)
	 { addOpt(new ArgHelp(shortname,fullname,desc,longhelp),true) ; return *this ; }
      template <typename Callable>
      ArgParser& addFunc(Callable& fn, const char* shortname, const char* fullname, const char* desc)
	 { addOpt(new ArgOptFunc<Callable>(fn,shortname,fullname,desc),true) ; return *this ; }

      void addOpt(ArgOptBase* opt, bool must_delete = false) ;
      bool parseArgs(int& argc, char**& argv, bool show_help_on_error = false) ;

      bool unknownOption(const char* name) const ;
      bool showHelp(bool longhelp = false) const ;

   protected:
      void init() ;
      ArgOptBase* matchingArg(const char* arg) const ;

   protected:
      ArgOptBase* m_options ;
      ArgParser*  m_self ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_ARGPARSER_H_INCLUDED */

// end of file argparser.h //
