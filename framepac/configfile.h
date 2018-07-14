/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-13					*/
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

#ifndef _Fr_CONFIGFILE_H_INCLUDED
#define _Fr_CONFIGFILE_H_INCLUDED

#include <cstdint>
#include <iostream>

/************************************************************************/
/************************************************************************/

namespace Fr
{

// when may a value be updated from a configuration file?  
enum class Settable
   {
   ReadOnly,
   Startup,
   Runtime,
   AnyTime
   } ;

//----------------------------------------------------------------------------

class CommandBit
   {
   public:
      const char*  m_name ;
      std::size_t  m_bitmask ;
      bool         m_default ;
      const char*  m_description ;
   } ;
   
//----------------------------------------------------------------------------

enum ConfigVariableType
   {
   integer, cardinal, real, basedir, filename, filelist, cstring, bitflags, list,
   assoclist, symbol, symlist, yesno, keyword, invalid, user
   } ;

//----------------------------------------------------------------------------

class ConfigurationTable ;
class CharGetter ;
class List ;

class Configuration
   {
   public:
      Configuration() ;
      Configuration(const char* basedir) ;
      virtual ~Configuration() ;

      bool load(CharGetter& stream, const char* section = nullptr, bool reset = true) ;
      bool load(const char* filename, const char* section = nullptr, bool reset = true) ;
      bool load(std::istream& instream, const char* section = nullptr, bool reset = true) ;

      bool loadRaw(CharGetter& stream, const char* section, List*& params) ;
      bool loadRaw(const char* filename, const char* section, List*& params) ;
      bool loadRaw(std::istream& instream, const char* section, List*& params) ;

      void warn(const char* msg) const ;
      template <typename T>
      void warn(const char* msg, const char* where, T value) const ;

      void freeValues() ;

      static void startupComplete() ;
      static void beginningShutdown() ;

      List* listParameters() const ;
      List* listFlags(const char* param_name) const ;
      List* describeParameter(const char* param_name) const ;

      bool validValues(const char* param_name, const char*& min_value, const char*& max_value,
	 const char*& def_value) const ;
      char* currentValue(const char* param_name) ;
      char* currentValue(const char* param_name, const char* bit_name) ;

      bool setParameter(const char* param_name, const char* new_value, std::ostream* err = nullptr) ;
      bool setParameter(const char* new_value, const ConfigurationTable* param, bool initializing,
	 std::ostream* err = nullptr) ;

      // access to internal state
      bool good() const { return m_valid ; }
      int lineNumber() const { return m_currline  ; }
      ConfigurationTable* currentState() const { return m_currstate ; }
      ostream& dumpFlags(const char* heading, unsigned long flags, CommandBit* bits, ostream& out) const ;
      ostream& dumpValues(const char* heading, ostream& out) const ;
      virtual ostream& dump(ostream&  out) const ;

      // methods which must be provided by each derived class
      virtual void init() ;
      virtual void resetState() = 0 ;

      // methods allowing derived classes to modify behavior

      //   number of bytes of configuration values stored as member variables in class instance
      virtual size_t localVarSize() const ;

      //   action to take whenever a variable is about to be retrieved
      virtual bool onRead(ConfigVariableType, void* where) ;

      //   action to take whenever a variable is modified
      virtual bool onChange(ConfigVariableType, void* where) ;
      
   protected: // methods
      char* currentValue(const ConfigurationTable* param) ;
      bool skipToSection(CharGetter& stream, const char* section_name, bool from_start = true) ;
      ConfigurationTable* findParameter(const char* param_name) ;
      const ConfigurationTable* findParameter(const char* param_name) const ;
      const ConfigurationTable* findParameter(const char* param_name, ConfigVariableType type) const ;

      bool parseParameter(const char* line, const ConfigurationTable* tbl, bool initializing) ;

   protected: // data
      static bool s_instartup ;
      ConfigurationTable* m_currstate ;
      char* m_infile_name ;
      char* m_basedir ;
      int   m_currline ;
      bool  m_valid ;
   } ;

//----------------------------------------------------------------------------

class ConfigurationTable
   {
   public:
      const char*         m_keyword ;
      ConfigVariableType  m_vartype ;
      void*               m_location ;
      Settable            m_settability ;
      ConfigurationTable* m_next_state ;
      void*               m_extra_args ;
      const char*         m_default_value ;
      const char*         m_min_value ;
      const char*         m_max_value ;
      const char*         m_description ;
   } ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !_Fr_CONFIGFILE_H_INCLUDED */

// end of file configfile.h //
