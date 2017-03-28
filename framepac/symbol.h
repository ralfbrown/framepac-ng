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

#include <cstring>
#include "framepac/object.h"
#include "framepac/map.h" //TEMP

namespace Fr
{

class SymbolTable_ ;

//----------------------------------------------------------------------------

class SymbolIter
   {
   private:
      const char *m_symbol ;
   public:
      SymbolIter(const char *s) : m_symbol(s) {}
      SymbolIter(const SymbolIter &s) = default ;
      ~SymbolIter() = default ;

      const char& operator* () const { return *m_symbol ; }
      const char* operator-> () const { return m_symbol ; }
      SymbolIter& operator++ () { ++m_symbol ; return *this ; }
      const char& operator[] (size_t index) const { return m_symbol[index] ; }

      bool operator== (const SymbolIter& other) const { return m_symbol == other.m_symbol ; }
      bool operator!= (const SymbolIter& other) const { return m_symbol != other.m_symbol ; }
   } ;

//----------------------------------------------------------------------------

class Symbol : public Object
   {
   public:
      static Symbol *create(const char *name) ;
      static Symbol *create(const Object *obj) ;
      static Symbol *create(const Symbol *sym) ;

      const char *name() const { return m_name ; }

      // *** standard info functions ***
      size_t size() const ;
      size_t empty() const { return false ; }

      // *** standard access functions ***
      char front() const { return m_name[0] ; }
      Symbol *subseq(SymbolIter start, SymbolIter stop, bool shallow = false) const ;

      // *** iterator support ***
      SymbolIter begin() const { return SymbolIter(name()) ; }
      SymbolIter cbegin() const { return SymbolIter(name()) ; }
      SymbolIter end() const { return SymbolIter(name()+size()) ; }
      SymbolIter cend() const { return SymbolIter(name()+size()) ; }
      Symbol *next() const { return nullptr ; }

   private: // static members
      static Allocator<Symbol> s_allocator ;
   protected:
      Object *m_binding ;
      uint8_t m_symtab_id ;
      uint8_t m_flags ;
      char    m_name[1] ;
   protected: // construction/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      Symbol(const char *) ;
      Symbol(const Symbol *) ;
      Symbol(const Symbol &) ;
      ~Symbol() ;
      Symbol& operator= (const Symbol&) = delete ;

      void unintern() ; // remove from the symbol table containing it

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Symbol> ;

      // *** destroying ***
      static void free_(Object *obj)
	 {
	 Symbol *s = static_cast<Symbol*>(obj) ;
	 if (s->m_symtab_id == 0) delete s ; 
	 }

      // type determination predicates
      static bool isSymbol_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "Symbol" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *obj) { return ObjectPtr(const_cast<Object*>(obj)) ; }
      static Object *shallowCopy_(const Object *obj) { return const_cast<Object*>(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object *,char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *) { return 1 ; }
      static bool empty_(const Object *) { return false ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) { return obj ; }
      static const Object *front_const(const Object *obj) { return obj ; }
      static const char *stringValue_(const Object *obj) { return ((Symbol*)obj)->m_name ; }

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }
   } ;

//----------------------------------------------------------------------------

template <int N>
class Symbol_T : public Symbol
   {
   private:
      char    m_name2[N] ;
   public:
      Symbol_T(const char *name) ;
      ~Symbol_T() ;
   } ;

//----------------------------------------------------------------------------

class SymbolTable : public Object
   {
   public:
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      SymbolTable(size_t initial_size) ;
      SymbolTable(const SymbolTable&) ;
      ~SymbolTable() ;

   private: // static members
      static Allocator<SymbolTable> s_allocator ;
   private:
      static const unsigned num_allocators =
	 (Fr::SYMBOL_MAX_NAME+FramepaC::SYMBOL_NAME_GRAN-1)/FramepaC::SYMBOL_NAME_GRAN ;
      static Allocator<Symbol> *m_allocators[num_allocators] ;//FIXME
   private:
      Map m_symbols ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<SymbolTable> ;

      // *** destroying ***
      static void free_(Object *obj) { delete static_cast<SymbolTable*>(obj) ; }
      static void shallowFree_(Object *obj) { free_(obj) ; }

      // type determination predicates
      static bool isSymbolTable_(const Object *) { return true ; }
      static const char *typeName_(const Object *) { return "SymbolTable" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object *) ;
      static Object *shallowCopy_(const Object *obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object *,size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object *,ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object *, size_t wrap_at, size_t indent) ;
      static bool toCstring_(const Object *,char *buffer, size_t buflen,
			     size_t wrap_at, size_t indent) ;
      static size_t jsonStringLength_(const Object *, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object *, char *buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object *obj) { return static_cast<const SymbolTable*>(obj)->m_symbols.size() ; }
      static bool empty_(const Object *obj) { return size_(obj) != 0 ; }

      // *** standard access functions ***
      static Object *front_(Object *obj) ;
      static const Object *front_const(const Object *obj) ;

      // *** comparison functions ***
      static bool equal_(const Object *obj, const Object *other) ;
      static int compare_(const Object *obj, const Object *other) ;
      static int lessThan_(const Object *obj, const Object *other) ;

      // *** iterator support ***
      static Object* next_(const Object *) { return nullptr ; }
      static ObjectIter& next_iter(const Object *, ObjectIter& it) { it.incrIndex() ; return it ; }
   } ;


//----------------------------------------------------------------------------

extern template class Allocator<Symbol> ;
extern template class Allocator<SymbolTable> ;

// end of namespace Fr
} ;

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Symbol> ;
extern template class FramepaC::Object_VMT<Fr::SymbolTable> ;

} ; // end namespace FramepaC

// end of file symbol.h //
