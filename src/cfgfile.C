/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-07-12					*/
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

namespace Fr
{

/************************************************************************/
/************************************************************************/


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
   (void)filename; (void)section; (void)reset;
   return false ; //FIXME
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
   (void)filename; (void)section; (void)params;
   return false ; //FIXME
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
   //TODO
   return ;
}

//----------------------------------------------------------------------------

void Configuration::startupComplete()
{
   //TODO
   return  ;
}

//----------------------------------------------------------------------------

void Configuration::beginningShutdown()
{
   //TODO
   return  ;
}

//----------------------------------------------------------------------------

List* Configuration::listParameters() const
{
   return nullptr ; //TODO
}

//----------------------------------------------------------------------------

List* Configuration::listFlags(const char* param_name) const
{
   (void)param_name; //FIXME
   return nullptr ;
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

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file cfgfile.C //
