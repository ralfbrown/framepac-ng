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

#ifndef _Fr_SYMBOL_H_INCLUDED
#define _Fr_SYMBOL_H_INCLUDED

#include <cstring>
#include "framepac/string.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

// forward declarations
class Frame ;
class List ;
class Symbol ;

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

class SymbolProperties
   {
   public:
      static SymbolProperties* create() { return new SymbolProperties ; }
      void free() { delete this ; }

      const Object* binding() const { return m_binding ; }
      Frame* frame() const { return m_frame ; }
      Symbol* inverseRelation() const { return m_invrelation  ; }
      Object* getProperty(Symbol* key) const ;
      List* plist() const { return m_plist ; }

      void binding(Object* b) { m_binding = b ; }
      void frame(Frame* f) ;
      void inverseRelation(Symbol* inv) { m_invrelation = inv ; }
      void setProperty(Symbol* key, Object* value) ;

   private:
      static Allocator s_allocator ;
   protected:
      ObjectPtr m_binding ;		// the symbol's value
      Symbol*   m_invrelation ;		// if the symbol is a relation, this is the inverse relation
      Frame*    m_frame ;		// the frame associated with the symbol
      List*     m_plist ;		// assoc-list of other properties

   protected: // methods
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      SymbolProperties() ;
      ~SymbolProperties() ;
} ;

//----------------------------------------------------------------------------

class Symbol : public String
   {
   public: // types
      typedef String super ;
   public:
      static Symbol* create(const char* name) { return new Symbol(name) ; }
      static Symbol* create(const String* str) { return new Symbol(str) ; }
      static Symbol* create(const Object* obj) ;
      static Symbol* create(istream&) ;

      const char* name() const { return c_str() ; }

      size_t hashValue() const { return hashValue_(this) ; }
      static size_t hashValue(const Symbol*) ;
      static size_t hashValue(const char* name, size_t* len) ;

      // accessing the Symbol's properties
      const Object* binding() { SymbolProperties* prop = properties() ; return prop ? prop->binding() : nullptr ; }
      Frame* frame() { SymbolProperties* prop = properties() ; return prop ? prop->frame() : nullptr ; }
      Object* getProperty(Symbol* key) const
	 { SymbolProperties* prop = properties() ; return prop ? prop->getProperty(key) : nullptr ; }

      // updating the Symbol's properties
      void binding(Object*) ;
      bool frame(Frame*) ;
      bool setProperty(Symbol* key, Object* value) ;

      // support for inheritance and the like in Frames
      bool isRelation() const { return (flags() & RELATION_FLAG) != 0 ; }
      Symbol* inverseRelation() const ;
      bool makeRelation(Symbol* inverse) ;
      bool clearRelation() { return makeRelation(nullptr) ; }

      // utility functions for I/O
      static bool nameNeedsQuoting(const char* name) ;
      bool nameNeedsQuoting() const { return nameNeedsQuoting(c_str()) ; }

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

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      Symbol(const char* nm) : String(nm), m_properties() {}
      Symbol(const char* nm, size_t len) : String(nm,len), m_properties() {}
      Symbol(const String* nm) : String(nm), m_properties() {}
      Symbol(const Symbol& nm) : String(nm), m_properties() {}
      ~Symbol() ;
      Symbol& operator= (const Symbol&) = delete ;

      void unintern() ; // remove from the symbol table containing it

      SymbolProperties* properties() const { return m_properties.pointer() ; }
      uint16_t symtabID() const { return m_properties.extra() >> 4 ; }
      uint8_t flags() const { return m_properties.extra() & 0x0F ; }

      void setFlag(uint8_t flag) ;
      void clearFlag(uint8_t flag) ;

   public: // for now...
      void symtabID(uint16_t id) ;

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
      static const char* typeName_(const Object*) { return s_typename ; }

      // *** copying ***
      static ObjectPtr clone_(const Object* obj) { return ObjectPtr(const_cast<Object*>(obj)) ; }
      static Object* shallowCopy_(const Object* obj) { return const_cast<Object*>(obj) ; }
      static ObjectPtr subseq_int(const Object*,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*,ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
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
      static size_t hashValue_(const Object* obj) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

      // *** iterator support ***
      static Object* next_(const Object*) { return nullptr ; }
      static ObjectIter& next_iter(const Object*, ObjectIter& it) { it.incrIndex() ; return it ; }

   private: // static members
      static Allocator s_allocator ;
      static const char s_typename[] ;
   protected:
      // we pack a pointer to the symbol's properties, its symboltable
      //   ID, and some bitflags, into a single 64-bit field to save
      //   memory
      FramepaC::PointerPlus16<SymbolProperties> m_properties ;

      static constexpr uint8_t RELATION_FLAG = 1 ;
      // placeholders for future bit flags
      static constexpr uint8_t Second_FLAG = 2 ;
      static constexpr uint8_t Third_FLAG = 4 ;
      static constexpr uint8_t Fourth_FLAG = 8 ;
   } ;

/************************************************************************/
/************************************************************************/

inline istream& operator >> (istream& in, Symbol*& obj)
{
   obj = Symbol::create(in) ;
   return in ;
}

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Symbol> ;

} ; // end namespace FramepaC

#endif /* !_Fr_SYMBOL_H_INCLUDED */

// end of file symbol.h //
