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

#include "framepac/charget.h"
#include "framepac/configfile.h"
#include "framepac/list.h"

namespace Fr
{

/************************************************************************/
/*	Global Data							*/
/************************************************************************/

bool Configuration::s_instartup = true ;

/************************************************************************/
/************************************************************************/

Configuration::Configuration()
{

   return ;
}

//----------------------------------------------------------------------------

Configuration::Configuration(const char* basedir)
{
   (void)basedir; //FIXME
   return ;
}

//----------------------------------------------------------------------------

bool Configuration::load(CharGetter& stream, const char* section, bool reset)
{
   (void)stream; (void)section; (void)reset;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool Configuration::load(const char* filename, const char* section, bool reset)
{
   CInputFile file(filename) ;
   if (!file)
      return false ;
   CharGetterFILE getter(file) ;
   return getter ? load(getter,section,reset) : false ;
}

//----------------------------------------------------------------------------

bool Configuration::load(std::istream& instream, const char* section, bool reset)
{
   CharGetterStream stream(instream) ;
   return load(stream,section,reset) ;
}

//----------------------------------------------------------------------------

bool Configuration::loadRaw(CharGetter& stream, const char* section, List*& params)
{
   (void)stream; (void)section; (void)params;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

bool Configuration::loadRaw(const char* filename, const char* section, List*& params)
{
   CInputFile file(filename) ;
   if (!file)
      return false ;
   CharGetterFILE getter(file) ;
   return getter ? loadRaw(getter,section,params) : false ;
}

//----------------------------------------------------------------------------

bool Configuration::loadRaw(std::istream& instream, const char* section, List*& params)
{
   CharGetterStream stream(instream) ;
   return loadRaw(stream,section,params) ;
}

//----------------------------------------------------------------------------

void Configuration::freeValues()
{
   if (!m_currstate)
      return ;
   for (size_t i = 0 ; m_currstate[i].m_keyword ; ++i)
      {
      void* loc = nullptr; //FIXME
      if (!loc)
	 continue ;
      switch (m_currstate[i].m_vartype)
	 {
	 case integer:
	 case cardinal:
	 case real:
	 case bitflags:
	 case symbol:
	 case yesno:
	 case keyword:
	 case invalid:
	    // do nothing: these are stored directly in the configuration table
	    break  ;
	 case basedir:
	 case filename:
	 case cstring:
	    {
	    // allocated C-style string, so free the memory
	    char** str = reinterpret_cast<char**>(loc) ;
	    Free(*str) ;
	    *str = nullptr ;
	    }
	    break ;
	 case list:
	 case assoclist:
	 case symlist:
	 case filelist:
	    {
	    // FramepaC List object
	    Object** obj = reinterpret_cast<Object**>(loc) ;
	    if (*obj) (*obj)->free() ;
	    *obj = nullptr ;
	    }
	    break ;
	 default:
	    // TODO: free user type
	    break ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

void Configuration::startupComplete()
{
   s_instartup = false ;
   return  ;
}

//----------------------------------------------------------------------------

void Configuration::beginningShutdown()
{
   s_instartup = true ;
   return  ;
}

//----------------------------------------------------------------------------

List* Configuration::listParameters() const
{
   ListBuilder lb ;
   for (ConfigurationTable* tbl = m_currstate ; tbl && tbl->m_keyword ; ++tbl)
      {
      if (*tbl->m_keyword)
	 lb += tbl->m_keyword ;
      }
   return lb.move() ;
}

//----------------------------------------------------------------------------

List* Configuration::listFlags(const char* param_name) const
{
   ListBuilder lb ;
   const ConfigurationTable* tbl = findParameter(param_name,bitflags) ;
   if (tbl)
      {
      //TODO

      }
   return lb.move() ;
}

//----------------------------------------------------------------------------

List* Configuration::describeParameter(const char* param_name) const
{
   (void)param_name; //FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

bool Configuration::validValues(const char* param_name, const char*& min_value, const char*& max_value,
   const char*& default_value) const
{
   (void)param_name; (void)min_value; (void)max_value; (void)default_value; //FIXME
   return false ;
}

//----------------------------------------------------------------------------

char* Configuration::currentValue(const char* param_name)
{
   (void)param_name; //FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

char* Configuration::currentValue(const char* param_name, const char* bit_name)
{
   (void)param_name; (void)bit_name; //FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

bool Configuration::setParameter(const char* param_name, const char* new_value, std::ostream* err)
{
   (void)param_name; (void)new_value; (void)err;
   return false ;  //FIXME
}

//----------------------------------------------------------------------------

bool Configuration::setParameter(const char* new_value, const ConfigurationTable* param, bool initializing,
   std::ostream* err)
{
   (void)param; (void)new_value; (void)initializing; (void)err;
   return false ;  //FIXME
}

//----------------------------------------------------------------------------

bool Configuration::skipToSection(CharGetter& stream, const char* section_name, bool from_start)
{
   (void)stream; (void)section_name; (void)from_start;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

const ConfigurationTable* Configuration::findParameter(const char* param_name) const
{
   (void)param_name ;
   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

const ConfigurationTable* Configuration::findParameter(const char* param_name, ConfigVariableType type) const
{
   const ConfigurationTable* tbl = findParameter(param_name) ;
   return (tbl && tbl->m_vartype == type) ? tbl : nullptr ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file cfgfile.C //
