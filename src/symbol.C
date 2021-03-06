/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-18					*/
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

#include <cstring>
#include "framepac/symboltable.h"
#include "framepac/nonobject.h"
#include "framepac/fasthash64.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

Allocator SymbolProperties::s_allocator(FramepaC::Object_VMT<NonObject>::instance(),sizeof(SymbolProperties),
   alignof(SymbolProperties)) ;
Allocator Symbol::s_allocator(FramepaC::Object_VMT<Symbol>::instance(),sizeof(Symbol)) ;
const char Symbol::s_typename[] = "Symbol" ;

/************************************************************************/
/************************************************************************/

Symbol::~Symbol()
{
   unintern() ;				// remove from symbol table before deleting
   return ;
}

//----------------------------------------------------------------------------

SymbolPtr Symbol::create(istream&)
{
   //FIXME
   return nullptr ;
}

//----------------------------------------------------------------------------

SymbolPtr Symbol::createInCurrentSymtab(const char* name)
{
   auto sym = Symbol::create(name) ;
   if (sym)
      sym->symtabID(SymbolTable::currentID()) ;
   return sym ;
}

//----------------------------------------------------------------------------

void Symbol::unintern()
{
   auto id = symtabID() ;
   symtabID(0) ;
   auto symtab = SymbolTable::table(id) ;
   if (symtab)
      symtab->remove(this) ;
   return ;
}

//----------------------------------------------------------------------------

size_t Symbol::hashValue(const Symbol* sym)
{
   if (!sym) return 0 ;
   size_t length = strlen(sym->name()) ;
   return FramepaC::fasthash64(sym->name(),length) ;
}

//----------------------------------------------------------------------------

size_t Symbol::hashValue(const char* name, size_t* len)
{
   size_t length = name ? strlen(name) : 0 ;
   if (len) *len = length ;
   return FramepaC::fasthash64(name,length) ;
}

//----------------------------------------------------------------------------

void Symbol::binding(Object* b)
{
   SymbolProperties* prop = properties() ;
   if (!prop)
      {
      // instantiate the properties struct for this symbol
      prop = SymbolProperties::create() ;
      m_properties.pointer(prop) ;
      }
   prop->binding(b) ;
   return  ;
}

//----------------------------------------------------------------------------

bool Symbol::frame(Frame* f)
{
   SymbolProperties* prop = properties() ;
   if (!prop)
      {
      // instantiate the properties struct for this symbol
      prop = SymbolProperties::create() ;
      m_properties.pointer(prop) ;
      }
   prop->frame(f) ;
   return true ;
}

//----------------------------------------------------------------------------

bool Symbol::setProperty(Symbol* key, Object* value)
{
   SymbolProperties* prop = properties() ;
   if (!prop)
      {
      // instantiate the properties struct for this symbol
      prop = SymbolProperties::create() ;
      m_properties.pointer(prop) ;
      }
   prop->setProperty(key,value) ;
   return true ;
}

//----------------------------------------------------------------------------

Symbol* Symbol::inverseRelation() const
{
   if (isRelation())
      {
      const SymbolProperties* prop = properties() ;
      return prop ? prop->inverseRelation() : nullptr ;
      }
   return nullptr ;
}
   
//----------------------------------------------------------------------------

bool Symbol::makeRelation(Symbol* inverse)
{
   if (inverse)
      {
      if (!properties())
	 {
	 //TODO
	 if (!properties())
	    return false ;
	 }
      properties()->inverseRelation(inverse) ;
      setFlag(RELATION_FLAG) ;
      }
   else
      clearFlag(RELATION_FLAG) ;
   return true ;
}

//----------------------------------------------------------------------------

void Symbol::setFlag(uint8_t flag)
{
   m_properties.extra(m_properties.extra() | flag) ;
   return ;
}

//----------------------------------------------------------------------------

void Symbol::symtabID(uint16_t id)
{
   m_properties.extra((m_properties.extra()&0x0F) | (id<<4)) ;
   return ;
}

//----------------------------------------------------------------------------

void Symbol::clearFlag(uint8_t flag)
{
   m_properties.extra(m_properties.extra() & ~flag) ;
   return ;
}

//----------------------------------------------------------------------------

bool Symbol::nameNeedsQuoting(const char* name)
{
   if (name)
      {
      char c = *name ;
      if (!isupper(c) && c != '_' && c != '#')
	 return true ;
      ++name ;
      while (*name)
	 {
	 c = *name ;
	 if (islower(c)) return true ;
	 if (!isalnum(c) && c != '-' && c != '_')
	    return true ;
	 ++name ;
	 }
      }
   return false ;
}

//----------------------------------------------------------------------------

ObjectPtr Symbol::subseq_int(const Object* obj, size_t start, size_t stop)
{
   auto sym = static_cast<const Symbol*>(obj) ;
   Symbol* new_sym = new Symbol(sym->c_str()+start,stop-start) ;
   //TODO? if original symbol is interned, do we want to add new one to same symbol table?
   return ObjectPtr(new_sym) ;
}

//----------------------------------------------------------------------------

ObjectPtr Symbol::subseq_iter(const Object*, ObjectIter start, ObjectIter stop)
{
   auto sym = static_cast<Symbol*>(start.baseObject()) ;
   Symbol* new_sym = new Symbol(sym->c_str()+start.currentIndex(),stop.currentIndex()-start.currentIndex()) ;
   //TODO? if original symbol is interned, do we want to add new one to same symbol table?
   return ObjectPtr(new_sym) ;
}

//----------------------------------------------------------------------------

size_t Symbol::cStringLength_(const Object* obj, size_t /*wrap_at*/, size_t indent, size_t /*wrapped_indent*/)
{
   size_t namelen = static_cast<const Symbol*>(obj)->c_len() ;
   const char* name = static_cast<const Symbol*>(obj)->c_str() ;
   size_t len = namelen + indent ;
   if (nameNeedsQuoting(name))
      {
      len += 2 ;			// account for leading and trailing vertical bars
      for (size_t i = 0 ; i < namelen ; ++i)
	 {
	 if (name[i] == '|')
	    len++ ;
	 }
      }
   return len ;
}

//----------------------------------------------------------------------------

char* Symbol::toCstring_(const Object* obj, char* buffer, size_t buflen, size_t /*wrap_at*/, size_t indent,
   size_t /*wrapped_indent*/)
{
   const char* name = static_cast<const Symbol*>(obj)->c_str() ;
   size_t len = static_cast<const Symbol*>(obj)->c_len() ;
   for (size_t i = 0 ; i < indent && buflen > 0 ; ++i)
      {
      --buflen ;
      *buffer++ = ' ' ;
      }
   if (nameNeedsQuoting(name))
      {
      if (buflen < 2) return buffer ;
      *buffer++ = '|' ;
      for ( ; *name && buflen > 1 ; ++name)
	 {
	 if (*name == '|')
	    {
	    if (--buflen < 2)
	       {
	       break ;
	       }
	    *buffer++ = *name ;
	    }
	 --buflen ;
	 *buffer++ = *name ;
	 }
      *buffer++ = '|' ;
      }
   else
      {
      size_t copycount = std::min(buflen,len) ;
      memcpy(buffer,name,copycount) ;
      buffer += copycount ;
      }
   *buffer = '\0' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t Symbol::hashValue_(const Object* obj)
{
   return FramepaC::fasthash64_mix((uint64_t)obj) ;
}

//----------------------------------------------------------------------------

bool Symbol::equal_(const Object *obj, const Object *other)
{
   // symbol equality comparisons are done strictly as pointer
   //   comparisons, because only a single instance of a string can be
   //   in a given symbol table
   return (obj == other) ;
}

//----------------------------------------------------------------------------

int Symbol::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int Symbol::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<Symbol>				*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::Symbol> ;

} // end namespace FramepaCC


// end of file symbol.C //
