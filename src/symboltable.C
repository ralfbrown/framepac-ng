/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-19					*/
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

#include "framepac/atomic.h"
#include "framepac/cstring.h"
#include "framepac/symboltable.h"
#include "framepac/fasthash64.h"
#include "framepac/texttransforms.h"

using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/************************************************************************/

Allocator SymbolTable::s_allocator(FramepaC::Object_VMT<SymbolTable>::instance(),sizeof(SymbolTable)) ;
SymbolTable::Initializer SymbolTable::s_init ;
const char SymbolTable::s_typename[] = "SymbolTable" ;

static Atomic<SymbolTable*> symbol_tables[256] ;
Atomic<unsigned> current_symbol_table ;

Atomic<size_t> gensym_count ;

/************************************************************************/
/*	Methods for class SymbolTable					*/
/************************************************************************/

static void cleanup_symbol(SymHashSet*, const Symbol* sym, NullObject)
{
   auto symbol = const_cast<Symbol*>(sym) ;
   symbol->symtabID(0) ;
   symbol->free() ;
   return ;
}

//----------------------------------------------------------------------------

SymbolTable::SymbolTable(size_t initial_size) : super(initial_size)
{
   onDelete(cleanup_symbol) ;
   return ;
}

//----------------------------------------------------------------------------

SymbolTable::SymbolTable(const SymbolTable &orig) : super(orig.bucket_count())
{
   return ;
}

//----------------------------------------------------------------------------

SymbolTable::~SymbolTable()
{
   // de-register the symbol table from the global list of tables
   if (m_table_id && m_table_id <= lengthof(symbol_tables))
      symbol_tables[m_table_id-1].store(nullptr) ;
   m_table_id = 0 ;

   return ;
}

//----------------------------------------------------------------------------

SymbolTable* SymbolTable::create(size_t capacity)
{
   Ptr<SymbolTable> symtab { new SymbolTable(capacity) } ;
   // allocate a table ID by scanning for an unused slot in symbol_tables
   for (size_t i = 0 ; i < lengthof(symbol_tables) ; ++i)
      {
      if (symbol_tables[i].load_relax() != nullptr)
	 continue ;
      SymbolTable* expected = nullptr ;
      if (symbol_tables[i].compare_exchange_strong(expected,symtab))
	 {
	 // we've successfully allocated a slot, so remember the ID
	 symtab->m_table_id = i+1 ;
	 return symtab.move() ;
	 }
      }
   // if we get here, we were unable to allocate a table ID, so we have to bail out
   return nullptr ;
}

//----------------------------------------------------------------------------

SymbolTable* SymbolTable::current()
{
   return current_symbol_table ? symbol_tables[current_symbol_table-1].load() : nullptr ;
}

//----------------------------------------------------------------------------

uint16_t SymbolTable::currentID()
{
   return current_symbol_table ;
}

//----------------------------------------------------------------------------

SymbolTable* SymbolTable::table(uint16_t id)
{
   return (id && id <= lengthof(symbol_tables)) ? symbol_tables[id-1] : nullptr ;
}

//----------------------------------------------------------------------------

void SymbolTable::select() const
{
   current_symbol_table.store(m_table_id) ;
   return ;
}

//----------------------------------------------------------------------------

bool SymbolTable::select(const char* name)
{
   for (size_t i = 0 ; i < lengthof(symbol_tables) ; ++i)
      {
      SymbolTable* symtab = symbol_tables[i].load() ;
      const char* table_name = symtab->tableName() ;
      if (name == table_name || (name && table_name && strcmp(name,table_name) == 0))
	 {
	 symtab->select() ;
	 return true ;
	 }
      }
   return false ;
}

//----------------------------------------------------------------------------

Symbol* SymbolTable::gensym(const char* basename, const char* suffix)
{
   if (!basename) basename = "GS" ;
   if (!suffix) suffix = "" ;
   const Symbol* sym ;
   bool existed(false) ;
   do {
      // get a unique count
      size_t cnt = gensym_count++ ;
      // generate the symbol's name
      CharPtr name { aprintf("%s%ld%s",basename,cnt,suffix) } ;
      sym = addKey(name,&existed) ;
      } while (!sym || existed) ; // loop until the generated name was not already a symbol in the table
   return const_cast<Symbol*>(sym) ;
}

//----------------------------------------------------------------------------

ObjectPtr SymbolTable::clone_(const Object *)
{

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr SymbolTable::subseq_int(const Object *, size_t start, size_t stop)
{
   (void)start;(void)stop;//FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

ObjectPtr SymbolTable::subseq_iter(const Object *, ObjectIter start, ObjectIter stop)
{
   (void)start;(void)stop;//FIXME

   return ObjectPtr(nullptr) ; //FIXME
}

//----------------------------------------------------------------------------

size_t SymbolTable::cStringLength_(const Object *, size_t wrap_at, size_t indent, size_t wrapped_indent)
{
   size_t len = indent + 9 ;
   (void)wrap_at;(void)wrapped_indent;//FIXME
   //FIXME

   return len ;
}

//----------------------------------------------------------------------------

char* SymbolTable::toCstring_(const Object *, char *buffer, size_t buflen, size_t wrap_at, size_t indent,
   size_t wrapped_indent)
{
   if (buflen < indent + 9) return buffer ;
   (void)wrap_at;(void)wrapped_indent;//FIXME
   buffer += snprintf(buffer,buflen,"%*s",(int)indent,"#Y(") ;
   //FIXME

   *buffer++ = ')' ;
   return buffer ;
}

//----------------------------------------------------------------------------

size_t SymbolTable::jsonStringLength_(const Object *obj, bool wrap, size_t indent)
{
   (void)obj; (void)wrap; (void)indent; //FIXME
   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool SymbolTable::toJSONString_(const Object *obj, char *buffer, size_t buflen, bool wrap, size_t indent)
{
   (void)obj; (void)buflen; (void)wrap; (void)indent; //FIXME
   if (!buffer)
      return false ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

Object *SymbolTable::front_(Object *)
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

const Object *SymbolTable::front_const(const Object *)
{

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

size_t SymbolTable::hashValue_(const Object* obj)
{
   const SymbolTable* st = static_cast<const SymbolTable*>(obj) ;
   uint64_t hashstate = fasthash64_init(st->size()) ;
   //FIXME: add in hash values of every symbol in the table

   return fasthash64_finalize(hashstate) ;
}

//----------------------------------------------------------------------------

bool SymbolTable::equal_(const Object *obj, const Object *other)
{
   if (obj == other)
      return true ;

   return false ; //FIXME
}

//----------------------------------------------------------------------------

int SymbolTable::compare_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

int SymbolTable::lessThan_(const Object *obj, const Object *other)
{
   if (obj == other)
      return 0 ;

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

void SymbolTable::StaticInitialization()
{
   // initialize a default symbol table
   if (!current())
      {
      SymbolTable* symtab = SymbolTable::create(10000) ;
      symtab->select() ;
      }
   return;
}

//----------------------------------------------------------------------------

void SymbolTable::StaticCleanup()
{
   // flag that there is no current symbol table
   current_symbol_table = 0 ;
   // then delete all symbol tables
   for (size_t i = 0 ; i < lengthof(symbol_tables) ; ++i)
      {
      auto symtab = symbol_tables[i].load() ;
      if (symtab)
	 symtab->free() ;
      }
   return ;
}

//----------------------------------------------------------------------------

} ; // end namespace Fr

/************************************************************************/
/*	Instantiation of Object_VMT<SymbolTable>			*/
/************************************************************************/

namespace FramepaC
{
// request an explicit instantiation of the template
template class Object_VMT<Fr::SymbolTable> ;

} // end namespace FramepaCC


// end of file symboltable.C //
