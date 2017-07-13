/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-12					*/
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

#include "framepac/atomic.h"
#include "framepac/symbol.h"
#include "framepac/fasthash64.h"

using namespace FramepaC ;

/************************************************************************/
/************************************************************************/

namespace Fr
{

Allocator SymbolTable::s_allocator(FramepaC::Object_VMT<SymbolTable>::instance(),sizeof(SymbolTable)) ;

static Atomic<SymbolTable*> symbol_tables[256] ;
Atomic<unsigned> current_symbol_table ;

/************************************************************************/
/************************************************************************/

SymbolTable* SymbolTable::current()
{
   if (current_symbol_table >= lengthof(symbol_tables)) return nullptr ;
   return symbol_tables[current_symbol_table].load() ;
}

//----------------------------------------------------------------------------

void SymbolTable::select() const
{
   current_symbol_table.store(m_table_id) ;
   return ;
}

//----------------------------------------------------------------------------

SymbolTable* SymbolTable::create(size_t capacity)
{
   SymbolTable* symtab = new SymbolTable(capacity) ;
   // allocate a table ID by scanning for an unused slot in symbol_tables
   for (size_t i = 0 ; i < lengthof(symbol_tables) ; ++i)
      {
      if (symbol_tables[i].load_relax() != nullptr)
	 continue ;
      SymbolTable* expected = nullptr ;
      if (symbol_tables[i].compare_exchange_strong(expected,symtab))
	 {
	 // we've successfully allocated a slot, so remember the ID
	 symtab->m_table_id = i ;
	 return symtab ;
	 }
      }
   // if we get here, we were unable to allocate a table ID, so we have to bail out
   delete symtab ;
   return nullptr ;
}

//----------------------------------------------------------------------------

SymbolTable::SymbolTable(size_t initial_size) : m_symbols(nullptr)
{
   (void)initial_size;//FIXME

   return ;
}

//----------------------------------------------------------------------------

SymbolTable::SymbolTable(const SymbolTable &orig) : Object(), m_symbols(nullptr)
{
   (void)orig;//FIXME

   return ;
}

//----------------------------------------------------------------------------

SymbolTable::~SymbolTable()
{
   // de-register the symbol table from the global list of tables
   if (m_table_id < lengthof(symbol_tables))
      symbol_tables[m_table_id].store(nullptr) ;
   m_table_id = ~0 ;

   return ;
}

//----------------------------------------------------------------------------

Symbol* SymbolTable::add(const char* name)
{
   if (!name) return nullptr ;

   return nullptr ; //FIXME
}

//----------------------------------------------------------------------------

Symbol* SymbolTable::find(const char* name) const
{
   if (!name) return nullptr ;

   return nullptr ; //FIXME
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

size_t SymbolTable::cStringLength_(const Object *, size_t wrap_at, size_t indent)
{
   (void)wrap_at;(void)indent;//FIXME

   return 0 ; //FIXME
}

//----------------------------------------------------------------------------

bool SymbolTable::toCstring_(const Object *, char *buffer, size_t buflen, size_t wrap_at, size_t indent)
{
   (void)buffer;(void)buflen;(void)wrap_at;(void)indent;//FIXME
   //FIXME
   return true ;
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
