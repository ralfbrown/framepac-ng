/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.02, last edit 2017-09-18					*/
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
#include "framepac/objreader.h"
#include "framepac/array.h"
#include "framepac/bignum.h"
#include "framepac/bitvector.h"
#include "framepac/list.h"
#include "framepac/map.h"
#include "framepac/number.h"
#include "framepac/rational.h"
#include "framepac/set.h"
#include "framepac/string.h"
#include "framepac/stringbuilder.h"
#include "framepac/symboltable.h"
#include "framepac/termvector.h"
#include "framepac/texttransforms.h"

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

class Object* symbolEOF { nullptr };

ObjectReader::Initializer ObjectReader::s_init ;
ObjectReader* ObjectReader::s_current { nullptr };

ObjectReaderFunc* ObjectReader::s_defaultdispatch[256]
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

static Object* skip_ws(const ObjectReader *reader, CharGetter &getter)
{
   (void)getter.peekNonWhite() ;	// skip all consecutive whitespace characters
   return getter.eof() ? symbolEOF : reader->read(getter) ;
}

//----------------------------------------------------------------------------

static Object* readsym(const ObjectReader *, CharGetter &getter, StringBuilder& sb)
{
   while (getter)
      {
      int nextch = getter.peek() ;
      if (is_symbol_char(nextch))
	 {
	 sb += toupper(*getter) ;
	 }
      else
	 break ;
      }
   sb += '\0' ;
   return SymbolTable::current()->add(*sb) ;
}

//----------------------------------------------------------------------------

static Object* readsym(const ObjectReader *reader, CharGetter &getter)
{
   StringBuilder sb ;
   return readsym(reader,getter,sb) ;
}

//----------------------------------------------------------------------------

static Number* readnum(const ObjectReader *, CharGetter &getter,
		        bool negate)
{
   StringBuilder sb ;
   bool havedecimal { false };
   bool inexponent { false };
   bool atstart { true };
   bool isfloat { false };
   bool isrational { false };

   if (negate)
      {
      atstart = false ;
      sb += '-' ;
      }
   while (getter)
      {
      int nextch { getter.peek() };
      if (isdigit(nextch))
	 {
	 atstart = false ;
	 sb += *getter ;
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
	 sb += *getter ;
	 }
      else if (nextch == '/')
	 {
	 if (isfloat || isrational)
	    break ;
	 isrational = true ;
	 sb += *getter ;
	 }
      else if (atstart && (nextch == '-' || nextch == '+'))
	 {
	 atstart = false ;
	 sb += *getter ;
	 }
      else
	 break ;
      }
   Number *obj ;
   sb += '\0' ;
   if (isfloat)
      {
      obj = Float::create(*sb) ;
      }
   else
      {
      obj = Number::create(*sb) ;
      }
   return obj ;
}

//----------------------------------------------------------------------------

static Object* readnum(const ObjectReader *reader, CharGetter &getter)
{
   return readnum(reader,getter,false) ;
}

//----------------------------------------------------------------------------
//  check whether a sequence of characters starting with a plus or minus sign
//  is a Number or a Symbol

static Object* readneg(const ObjectReader *reader, CharGetter &getter)
{
   int c { *getter };
   bool negated { c == '-' };
   if (isdigit(getter.peek()))
      return readnum(reader,getter,negated) ;
   else
      {
      StringBuilder sb ;
      sb += c ;
      return readsym(reader,getter,sb) ;
      }
}

//----------------------------------------------------------------------------

char* ObjectReader::read_delimited_string(CharGetter &getter, char quotechar, size_t &len)
{
   StringBuilder sb ;
   int delim { *getter };	// consume the opening delimiter

   while (getter)
      {
      int nextch = *getter ;
      if (nextch == quotechar)
	 {
	 if (delim == quotechar)
	    {
	    // if it's doubled, it was a quoted char
	    nextch = getter.peek() ;
	    if (nextch == delim)
	       {
	       sb += *getter ;
	       }
	    else
	       break ;		// it was the terminator, so we're done
	    }
	 else // interpret the quoted character
	    {
	    nextch = *getter ;
	    if (quotechar == '\\')
	       {
	       if (nextch == 'a') nextch = '\a' ;
	       else if (nextch == 'b') nextch = '\b' ;
	       else if (nextch == 'e') nextch = '\e' ;
	       else if (nextch == 'f') nextch = '\f' ;
	       else if (nextch == 'n') nextch = '\n' ;
	       else if (nextch == 'r') nextch = '\r' ;
	       else if (nextch == 't') nextch = '\t' ;
	       else if (nextch == 'v') nextch = '\v' ;
	       else if (nextch == 'u')
		  {
		  //TODO: interpret Unicode sequence
		  }
	       else if (nextch == 'x')
		  {
		  //TODO: interpret hex sequence
		  }
	       else if (nextch == '0')
		  {
		  //TODO: interpret octal sequence
		  nextch = '\0' ;
		  }
	       }
	    sb += nextch ;
	    }
	 }
      else if (nextch == delim)
	 {
	 break ;
	 }
      else
	 {
	 sb += nextch ;
	 }
      }
   len = sb.currentLength() ;
   sb += '\0' ;
   return sb.finalize() ;
}

//----------------------------------------------------------------------------

static Object* readqsym(const ObjectReader *reader, CharGetter &getter)
{
   size_t len ;
   char* buf { reader->read_delimited_string(getter,'|',len) };
   Object* obj { SymbolTable::current()->add(buf) };
   delete[] buf ;
   return obj ;
}

//----------------------------------------------------------------------------

static Object* read_uninterned_symbol(const ObjectReader* 
reader, CharGetter& getter)
{
   char* buf ;
   if (getter.peek() == '|')
      {
      size_t len ;
      buf = reader->read_delimited_string(getter,'|',len) ;
      Object* obj { Symbol::create(buf) } ;
      delete[] buf ;
      return obj ;
      }
   else
      {
      StringBuilder sb ;
      while (getter)
	 {
	 int nextch = getter.peek() ;
	 if (is_symbol_char(nextch))
	    {
	    sb += *getter ;
	    }
	 else
	    break ;
	 }
      sb += '\0' ;
      return Symbol::create(*sb) ;
      }
}

//----------------------------------------------------------------------------

static Object* readstr(const ObjectReader *reader, CharGetter& getter)
{
   size_t len ;
   char *buf { reader->read_delimited_string(getter,'\\',len) };
   Object *obj { String::create(buf,len) };
   delete [] buf ;
   return obj ;
}

//----------------------------------------------------------------------------

static Object* skip_balanced_comment(const ObjectReader* reader,
				     CharGetter& getter)
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
	       *getter ;		// consume the character
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

static Object* read_radix_number(const ObjectReader *,
				 CharGetter &getter,
				 unsigned radix)
{
   if (radix < 2 || radix > 36)
      return nullptr ;

   // accumulate digits until we hit a character that isn't a valid digit in the
   //   given radix
   StringBuilder sb ;
   int nextdigit ;
   while ((nextdigit = getter.peek()) != EOF && valid_digit(nextdigit,radix))
      {
      sb += *getter ;
      }
   sb += '\0' ;
   return Integer::create(*sb,radix) ;
}

//----------------------------------------------------------------------------

static Object* read_array(const ObjectReader *reader, CharGetter &getter)
{
   Array* arr = Array::create(1000) ;

   size_t idx { 0 } ;
   int nextch ;
   while ((nextch = getter.peek()) != EOF)
      {
      // skip whitespace
      if (reader->getDispatcher(nextch) == skip_ws)
	 {
	 *getter ;			// consume the character
	 continue ;
	 }
      else if (nextch == ')')
	 {
	 *getter ;		    	// consume the character
	 break ;
	 }
      // not whitespace or the array terminator, so read an object and
      //   add it to the array
      Object* nextobj { reader->read(getter) };
      if (idx >= arr->capacity())
	 arr->reserve(2*arr->capacity()) ;
      (*arr)[idx++] = nextobj ;
      }
   // we either hit the terminating paren or ran out of input, so return what we got so far
   arr->resize(idx) ;
   return arr ;
}

//----------------------------------------------------------------------------

static Object* read_bitvector(const ObjectReader*, CharGetter& getter)
{
   // accumulate digits until we hit something other than '0' or '1'
   StringBuilder sb ;
   int nextdigit ;
   while ((nextdigit = getter.peek()) != EOF && (nextdigit == '0' || nextdigit == '1'))
      {
      sb += *getter ;
      }
   sb += '\0' ;
   return BitVector::create(*sb) ;
}

//----------------------------------------------------------------------------

static Object* read_generic_object(const ObjectReader* /*reader*/, CharGetter& getter, size_t size_hint)
{
   char type_name[500] ;
   unsigned namelen { 0 } ;

   // collect the type name
   int nextch ;
   while ((nextch = *getter) != EOF && nextch != ':' && nextch != '>')
      {
      type_name[namelen++] = (char)nextch ;
      if (namelen+1 >= sizeof(type_name)) break ;
      }
   type_name[namelen] = '\0' ;
   if (nextch == '>')
      return nullptr ;	       // empty object descriptor!
   // check for standard, known type names
#if 1||FIXME // currently generates link errors due to unimplemented standard functions
   if (strcmp(type_name,"TermVector") == 0)
      {
      return TermVector::read(getter,size_hint) ;
      }
   else if (strcmp(type_name,"TermCountVector") == 0)
      {
      return TermCountVector::read(getter,size_hint) ;
      }
#else
   (void)size_hint; //suppress unused-arg warning
#endif /* FIXME */
   //TODO: check list of registered readers for any additional user-defined types

   // nothing matched, so this is an unknown type.  Skip to the terminating right bracket
   while ((nextch = *getter) != EOF && nextch != '>')
      continue ;
   return nullptr ;
}

//----------------------------------------------------------------------------

static Object* read_hashmap(const ObjectReader* reader, CharGetter& getter, const char* digits)
{
   *getter ;				// discard the opening parenthesis
   size_t capacity = 200 ;
   if (digits && *digits)
      {
      capacity = atol(digits) ;
      }
   Map* map = Map::create(capacity) ;

   int nextch ;
   while ((nextch = getter.peek()) != EOF)
      {
      // skip whitespace
      if (reader->getDispatcher(nextch) == skip_ws)
	 {
	 *getter ;			// consume the character
	 continue ;
	 }
      else if (nextch == ')')
	 {
	 // end of listing of hashtable entries
	 *getter ;			// consume the character
	 break ;
	 }
      // read key and value and add them to the map
      Object* key { reader->read(getter) } ;
      Object* value { nullptr } ;
      if (getter.peekNonWhite() != EOF && getter.peek() != ')')
	 value = reader->read(getter) ;
      if (key)
	 {
	 map->add(key,value) ;
	 }
      }
   // we either hit the terminating paren or ran out of input, so return what we got so far
   return map ;
}

//----------------------------------------------------------------------------

static Object* read_hashset(const ObjectReader* reader, CharGetter& getter, const char* digits)
{
   *getter ;				// discard the opening parenthesis
   size_t capacity = 200 ;
   if (digits && *digits)
      {
      capacity = atol(digits) ;
      }
   Set* set = Set::create(capacity) ;

   int nextch ;
   while ((nextch = getter.peek()) != EOF)
      {
      // skip whitespace
      if (reader->getDispatcher(nextch) == skip_ws)
	 {
	 *getter ;			// consume the character
	 continue ;
	 }
      else if (nextch == ')')
	 {
	 // end of listing of hashtable entries
	 *getter ;			// consume the character
	 break ;
	 }
      // read key and add it to the set
      Object* key { reader->read(getter) } ;
      if (key)
	 {
	 set->add(key) ;
	 }
      }
   // we either hit the terminating paren or ran out of input, so return what we got so far
   return set ;
}

//----------------------------------------------------------------------------
// read a Lisp-style form
//    #|  |#  balanced comment
//    #(  )   vector
//    #<  >   FramepaC object by typeName()
//    #'foo   -> (FUNCTION foo)
//    #\a     -> |a|
//    #:FOO   uninterned symbol
//    #nnn#   shared object reference
//    #nnn=   shared object definition
//    #*bbbb  bitvector
//    #A( )   array
//    #Bbbb   binary integer
//    #H( )   hash set (keys only)
//    #M( )   hash map (key/value pairs)
//    #N<>    nullptr  -- should only occur inside an Array
//    #Oooo   octal integer
//    #Q( )   queue
//    #Rnnn   integer in given radix
//    #S( )   struct
//    #Xxxx   hexadecimal integer
//    #Y( )   symboltable
//    other   symbol starting with #

static Object* rdhash(const ObjectReader* reader, CharGetter& getter)
{
   *getter ;			// consume the hash mark
   // accumulate any leading digits (used e.g. to specify radix for #R)
   char digits[200] ;
   unsigned numdigits { 0 };
   int nextch ;
   while ((nextch = getter.peek()) != EOF && isdigit(nextch))
      {
      if (numdigits < lengthof(digits)-1)
	 digits[numdigits++] = *getter ;
      }
   digits[numdigits] = '\0' ;
   int type { getter.peek() } ;
   if (type != EOF) *getter ;
   nextch = getter.peek() ;
   if (!isalpha(type) || nextch == '(' || nextch == '<')
      {
      switch (type)
	 {
	 case '|':			// balanced comment (non-nested)
	    return skip_balanced_comment(reader,getter) ;
	 case '\'':		// FUNCTION shortcut
	    return List::create(SymbolTable::current()->add("FUNCTION"),
	       reader->read(getter)) ;
	 case '\\':		// Character (simulated as single-char Symbol)
	 {
	 char buf[2] ;
	 buf[0] = *getter ;
	 buf[1] = '\0' ;
	 return SymbolTable::current()->add(buf) ;
	 }
	 case '(':
	    // read vector: simulate as an array
	    return read_array(reader,getter) ;
	 case '<':
	    // FramepaC object by typeName
	    return read_generic_object(reader,getter,strtoul(digits,nullptr,10)) ;
	 case ':':
	    return read_uninterned_symbol(reader,getter) ;
	 case '#':
	    // TODO: shared object reference
	    if (!is_symbol_char(nextch))
	       {
	       // retrieve the object stored in the hash table under the index given by 'digits'
	       }
	    break ;
	 case '=':
	    // TODO: shared object definition
	    // read an object, store a copy in a hash table under the
	    //   index given by 'digits', and then return that object
	    break ;
	 case '*':
	    return read_bitvector(reader,getter) ;
	 case 'A':
	    // read array if the next character is an open paren, else treat it as a symbol
	    if (getter.peek() == '(')
	       {
	       *getter ;
	       return read_array(reader,getter) ;
	       }
	    break ;
	 case 'b':
	 case 'B':
	    return read_radix_number(reader,getter,2) ;
	 case 'H':
	    if (getter.peek() == '(')
	       return read_hashset(reader,getter,digits) ;
	    break ;
	 case 'M':
	    if (getter.peek() == '(')
	       return read_hashmap(reader,getter,digits) ;
	    break ;
	 case 'N':
	    {
	    if (nextch == '<')
	       {
	       *getter ;
	       if (getter.peek() == '>')
		  {
		  *getter ;
		  return nullptr ;  // special case for Array elements
		  }
	       }
	    break ;
	    }
	 case 'o':
	 case 'O':
	    return read_radix_number(reader,getter,8) ;
	 case 'Q':
	    // TODO: read queue
	    if (getter.peek() == '(')
	       {
	       //FIXME
	       }
	    break ;
	 case 'r':
	 case 'R':
	    return read_radix_number(reader,getter,atoi(digits)) ;
	 case 'S':
	    // TODO: read struct, ???simulated as a hashtable?
	    if (getter.peek() == '(')
	       {
	       //FIXME
	       }
	    break ;
	 case 'x':
	 case 'X':
	    return read_radix_number(reader,getter,16) ;
	 case EOF:
	 default:
	    // this is a symbol starting with #, which we'll complete reading below
	    break ;
	 }
      }
   // return the accumulated characters as a symbol: hash mark + digits + typechar + additional chars
   StringBuilder sb ;
   sb += '#' ;
   if (*digits) sb += digits ;
   sb += (char)type ;
   return readsym(reader,getter,sb) ;
}

//----------------------------------------------------------------------------

static Object* rdframe(const ObjectReader* reader, CharGetter& getter)
{
   (void)reader;
   *getter ;			// consume the opening left bracket

   //FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

static Object* rdlist(const ObjectReader* reader, CharGetter& getter)
{
   *getter ;		// consume the opening paren
   ListBuilder lb ;
   int nextch ;
   while ((nextch = getter.peek()) != EOF)
      {
      // skip whitespace
      if (reader->getDispatcher(nextch) == skip_ws)
	 {
	 *getter ;			// consume the character
	 continue ;
	 }
      else if (nextch == ')')
	 {
	 *getter ;		    	// consume the character
	 break ;
	 }
      // not whitespace or the list terminator, so read an object and
      //   add it to the list
      Object* nextobj { reader->read(getter) };
      lb += nextobj ;
      }
   // we either hit the terminating paren or ran out of input, so return what we got so far
   return lb.move() ;
}

/************************************************************************/
/************************************************************************/

ObjectReader::ObjectReader()
{
   memcpy(m_dispatch,s_defaultdispatch,sizeof(m_dispatch)) ;
   return ;
}

//----------------------------------------------------------------------------

ObjectReader::~ObjectReader()
{

   return ;
}

//----------------------------------------------------------------------------

Object* ObjectReader::read(CharGetter &getter) const
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

Object* ObjectReader::readObject(istream& in) const
{
   CharGetterStream getter(in) ;
   return read(getter) ;
}

//----------------------------------------------------------------------------

Object* ObjectReader::readObject(FILE* infp) const
{
   CharGetterFILE getter(infp) ;
   return read(getter) ;
}

//----------------------------------------------------------------------------

Object* ObjectReader::readObject(CFile& infile) const
{
   CharGetterFILE getter(infile) ;
   return read(getter) ;
}

//----------------------------------------------------------------------------

Object* ObjectReader::readObject(char*& instr) const
{
   CharGetterCString getter(instr) ;
   Object* o = read(getter) ;
   // update caller's string pointer so that multiple objects can be read from one string
   instr = const_cast<char*>(getter.data()) ;
   return o ;
}

//----------------------------------------------------------------------------

Object* ObjectReader::readObject(const char*& instr) const
{
   CharGetterCString getter(instr) ;
   Object* o = read(getter) ;
   instr = getter.data() ;
   return o ;
}

//----------------------------------------------------------------------------

Object* ObjectReader::readObject(const std::string& instr) const
{
   CharGetterStdString getter(instr) ;
   return read(getter) ;
}

//----------------------------------------------------------------------------

Number* ObjectReader::readNumber(CharGetter& getter) const
{
   int c = getter.peekNonWhite() ;
   if (c == EOF) return nullptr ;
   bool negated { c == '-' };
   if (negated) *getter ;
   return readnum(nullptr,getter,negated) ;
}

//----------------------------------------------------------------------------

void ObjectReader::StaticInitialization()
{
   if (!s_current)
      s_current = new ObjectReader ;
   if (!symbolEOF)
      {
      SymbolTable::StaticInitialization() ;  // ensure that we have a default symbol table
      SymHashSet::threadInit() ;	     // and initialize per-thread data for main thread
      symbolEOF = SymbolTable::current()->add("*EOF*") ;
      }
   return ;
}

/************************************************************************/
/*	Additional methods for class Object				*/
/************************************************************************/

ObjectPtr Object::create(const char*& printed)
{
   return ObjectPtr(ObjectReader::current()->readObject(printed)) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::create(FILE* fp)
{
   return ObjectPtr(ObjectReader::current()->readObject(fp)) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::create(CFile& file)
{
   return ObjectPtr(ObjectReader::current()->readObject(file)) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::create(istream& in)
{
   return ObjectPtr(ObjectReader::current()->readObject(in)) ;
}

//----------------------------------------------------------------------------

ObjectPtr Object::create(const std::string& s)
{
   return ObjectPtr(ObjectReader::current()->readObject(s)) ;
}

//----------------------------------------------------------------------------

Object* Object::read(FILE* fp)
{
   return ObjectReader::current()->readObject(fp) ;
}

//----------------------------------------------------------------------------

Object* Object::read(istream& in)
{
   return ObjectReader::current()->readObject(in) ;
}

//----------------------------------------------------------------------------


} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

using namespace Fr ;

bool read_value(CharGetter& getter, uint32_t& value)
{
   int nextch ;
   uint32_t val { 0 } ;
   while ((nextch = getter.peek()) != EOF && isdigit(nextch))
      {
      *getter ;
      val = 10 * val + (nextch - '0') ;
      }
   value = val ;
   return true ;
}

//----------------------------------------------------------------------------

void read_floating_point(CharGetter& getter, StringBuilder& sb)
{
   int nextch = getter.peek() ;
   if (nextch == '+' || nextch == '-')
      {
      *getter ;
      sb += (char)nextch ;
      }
   while ((nextch = getter.peek()) != EOF && isdigit(nextch))
      {
      *getter ;
      sb += (char)nextch ;
      }
   if (nextch == '.')
      {
      // read the decimal part
      *getter ;
      sb += (char)nextch ;
      while ((nextch = getter.peek()) != EOF && isdigit(nextch))
	 {
	 *getter ;
	 sb += (char)nextch ;
	 }
      }
   if (nextch == 'e' || nextch == 'E')
      {
      // read exponent
      sb += (char)nextch ;
      nextch = getter.peek() ;
      if (nextch == '+' || nextch == '-')
	 {
	 *getter ;
	 sb += (char)nextch ;
	 }
      while ((nextch = getter.peek()) != EOF && isdigit(nextch))
	 {
	 *getter ;
	 sb += (char)nextch ;
	 }
      }
   return ;
}

//----------------------------------------------------------------------------

bool read_value(CharGetter& getter, float& value)
{
   StringBuilder sb ;
   read_floating_point(getter,sb) ;
   value = strtof(*sb,nullptr) ;
   return true ;
}

//----------------------------------------------------------------------------

bool read_value(CharGetter& getter, double& value)
{
   StringBuilder sb ;
   read_floating_point(getter,sb) ;
   value = strtod(*sb,nullptr) ;
   return true ;
}

//----------------------------------------------------------------------------

bool read_value(CharGetter& getter, Object*& value)
{
   (void)getter; (void)value ;
//FIXME
   return false ;
}


} // end namespace FramepaC

// end of file objreader.C //
