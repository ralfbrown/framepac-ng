/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-16					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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
#include "framepac/file.h"

using namespace std ;

namespace Fr
{

// forward declaration
class Object ;

/************************************************************************/
/************************************************************************/

// NOTE: the underlying stream given to the instance of descendants of
// this class in their constructor must remain valid for the lifetime
// of the instance
class CharGetter
   {
   public:
      virtual ~CharGetter() = default ;
      virtual bool eof() const = 0 ;
      virtual bool rewind() = 0 ;
      virtual int peek() = 0 ;
      virtual int peekNonWhite() ;
      virtual int get() = 0 ;
      char* getLine() ;
      int getNonWhite() ;
      int operator * () { return get() ; }
      operator bool () const { return !eof() ; }
   } ;

//----------------------------------------------------------------------------

class CharGetterStream : public CharGetter
   {
   public:
      typedef CharGetter super ;
   public:
      CharGetterStream(istream &in) : m_stream(in) {}
      CharGetterStream(const CharGetterStream &orig) : m_stream(orig.m_stream) {}
      virtual ~CharGetterStream() {}
      virtual bool rewind() ;
      virtual int peek() { return m_stream.peek() ; }
      virtual int get() { return m_stream.get() ; }
      virtual bool eof() const { return m_stream.eof() ; }

   private:
      istream &m_stream ;
   } ;

//----------------------------------------------------------------------------

class CharGetterFILE : public CharGetter
   {
   public:
      typedef CharGetter super ;
   public:
      CharGetterFILE(FILE *fp) : m_stream(fp) {}
      CharGetterFILE(Fr::CFile& file) : m_stream(file.fp()) {}
      CharGetterFILE(const CharGetterFILE &orig) : m_stream(orig.m_stream) {}
      virtual ~CharGetterFILE() {}
      CharGetterFILE& operator= (const CharGetterFILE &orig) { m_stream = orig.m_stream ; return *this ; }
      virtual bool rewind() ;
      virtual int peek() ;
      virtual int peekNonWhite() ;
      virtual int get() { return fgetc(m_stream) ; }
      virtual bool eof() const { return feof(m_stream) ; }

   protected:
      FILE *m_stream ;
   } ;

//----------------------------------------------------------------------------

class CharGetterCString : public CharGetter
   {
   public:
      typedef CharGetter super ;
   public:
      CharGetterCString(const char *s) : m_stream(s?s:""), m_stream_start(m_stream) {}
      CharGetterCString(const CharGetterCString &orig) : m_stream(orig.m_stream), m_stream_start(orig.m_stream_start) {}
      virtual ~CharGetterCString() {}
      CharGetterCString& operator= (const CharGetterCString &orig) { m_stream = orig.m_stream ; return *this ; }
      virtual bool rewind() { m_stream = m_stream_start ; return true ; }
      virtual int peek() { return *m_stream ? *m_stream : EOF ; }
      using CharGetter::peekNonWhite ; //virtual int peekNonWhite() ;
      virtual int get() { return *m_stream ? *m_stream++ : EOF ; }
      virtual bool eof() const { return *m_stream == '\0' ; }
      // get the current data pointer, to allow resynchronization back to caller
      const char* data() const { return m_stream ; }

   private:
      const char *m_stream ;
      const char *m_stream_start ;
} ;

//----------------------------------------------------------------------------

class CharGetterStdString : public CharGetter
   {
   public:
      typedef CharGetter super ;
   public:
      CharGetterStdString(const std::string &s) : m_currpos(s.begin()), m_endpos(s.end()), m_startpos(s.begin()) {}
      CharGetterStdString(const CharGetterStdString &orig)
	 : m_currpos(orig.m_currpos), m_endpos(orig.m_endpos), m_startpos(orig.m_startpos) {}
      virtual ~CharGetterStdString() {}
      CharGetterStdString& operator= (const CharGetterStdString &orig)
	 {
	 m_currpos = orig.m_currpos ;
	 m_endpos = orig.m_endpos ;
	 return *this ;
	 }
      virtual bool rewind() { m_currpos = m_startpos ; return true ; }
      virtual int peek() { return eof() ? EOF : *m_currpos ; }
      using CharGetter::peekNonWhite ; //virtual int peekNonWhite() ;
      virtual int get() { return eof() ? EOF : *m_currpos++ ; }
      virtual bool eof() const { return m_currpos != m_endpos ; }

   private:
      std::string::const_iterator m_currpos ;
      std::string::const_iterator m_endpos ;
      std::string::const_iterator m_startpos ;
   } ;

//----------------------------------------------------------------------------

} // end of namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

// helper functions to read values of various types from within a generic-object form

bool read_value(Fr::CharGetter&, uint32_t&) ;
bool read_value(Fr::CharGetter&, float&) ;
bool read_value(Fr::CharGetter&, double&) ;
bool read_value(Fr::CharGetter&, Fr::Object*&) ;

} // end namespace Fr

#endif /* !_Fr_CHARGET_H_INCLUDED */

// end of file charget.h //
