/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
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

#ifndef _Fr_CSTRING_H_INCLUDED
#define _Fr_CSTRING_H_INCLUDED

/************************************************************************/
/************************************************************************/

namespace Fr {

class CString 
   {
   public:
      CString() noexcept : _s(nullptr) {}
      CString(const char *string) noexcept : _s(string) {}
      CString(const CString& other) noexcept : _s(other._s) {}
      ~CString() {}
      CString& operator= (const CString& other) { _s = other._s ; return *this ; }

      void clear() { _s = nullptr ; }

      // methods to support use in HashTable
      unsigned long hashValue() const ;
      bool equal(const CString* s) const ;
      bool equal(const CString& s) const ;
      int compare(const CString* s) const ;
      int compare(const CString& s) const ;

      // accessors
      const char *str() const { return _s ; }
	    
      // interoperation with char*
      CString& operator= (const char *string) { _s = string ; return *this ; }
      const char *operator* () const { return _s ; }
      operator const char* () const { return _s ; }

      // functions needed by HashTable template
      CString(unsigned long v) noexcept : _s((const char*)v) {}
      const CString* operator-> () const { return this ; }
      CString* operator-> () { return this ; }
      unsigned displayLength() const { return 0 ; }
      char *displayValue(char *buf) { return buf ; }

   private:
      const char *_s ;
   } ;

//----------------------------------------------------------------------------

// end of namespace Fr
} ;

#endif /* !_Fr_CSTRING_H_INCLUDED */

// end of file cstring.h //

