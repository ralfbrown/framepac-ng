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

#ifndef _Fr_ARGPARSER_H_INCLUDED
#define _Fr_ARGPARSER_H_INCLUDED

#include <cstdint>

namespace Fr
{

//----------------------------------------------------------------------------

class ArgParser ;

class ArgOptBase
   {
   public:
      ArgOptBase(ArgParser&, const char* shortname, const char* fullname, const char* desc) ;
      ~ArgOptBase() ;

      bool parse(int& argc, char**& argv) const ;

      ArgOptBase* next() const { return m_next ; }
      void next(ArgOptBase* nxt) { m_next = nxt ; }

      void mustDelete(bool del) { m_must_delete = del ; }
      bool mustDelete() const { return m_must_delete ; }

      const char* shortName() const { return m_shortname ; }
      const char* fullName() const { return m_fullname ; }
      const char* description() const { return m_description ; }

   protected:
      virtual bool parseValue(const char* arg) const = 0 ;
      virtual bool optional() const { return false ; } ;
   protected:
      ArgOptBase* m_next ;
      const char* m_shortname ;
      const char* m_fullname ;
      const char* m_description ;
      bool        m_must_delete ;
   } ;

//----------------------------------------------------------------------------

template <typename Callable>
class ArgOptFunc : public ArgOptBase
   {
   public:
      ArgOptFunc(ArgParser& parser, Callable& fn, const char* shortname, const char* fullname, const char* desc)
	 : ArgOptBase(parser,shortname,fullname,desc), m_func(fn)
	 {
	 m_func = fn ; 
	 }
      ~ArgOptFunc() {}

   protected:
      virtual bool parseValue(const char* arg) const
	 {
	    return m_func(arg) ;
	 }
   protected:
      Callable&  m_func ;
   } ;

//----------------------------------------------------------------------------

template <typename T>
class ArgOpt : public ArgOptBase
   {
   public:
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var)
	 {}
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc, T def_value)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var), m_defvalue(def_value), m_have_defvalue(true)
	 {}
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc,
	     T min_value, T max_value)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var), m_minvalue(min_value),
	   m_maxvalue(max_value), m_have_defvalue(false), m_have_minmax(true)
	 {}
      ArgOpt(ArgParser& parser, T& var, const char* shortname, const char* fullname, const char* desc, T def_value,
	     T min_value, T max_value)
	 : ArgOptBase(parser,shortname,fullname,desc), m_value(var), m_defvalue(def_value), m_minvalue(min_value),
	   m_maxvalue(max_value), m_have_defvalue(true), m_have_minmax(true)
	 {}
      ~ArgOpt()
	 {
	    m_have_defvalue = m_have_minmax = false ;
	 }

   protected:
      static bool convert(const char* arg, T& value) ;
      virtual bool parseValue(const char* arg) const ;
      virtual bool optional() const { return m_have_defvalue ; }
   protected:
      T& m_value ;
      T m_defvalue { } ;
      T m_minvalue { } ;
      T m_maxvalue { } ;
      bool m_have_defvalue { false } ;
      bool m_have_minmax { false } ;
   } ;


// library provides instantiations for the common variable types
template<> bool ArgOpt<bool>::convert(const char*, bool&) ;
template<> bool ArgOpt<bool>::parseValue(const char*) const ;
extern template class ArgOpt<bool> ;
template<> bool ArgOpt<int>::convert(const char*, int&) ;
extern template class ArgOpt<int> ;
template<> bool ArgOpt<long>::convert(const char*, long&) ;
extern template class ArgOpt<long> ;
template<> bool ArgOpt<unsigned>::convert(const char*, unsigned&) ;
extern template class ArgOpt<unsigned> ;
template<> bool ArgOpt<size_t>::convert(const char*, size_t&) ;
extern template class ArgOpt<size_t> ;
template<> bool ArgOpt<float>::convert(const char*, float&) ;
extern template class ArgOpt<float> ;
template<> bool ArgOpt<double>::convert(const char*, double&) ;
extern template class ArgOpt<double> ;
template<> bool ArgOpt<const char*>::convert(const char*, const char*&) ;
extern template class ArgOpt<const char*> ;

//----------------------------------------------------------------------------

class ArgHelp : public ArgOptBase
   {
   public:
      ArgHelp(ArgParser&, const char* shortname, const char* fullname, const char* desc, bool longhelp = false) ;
      ArgHelp(ArgParser&, bool& flag, const char* shortname, const char* fullname, const char* desc, bool longhelp = false) ;
      ~ArgHelp() ;

      bool isLongHelp() const { return m_long ; }
      bool defer() const { return m_defer ; }

   protected:
      virtual bool parseValue(const char* arg) const ;
      virtual bool optional() const { return true ; }
   protected:
      bool* m_flag ;
      bool  m_long  ;
      bool  m_defer ;
   } ;

//----------------------------------------------------------------------------

class ArgParser
   {
   public:
      ArgParser() ;
      ~ArgParser() ;

      ArgParser& add(bool& var, const char* shortname, const char* fullname, const char* desc) ;
      ArgParser& add(bool& var, const char* shortname, const char* fullname, const char* desc, bool def_value) ;
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc) ;
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc, T defvalue) ;
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc,
		     T min_value, T max_value) ;
      template <typename T>
      ArgParser& add(T& var, const char* shortname, const char* fullname, const char* desc, T defvalue,
		     T min_value, T max_value) ;
      ArgParser& add(const char* shortname, const char* fullname, const char* desc, bool longhelp = false) ;

      void addOpt(ArgOptBase* opt, bool must_delete = false) ;
      bool parseArgs(int& argc, char**& argv) ;

      bool unknownOption(const char *name) const ;
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
