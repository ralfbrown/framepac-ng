/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-07-07					*/
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

#ifndef _Fr_SYMBOL_H_INCLUDED
#define _Fr_SYMBOL_H_INCLUDED

#include <cstring>
#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

//----------------------------------------------------------------------------

class SymbolIter
   {
   private:
      const char* m_symbol ;
   public:
      SymbolIter(const char* s) : m_symbol(s) {}
      SymbolIter(const SymbolIter& s) = default ;
      ~SymbolIter() = default ;

      const char& operator* () const { return *m_symbol ; }
      const char* operator-> () const { return m_symbol ; }
      SymbolIter& operator++ () { ++m_symbol ; return *this ; }
      const char& operator[] (size_t index) const { return m_symbol[index] ; }

      bool operator== (const SymbolIter& other) const { return m_symbol == other.m_symbol ; }
      bool operator!= (const SymbolIter& other) const { return m_symbol != other.m_symbol ; }
   } ;

//----------------------------------------------------------------------------

class Symbol : public String
   {
   public:
      static Symbol* create(const char* name) { return new Symbol(name) ; }
      static Symbol* create(const String* str) { return new Symbol(str) ; }
      static Symbol* create(const Object* obj) ;

      const char* name() const { return c_str() ; }

      static size_t hashValue(const char* name, size_t* len) ;

      // *** standard info functions ***
      size_t size() const { return c_len() ; }
      size_t empty() const { return false ; }

      // *** standard access functions ***
      //char front() const : inherited from String
      Symbol* subseq(SymbolIter start, SymbolIter stop, bool shallow = false) const ;

      // *** iterator support ***
      SymbolIter begin() const { return SymbolIter(name()) ; }
      SymbolIter cbegin() const { return SymbolIter(name()) ; }
      SymbolIter end() const { return SymbolIter(name()+size()) ; }
      SymbolIter cend() const { return SymbolIter(name()+size()) ; }
      Symbol *next() const { return nullptr ; }

   private: // static members
      static Allocator s_allocator ;
   protected:
      // we pack a pointer to the symbol's properties, its symboltable
      //   ID, and some bitflags, into a single 64-bit field to save
      //   memory
      FramepaC::PointerPlus16<Object> m_binding ;

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Symbol(const char* name) : String(name), m_binding() {}
      Symbol(const String* name) : String(name), m_binding() {}
      Symbol(const Symbol& name) : String(name), m_binding() {}
      ~Symbol() ;
      Symbol& operator= (const Symbol&) = delete ;

      void unintern() ; // remove from the symbol table containing it

      uint8_t symtabID() { return m_binding.extra() >> 8 ; }
      uint8_t flags() { return m_binding.extra() & 0xFF ; }
      Object* binding() { return m_binding.pointer() ; }

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Symbol> ;

      // *** destroying ***
      static void free_(Object *obj)
	 {
	 Symbol* s = static_cast<Symbol*>(obj) ;
	 if (s->symtabID() == 0) delete s ; 
	 }

      // type determination predicates
      static bool isSymbol_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "Symbol" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object* obj) { return ObjectPtr(const_cast<Object*>(obj)) ; }
      static Object* shallowCopy_(const Object* obj) { return const_cast<Object*>(obj) ; }
      static ObjectPtr subseq_int(const Object*,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*,ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      //static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      //static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
      //     size_t indent) ;
      using String::jsonStringLength_ ;
      using String::toJSONString_ ;

      // *** standard info functions ***
      static size_t size_(const Object*) { return 1 ; }
      static bool empty_(const Object*) { return false ; }

      // *** standard access functions ***
      static Object* front_(Object* obj) { return obj ; }
      static const Object* front_const(const Object* obj) { return obj ; }
      //static const char *stringValue_(const Object *obj) : inherited from String

      // *** comparison functions ***
      static size_t hashValue_(const Object* obj) { return (size_t)obj ; }
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

extern template class FramepaC::Object_VMT<Fr::Symbol> ;

} ; // end namespace FramepaC

#endif /* !_Fr_SYMBOL_H_INCLUDED */

// end of file symbol.h //
