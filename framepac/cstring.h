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

#include <iostream>

/************************************************************************/
/************************************************************************/

namespace Fr {

class CString 
   {
   public:
      CString() noexcept : _s(nullptr) {}
      CString(int) noexcept : _s(nullptr) {} ;
      CString(const char *string) noexcept : _s(string) {}
      CString(const CString& other) = default ;
      ~CString() = default ;
      CString& operator= (const CString& other) = default ;

      void clear() { _s = nullptr ; }

      // methods to support use in HashTable
      unsigned long hashValue() const ;
      bool equal(const CString* s) const ;
      bool equal(const CString& s) const ;
      int compare(const CString* s) const ;
      int compare(const CString& s) const ;
      CString clone() const { return CString(_s) ; }
      char* toCstring(char* buf, size_t buflen) const
	 {
	    if (!buf || !_s || buflen == 0) return buf ;
	    if (buflen > cStringLength()+1) buflen = cStringLength()+1 ;
	    memcpy(buf,_s,buflen-1) ;
	    buf[buflen] = '\0' ;
	    return buf+buflen ;
	 }

      // accessors
      const char *str() const { return _s ; }
	    
      // interoperation with char*
      CString& operator= (const char *string) { _s = string ; return *this ; }
      const char *operator* () const { return _s ; }
      operator const char* () const { return _s ; }
      operator bool () const { return _s != nullptr ; }

      // functions needed by HashTable template
      friend std::ostream& operator << (std::ostream&, CString& ) ;
      operator class Object* () const { return nullptr ; }
      CString(unsigned long v) noexcept : _s((const char*)v) {}
      const CString* operator-> () const { return this ; }
      CString* operator-> () { return this ; }
      size_t cStringLength() const { return _s ? strlen(_s) : 0 ; }
      unsigned displayLength() const { return 0 ; }
      char *displayValue(char *buf) { return buf ; }

      bool operator== (const CString& other) const { return _s == other._s ; }
      bool operator!= (const CString& other) const { return _s != other._s ; }
   private:
      const char *_s ;
   } ;

//----------------------------------------------------------------------------

inline std::ostream& operator<< (std::ostream& out, Fr::CString& s)
{
   if (s.str())
      out << s.str() ;
   return out ;
}

// end of namespace Fr
} ;

#endif /* !_Fr_CSTRING_H_INCLUDED */

// end of file cstring.h //

