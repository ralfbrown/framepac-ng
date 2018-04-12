/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-29					*/
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
      if (!obj_key || strlen(obj_key) < keylen) continue ;
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

} // end namespace Fr

// end of file prefixmatcher.C //
