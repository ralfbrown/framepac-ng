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

#ifndef _Fr_UTILITY_H_INCLUDED
#define _Fr_UTILITY_H_INCLUDED

namespace Fr
{

class PrefixMatcher
   {
   public: // types
      typedef const char* KeyFunc(const void*) ;
      typedef const void* NextFunc(const void*) ;
      enum Status { NoMatch, UniqueMatch, ExactMatch, Ambiguous, FuncError, KeyError } ;

   public: // methods
      PrefixMatcher(KeyFunc* keyfn, NextFunc* nextfn, bool casefold = true)
	 : m_keyfunc(keyfn), m_nextfunc(nextfn), m_casefold(casefold), m_status(NoMatch)
	 {}
      ~PrefixMatcher() {}

      const void* match(const char* key, const void* first_obj) const ; // returns found match or nullptr
      Status status() const { return m_status ; }
      //TODO: enumerateMatches(const char* key, const void* first_obj) const ;

   protected:   
      KeyFunc*  m_keyfunc ;
      NextFunc* m_nextfunc ;
      bool      m_casefold ;
      mutable Status m_status ;
   } ;

} // end namespace Fr

#endif /* !_Fr_UTILITY_H_INCLUDED */

// end of file utility.h //
