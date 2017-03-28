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

#ifndef _Fr_CHARGET_H_INCLUDED
#define _Fr_CHARGET_H_INCLUDED

#include <cstdlib>
#include <iostream>
using namespace std ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

// NOTE: the underlying stream given to the instance of descendants of
// this class in their constructor must remain valid for the lifetime
// of the instance
class CharGetter
   {
   private:
   public:
      virtual ~CharGetter() = default ;
      virtual bool eof() const = 0 ;
      virtual int peek() = 0 ;
      virtual int peekNonWhite() ;
      virtual int get() = 0 ;
      int getNonWhite() ;
      int operator * () { return get() ; }
      operator bool () const { return !eof() ; }
   } ;

//----------------------------------------------------------------------------

class CharGetterStream : public CharGetter
   {
   private:
      istream &m_stream ;
   public:
      CharGetterStream(istream &in) : m_stream(in) {}
      CharGetterStream(const CharGetterStream &orig) : m_stream(orig.m_stream) {}
      virtual ~CharGetterStream() {}
      virtual int peek() { return m_stream.peek() ; }
      virtual int get() { return m_stream.get() ; }
      virtual bool eof() const { return m_stream.eof() ; }
   } ;

//----------------------------------------------------------------------------

class CharGetterFILE : public CharGetter
   {
   private:
      FILE *m_stream ;
   public:
      CharGetterFILE(FILE *fp) : m_stream(fp) {}
      CharGetterFILE(const CharGetterFILE &orig) : m_stream(orig.m_stream) {}
      virtual ~CharGetterFILE() {}
      CharGetterFILE& operator= (const CharGetterFILE &orig) { m_stream = orig.m_stream ; return *this ; }
      virtual int peek() ;
      virtual int peekNonWhite() ;
      virtual int get() { return fgetc(m_stream) ; }
      virtual bool eof() const { return feof(m_stream) ; }
   } ;

//----------------------------------------------------------------------------

class CharGetterCString : public CharGetter
   {
   private:
      const char *m_stream ;
   public:
      CharGetterCString(const char *s) : m_stream(s?s:"") {}
      CharGetterCString(const CharGetterCString &orig) : m_stream(orig.m_stream) {}
      virtual ~CharGetterCString() {}
      CharGetterCString& operator= (const CharGetterCString &orig) { m_stream = orig.m_stream ; return *this ; }
      virtual int peek() { return *m_stream ? *m_stream : EOF ; }
      using CharGetter::peekNonWhite ; //virtual int peekNonWhite() ;
      virtual int get() { return *m_stream ? *m_stream++ : EOF ; }
      virtual bool eof() const { return *m_stream == '\0' ; }
   } ;

//----------------------------------------------------------------------------

class CharGetterStdString : public CharGetter
   {
   private:
      std::string::const_iterator m_currpos ;
      std::string::const_iterator m_endpos ;
   public:
      CharGetterStdString(const std::string &s) : m_currpos(s.begin()), m_endpos(s.end()) {}
      CharGetterStdString(const CharGetterStdString &orig) : m_currpos(orig.m_currpos), m_endpos(orig.m_endpos) {}
      virtual ~CharGetterStdString() {}
      CharGetterStdString& operator= (const CharGetterStdString &orig)
	 {
	 m_currpos = orig.m_currpos ;
	 m_endpos = orig.m_endpos ;
	 return *this ;
	 }
      virtual int peek() { return eof() ? EOF : *m_currpos ; }
      using CharGetter::peekNonWhite ; //virtual int peekNonWhite() ;
      virtual int get() { return eof() ? EOF : *m_currpos++ ; }
      virtual bool eof() const { return m_currpos != m_endpos ; }
   } ;

//----------------------------------------------------------------------------

} // end of namespace Fr

#endif /* !_Fr_CHARGET_H_INCLUDED */

// end of file charget.h //
