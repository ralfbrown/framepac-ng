/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-14					*/
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

#include "framepac/list.h"
#include "framepac/map.h"
#include "framepac/number.h"
#include "framepac/objreader.h"
#include "framepac/stringbuilder.h"

using namespace Fr ;

/************************************************************************/
/************************************************************************/

static Object* read_json_map(const ObjectReader* reader, CharGetter& getter)
{
   *getter ;		// discard the opening left brace
   int nextch ;
   Map* map = Map::create() ;
   while ((nextch = getter.peekNonWhite()) != EOF && nextch != '}')
      {
      // map consists of keyword/value pairs
      Object* keyword = reader->read(getter) ;
      // JSON requires a colon after the keyword, but we'll muddle on if it's missing
      if (getter.peekNonWhite() == ':')
	 *getter ;
      if (getter.peekNonWhite() == '}')
	 {
	 // incomplete structure!  Set the value to null and bail out
	 *getter ;
	 map->add(keyword,nullptr) ;
	 return map ;
	 }
      // get the value for the keyword/value pair
      Object* value = reader->read(getter) ;
      // JSON requires a comma after the value, but we'll muddle on if it's missing
      if (getter.peekNonWhite() == ',')
	 *getter ;
      // insert the pair into the hash table
      map->add(keyword,value) ;
      }
   return map ;
}

//----------------------------------------------------------------------------

static Object* read_json_array(const ObjectReader* reader, CharGetter& getter)
{
   *getter ;		// discard the opening left bracket
   ListBuilder array ;
   int nextch ;
   while ((nextch = getter.peekNonWhite()) != EOF && nextch != ']')
      {
      if (nextch == ',')
	 {
	 // null element in the list
	 array += List::emptyList() ;
	 continue ;
	 }
      // get the next object and add it to the tail of the list
      Object* obj = reader->read(getter) ;
      if (obj == nullptr)
	 break ;
      array += obj ;
      // check for a comma (it's required by JSON, but we'll allow it to be omitted)
      if (getter.peekNonWhite() == ',')
	 {
	 *getter ;
	 }
      }
   return array.move() ;
}

//----------------------------------------------------------------------------

static Object* read_json_number(const ObjectReader* reader, CharGetter& getter)
{
   return reader->readNumber(getter) ;
}

//----------------------------------------------------------------------------

static Object* read_json_string(const ObjectReader* reader, CharGetter& getter)
{
   size_t len ;
   char* buf { reader->read_delimited_string(getter,'\\',len) } ;
   String* s = String::create(buf,len) ;
   delete[] buf ;
   return s ;
}

/************************************************************************/
/************************************************************************/

JSONReader::JSONReader()
{
   registerDispatcher('{',read_json_map) ;
   registerDispatcher('[',read_json_array) ;
   registerDispatcher('"',read_json_string) ;
   registerDispatcher('\'',read_json_string) ;
   for (char c = '0' ; c <= '9' ; ++c)
      registerDispatcher(c,read_json_number) ;
   registerDispatcher('+',read_json_number) ;
   registerDispatcher('-',read_json_number) ;
   return ;
}

//----------------------------------------------------------------------------

JSONReader::~JSONReader()
{
   return ;
}

// end of file jsonreader.C //
