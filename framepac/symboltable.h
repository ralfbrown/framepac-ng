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

#ifndef _Fr_SYMBOLTABLE_H_INCLUDED
#define _Fr_SYMBOLTABLE_H_INCLUDED

#include "framepac/hashtable.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

class SymbolTable : public Object
   {
   public:
      static SymbolTable* create(size_t capacity = 0) ;
      static SymbolTable* current() ;

      void select() const ; // make this symbol table the current one
      static bool select(const char* name) ; // select the symbol table with the given name

      Symbol* gensym(const char* basename = nullptr, const char* suffix = nullptr) ;

      Symbol* add(const char* name) { return name ? const_cast<Symbol*>(m_symbols.addKey(name)) : nullptr ; }
      Symbol* add(const String* name) { return name ? add(name->c_str()) : nullptr ; }
      Symbol* add(const Object* obj) { return obj ? add(obj->stringValue()) : nullptr ; }

      Symbol* find(const char* name) const { return const_cast<Symbol*>(m_symbols.lookupKey(name)) ; }
      Symbol* find(const String* name) const { return name ? find(name->c_str()) : nullptr ; }

      bool exists(const char* name) const { return m_symbols.contains(name) ; }
      bool exists(const String* name) const { return name ? m_symbols.contains(name->c_str()) : false ; }

      void tableName(const char* name) ;
      const char* tableName() const { return m_name ; }

   private: // static members
      static Allocator s_allocator ;
   protected:
      SymHashSet m_symbols ;
      unsigned   m_table_id ;
      char*      m_name { nullptr } ;
      
   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      SymbolTable(size_t initial_size) ;
      SymbolTable(const SymbolTable&) ;
      ~SymbolTable() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<SymbolTable> ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<SymbolTable*>(obj) ; }
      static void shallowFree_(Object* obj) { free_(obj) ; }

      // type determination predicates
      static bool isSymbolTable_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "SymbolTable" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*,ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return static_cast<const SymbolTable*>(obj)->m_symbols.size() ; }
      static bool empty_(const Object* obj) { return size_(obj) != 0 ; }

      // *** standard access functions ***
      static Object* front_(Object* obj) ;
      static const Object* front_const(const Object* obj) ;

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

      // *** iterator support ***
      static Object* next_(const Object*) { return nullptr ; }
      static ObjectIter& next_iter(const Object*, ObjectIter& it) { it.incrIndex() ; return it ; }
   } ;


} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::SymbolTable> ;

} ; // end namespace FramepaC

#endif /* !_Fr_SYMBOLTABLE_H_INCLUDED */

// end of file symboltable.h //
