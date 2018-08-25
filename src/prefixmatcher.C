/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-25					*/
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

#include <cstdint>
#include <cstring>
#include "framepac/utility.h"
using namespace std ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

static const char* stringarray_keyfunc(const void* obj)
{
   auto stringarr = (const char**)obj ;
   return stringarr[0] ;
}

//----------------------------------------------------------------------------

static const void* stringarray_nextfunc(const void* obj)
{
   auto stringarr = (const char**)obj ;
   return stringarr+1 ;
}

//----------------------------------------------------------------------------

PrefixMatcher::PrefixMatcher(const char**keys, bool casefold)
   : m_keys(keys), m_keyfunc(stringarray_keyfunc), m_nextfunc(stringarray_nextfunc), m_casefold(casefold)
{
   return ;
}

//----------------------------------------------------------------------------

const char* PrefixMatcher::match(const char* key) const
{
   if (!m_keys || !key)
      {
      m_status = KeyError ;
      return nullptr ;
      }
   if (!m_keyfunc)
      {
      m_status = FuncError ;
      return nullptr ;
      }
   const void* m = match(key,m_keys) ;
   return m ? m_keyfunc(m) : nullptr ;
}

//----------------------------------------------------------------------------

const void* PrefixMatcher::match(const char* key, const void* obj) const
{
   m_status = NoMatch ;
   if (!obj) return nullptr ;
   if (!key || !*key)
      {
      m_status = KeyError ;
      return nullptr ;
      }
   if (!m_keyfunc || !m_nextfunc)
      {
      m_status = FuncError ;
      return nullptr ;
      }
   size_t keylen = strlen(key) ;
   const void* best_match = nullptr ;
   for ( ; obj ; obj = m_nextfunc(obj))
      {
      const char* obj_key = m_keyfunc(obj) ;
      if (!obj_key)
	 break  ;
      if (strlen(obj_key) < keylen) continue ;
      if (m_casefold)
	 {
	 // check for exact match
	 if (strcasecmp(key,obj_key) == 0)
	    {
	    m_status = ExactMatch ;
	    return obj ;
	    }
	 if (strncasecmp(key,obj_key,keylen) == 0)
	    {
	    if (m_status == NoMatch)
	       {
	       m_status = UniqueMatch ;
	       best_match = obj ;
	       }
	    else
	       {
	       m_status = Ambiguous ;
	       best_match = nullptr ;
	       break ;
	       }
	    }
	 }
      else // no case-folding
	 {
	 // check for exact match
	 if (strcmp(key,obj_key) == 0)
	    {
	    m_status = ExactMatch ;
	    return obj ;
	    }
	 if (strncmp(key,obj_key,keylen) == 0)
	    {
	    if (m_status == NoMatch)
	       {
	       m_status = UniqueMatch ;
	       best_match = obj ;
	       }
	    else
	       {
	       m_status = Ambiguous ;
	       best_match = nullptr ;
	       break ;
	       }
	    }
	 }
      }
   return best_match ;
}

//----------------------------------------------------------------------------

ListPtr PrefixMatcher::enumerateKeys(const void* obj) const
{
   ListBuilder lb ;;
   if (!m_keyfunc || !m_nextfunc)
      {
      m_status = FuncError ;
      return lb.move() ;
      }
   m_status = Successful ;
   for ( ; obj ; obj = m_nextfunc(obj))
      {
      lb += m_keyfunc(obj) ;
      }
   return lb.move() ;
}

//----------------------------------------------------------------------------

ListPtr PrefixMatcher::enumerateKeys() const
{
   return enumerateKeys(m_keys) ;
}

//----------------------------------------------------------------------------

ListPtr PrefixMatcher::enumerateMatches(const char* prefix, const void* obj) const
{
   ListBuilder lb ;;
   if (!m_keyfunc || !m_nextfunc)
      {
      m_status = FuncError ;
      return lb.move() ;
      }
   m_status = Successful ;
   size_t len = strlen(prefix) ;
   for ( ; obj ; obj = m_nextfunc(obj))
      {
      const char* key = m_keyfunc(obj) ;
      if (m_casefold)
	 {
	 if (strncasecmp(prefix,key,len) == 0)
	    lb += key ;
	 }
      else if (strncmp(prefix,key,len) == 0)
	 lb += key ;
      }
   return lb.move() ;
}

//----------------------------------------------------------------------------

ListPtr PrefixMatcher::enumerateMatches(const char* prefix) const
{
   return enumerateMatches(prefix,m_keys) ;
}

} // end namespace Fr

// end of file prefixmatcher.C //
