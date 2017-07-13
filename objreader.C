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

#include <istream>
#include "framepac/bignum.h"
#include "framepac/builder.h"
#include "framepac/objreader.h"
#include "framepac/list.h"
#include "framepac/rational.h"
#include "framepac/string.h"
#include "framepac/symbol.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

static ObjectReaderFunc skip_ws ;
static ObjectReaderFunc rdframe ;
static ObjectReaderFunc rdhash ;
static ObjectReaderFunc rdlist ;
static ObjectReaderFunc readneg ;
static ObjectReaderFunc readnum ;
static ObjectReaderFunc readsym ;
static ObjectReaderFunc readqsym ;
static ObjectReaderFunc readstr ;

class Object *symbolEOF { nullptr };

ObjectReader *ObjectReader::s_current { nullptr };

ObjectReaderFunc *ObjectReader::s_defaultdispatch[256]
   {
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 00-07
      skip_ws, skip_ws, skip_ws, nullptr, skip_ws, nullptr, nullptr, nullptr,  // 08-0F
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 10-17
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 18-1F
      skip_ws, nullptr, readstr, rdhash,  nullptr, nullptr, nullptr, nullptr,  // 20-27
      rdlist,  nullptr, nullptr, readneg, nullptr, readneg, nullptr, nullptr,  // 28-2F
      readnum, readnum, readnum, readnum, readnum, readnum, readnum, readnum,  // 30-37
      readnum, readnum, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 38-3F
      readsym, readsym, readsym, readsym, readsym, readsym, readsym, readsym,  // @-G
      readsym, readsym, readsym, readsym, readsym, readsym, readsym, readsym,  // H-O
      readsym, readsym, readsym, readsym, readsym, readsym, readsym, readsym,  // P-W
      readsym, readsym, readsym, rdframe, nullptr, nullptr, nullptr, readsym,  // X-5F
      nullptr, readsym, readsym, readsym, readsym, readsym, readsym, readsym,  // `-g
      readsym, readsym, readsym, readsym, readsym, readsym, readsym, readsym,  // h-o
      readsym, readsym, readsym, readsym, readsym, readsym, readsym, readsym,  // p-w
      readsym, readsym, readsym, nullptr, readqsym,nullptr, nullptr, nullptr,  // X-7F
      //FIXME
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 80-87
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 88-8F
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 90-97
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // 98-9F
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // A0-A7
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // A8-AF
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // B0-B7
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // B8-BF
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // C0-C7
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // C8-CF
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // D0-D7
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // D8-DF
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // E0-E7
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // E8-EF
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // F0-F7
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,  // F8-FF
   };

/************************************************************************/
/*	Helper functions						*/
/************************************************************************/

static bool is_symbol_char(int ch)
{
   //FIXME
   return isalnum(ch) || ch == '_' || ch == '-' ;
}

//----------------------------------------------------------------------------

/************************************************************************/
/************************************************************************/

static Object *skip_ws(const ObjectReader *reader, CharGetter &getter)
{
   (void)getter.peekNonWhite() ;	// skip all consecutive whitespace characters
   return getter.eof() ? symbolEOF : reader->read(getter) ;
}

//----------------------------------------------------------------------------

static Object *readsym(const ObjectReader *, CharGetter &getter,
		       char prefixchar)
{
   BufferBuilder<char> buf ;

   if (prefixchar)
      buf += prefixchar ;
   while (getter)
      {
      int nextch = getter.peek() ;
      if (is_symbol_char(nextch))
	 {
	 buf += *getter ;
	 }
      else
	 break ;
      }
   buf += '\0' ;
   return Symbol::create(*buf) ;
}

//----------------------------------------------------------------------------

static Object *readsym(const ObjectReader *reader, CharGetter &getter)
{
   return readsym(reader,getter,'\0') ;
}

//----------------------------------------------------------------------------

static Object *readnum(const ObjectReader *, CharGetter &getter,
		       bool negate)
{
   BufferBuilder<char> buf ;
   bool havedecimal { false };
   bool inexponent { false };
   bool atstart { true };
   bool isfloat { false };
   bool isrational { false };

   if (negate)
      {
      buf += '-' ;
      atstart = false ;
      }
   while (getter)
      {
      int nextch { getter.peek() };
      if (isdigit(nextch))
	 {
	 buf += *getter ;
	 atstart = false ;
	 }
      else if (nextch == '.' || nextch == 'e' || nextch == 'E')
	 {
	 if (inexponent || isrational)
	    break ;
	 isfloat = true ;
	 if (nextch == '.')
	    {
	    if (havedecimal)
	       break ;
	    havedecimal = true ;
	    atstart = false ;
	    }
	 else // 'e' or 'E'
	    {
	    inexponent = true ;
	    atstart = true ;
	    }
	 buf += *getter ;
	 }
      else if (nextch == '/')
	 {
	 if (!isfloat && !isrational)
	    {
	    isrational = true ;
	    buf += *getter ;
	    }
	 else
	    break ;
	 }
      else if (atstart && (nextch == '-' || nextch == '+'))
	 {
	 buf += *getter ;
	 atstart = false ;
	 }
   //FIXME
      }
   buf += '\0' ;
   Object *obj ;
   if (isfloat)
      {
      obj = Float::create(*buf) ;
      }
   else
      {
      obj = Number::create(*buf) ;
      }
   return obj ;
}

//----------------------------------------------------------------------------

static Object *readnum(const ObjectReader *reader, CharGetter &getter)
{
   return readnum(reader,getter,false) ;
}

//----------------------------------------------------------------------------
//  check whether a sequence of characters starting with a plus or minus sign
//  is a Number or a Symbol

static Object *readneg(const ObjectReader *reader, CharGetter &getter)
{
   int c { *getter };
   bool negated { c == '-' };
   if (isdigit(getter.peek()))
      return readnum(reader,getter,negated) ;
   else
      return readsym(reader,getter,c) ;
}

//----------------------------------------------------------------------------

static char *read_delimited_string(const ObjectReader *,
				   CharGetter &getter,
				   char quotechar, unsigned &len)
{
   BufferBuilder<char> buf ;
   int delim { *getter };	// consume the opening delimiter

   while (getter)
      {
      int nextch { *getter };
      if (nextch == quotechar)
	 {
	 if (delim == quotechar)
	    {
	    // if it's doubled, it was a quoted char
	    nextch = getter.peek() ;
	    if (nextch == delim)
	       {
	       buf += *getter ;
	       }
	    else
	       break ;		// it was the terminator, so we're done
	    }
	 else // interpret the quoted character
	    {
	    nextch = *getter ;
	    //FIXME
	    buf += nextch ;
	    }
	 }
      else if (nextch == delim)
	 {
	 break ;
	 }
      else
	 {
	 buf += *getter ;
	 }
      }
   len = buf.currentLength() ;
   buf += '\0' ;
   return buf.finalize() ;
}

//----------------------------------------------------------------------------

static Object *readqsym(const ObjectReader *reader, CharGetter &getter)
{
   unsigned len ;
   char *buf { read_delimited_string(reader,getter,'|',len) };
   Object *obj { Symbol::create(buf) };
   delete [] buf ;
   return obj ;
}

//----------------------------------------------------------------------------

static Object *readstr(const ObjectReader *reader, CharGetter &getter)
{
   unsigned len ;
   char *buf { read_delimited_string(reader,getter,'\\',len) };
   Object *obj { String::create(buf,len) };
   delete [] buf ;
   return obj ;
}

//----------------------------------------------------------------------------

static Object *skip_balanced_comment(const ObjectReader *reader,
				     CharGetter &getter)
{
   int nextch ;
   int nesting { 1 };
   do {
      do {
         // skip everything up to a vertical bar
         while ((nextch = *getter) != EOF && nextch != '|')
	    {
	    // discard this character, after checking for another #| opening delimiter
	    if (nextch == '#' && getter.peek() == '|')
	       {
	       (void)getter.get() ;
	       nesting++ ;
	       }
	    }
	 // keep going until the character following the vertical bar is a hash mark
         } while ((nextch = getter.peek()) != EOF && nextch != '#') ;
      // continue until all nested levels have been closed
      } while (--nesting > 0) ;
   // now that we've skipped the comment, return the next object to be read
   return reader->read(getter) ;
}

//----------------------------------------------------------------------------

static bool valid_digit(int digit, unsigned radix)
{
   if (digit < '0')
      return false ;
   digit = toupper(digit) ;
   if (digit >= 'A')
      digit -= ('A'-10) ;
   else if (digit <= '9')
      digit -= '0' ;
   else
      return false ;
   return (unsigned)digit < radix ;
}

//----------------------------------------------------------------------------

static Object *read_radix_number(const ObjectReader *,
				 CharGetter &getter,
				 unsigned radix)
{
   if (radix < 2 || radix > 36)
      return nullptr ;

   // accumulate digits until we hit a character that isn't a valid digit in the
   //   given radix
   BufferBuilder<char> buf ;
   int nextdigit ;
   while ((nextdigit = getter.peek()) != EOF && valid_digit(nextdigit,radix))
      {
      buf += *getter ;
      }
   buf += '\0' ;
   return Integer::create(*buf,radix) ;
}

//----------------------------------------------------------------------------
// read a Lisp-style form
//    #|  |#  balanced comment
//    #(  )   vector
//    #<  >
//    #'foo   -> (FUNCTION foo)
//    #\a     -> |a|
//    #:FOO   uninterned symbol
//    #nnn#   shared object reference
//    #nnn=   shared object definition
//    #*bbbb  bitvector
//    #A( )   array
//    #Bbbb   binary integer
//    #H( )   hash table
//    #Oooo   octal integer
//    #Q( )   queue
//    #Rnnn   integer in given radix
//    #S( )   struct
//    #Xxxx   hexadecimal integer
//    other   symbol starting with #

static Object *rdhash(const ObjectReader *reader, CharGetter &getter)
{
   (void)getter.get() ;		// consume the hash mark
   // accumulate any leading digits (used e.g. to specify radix for #R)
   char digits[200] ;
   unsigned numdigits { 0 };
   int nextch ;
   while ((nextch = getter.peek()) != EOF && isdigit(nextch))
      {
      if (numdigits < lengthof(digits)-1)
	 digits[numdigits++] = (char)nextch ;
      }
   digits[numdigits] = '\0' ;
   int type { *getter };	// figure out what type of hash-expression this is
   switch (type)
      {
      case EOF:
	 //FIXME
	 break ;
      case '|':			// balanced comment (non-nested)
	 return skip_balanced_comment(reader,getter) ;
      case '\'':		// FUNCTION shortcut
	 return List::create(Symbol::create("FUNCTION"),
			     reader->read(getter)) ;
      case '\\':		// Character (simulated as single-char Symbol)
	 {
	 char buf[2] ;
	 buf[0] = *getter ;
	 buf[1] = '\0' ;
	 return Symbol::create(buf) ;
	 }
      case 'b':
      case 'B':
	 return read_radix_number(reader,getter,2) ;
      case 'o':
      case 'O':
	 return read_radix_number(reader,getter,8) ;
      case'x':
      case 'X':
	 return read_radix_number(reader,getter,16) ;
      case 'r':
      case 'R':
	 return read_radix_number(reader,getter,atoi(digits)) ;
   //FIXME
      default:
	 break ;
      }
   return symbolEOF ;
}

//----------------------------------------------------------------------------

static Object *rdframe(const ObjectReader *reader, CharGetter &getter)
{
   (void)reader;
   (void)getter.get() ;		// consume the opening left bracket

   //FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

static Object *rdlist(const ObjectReader *reader, CharGetter &getter)
{
   (void)getter.get() ;		// consume the opening paren
   int nextch ;
   while ((nextch = getter.peek()) != EOF)
      {
      // skip whitespace
      if (reader->getDispatcher(nextch) == skip_ws)
	 continue ;
      if (nextch == ')')
	 {
	 // finalize the list and return it
	 //FIXME
	 }
      // not whitespace or the list terminator, so read an object and
      //   add it to the list
      Object *nextobj { reader->read(getter) };
      //FIXME
      (void)nextobj;
      }
   //FIXME
   return nullptr ;
}

/************************************************************************/
/************************************************************************/

ObjectReader::ObjectReader()
{

   return ;
}

//----------------------------------------------------------------------------

ObjectReader::~ObjectReader()
{

   return ;
}

//----------------------------------------------------------------------------

bool ObjectReader::initialize()
{
   if (!current())
      {
      s_current = new ObjectReader ;
      }
   return current() != nullptr ;
}

//----------------------------------------------------------------------------

Object *ObjectReader::read(CharGetter &getter) const
{
   int nextch { getter.peek() };
   if (nextch == -1)
      return symbolEOF ;
   if (m_dispatch[nextch])
      return m_dispatch[nextch](this,getter) ;
   // we got a char we don't know how to dispatch, so return an object representing
   //   just that char
   char s[2] ;
   s[0] = *getter ;
   s[1] = '\0' ;
   return String::create(s) ;
}

//----------------------------------------------------------------------------

Object *ObjectReader::readObject(istream &in) const
{
   CharGetterStream getter(in) ;
   return read(getter) ;
}

//----------------------------------------------------------------------------

Object *ObjectReader::readObject(FILE *infp) const
{
   CharGetterFILE getter(infp) ;
   return read(getter) ;
}

//----------------------------------------------------------------------------

Object *ObjectReader::readObject(char *&instr) const
{
   CharGetterCString getter(instr) ;
   return read(getter) ;
}

//----------------------------------------------------------------------------

Object *ObjectReader::readObject(std::string &instr) const
{
   CharGetterStdString getter(instr) ;
   return read(getter) ;
}

/************************************************************************/
/************************************************************************/

istream &operator >> (istream &in, Object &obj)
{
   (void)obj; //FIXME

   return in ;
}

//----------------------------------------------------------------------------


} ; // end namespace Fr

// end of file objreader.C //
