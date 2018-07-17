/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.07, last edit 2018-07-17					*/
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

#ifndef _Fr_MAP_H_INCLUDED
#define _Fr_MAP_H_INCLUDED

#include "framepac/hashtable.h"

namespace Fr {

//----------------------------------------------------------------------------
// forward declarations

class CFile ;
class List ;
class Map ;

//----------------------------------------------------------------------------

class MapIter
   {
   private:
      Map   *m_map ;
      size_t m_index ;
   public:
      MapIter(Map *m,size_t idx) : m_map(m), m_index(idx) {}
      MapIter(const MapIter &) = default ;
      ~MapIter() = default ;

      Object* operator* () ;
      const Object* operator* () const ;
      Object* operator-> () ;
      MapIter& operator++ () { ++m_index ; return *this ; }
      const Object* operator[] (size_t index) const ;

      bool operator== (const MapIter& other) const { return m_map == other.m_map && m_index == other.m_index ; }
      bool operator!= (const MapIter& other) const { return m_map != other.m_map || m_index != other.m_index ; }
   } ;

//----------------------------------------------------------------------------

class Map : public Object
   {
   public: // types
      typedef Object super ;
      typedef bool remove_fn(Object*,Object*) ;
   public:
      static Map *create(size_t capacity = 0) { return new Map(capacity) ; }
      static Map *create(const List *) ;
      static Map *create(const Map *orig) { return new Map(orig) ; }

      bool load(CFile&) ;
      bool load(void* mmap_base, size_t mmap_len) ;
      bool save(CFile&) ;

      bool add(Object* key, Object* value = nullptr) { return m_map.add(key,value) ; }
      Object* lookup(const Object* key) const { return m_map.lookup(const_cast<Object*>(key)) ; }

      void onRemove(remove_fn* fn) { m_map.onRemove(fn) ; }

      // *** standard info functions ***
      size_t size() const { return m_size ; }
      size_t empty() const { return size() == 0 ; }

      // *** standard access functions ***
      Object *front() const { return *cbegin() ; }
      Map *subseq(MapIter start, MapIter stop, bool shallow = false) const ;

      // *** iterator support ***
      MapIter begin() { return MapIter(this,0) ; }
      MapIter cbegin() const { return MapIter(const_cast<Map*>(this),0) ; }
      MapIter end() { return MapIter(this,size()) ; }
      MapIter cend() const { return MapIter(const_cast<Map*>(this),size()) ; }

      // STL compatibility
      bool reserve(size_t n) ;
      
   protected: // construction/destruction
      void *operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void *blk,size_t) { s_allocator.release(blk) ; }
      Map(size_t initial_size = 0) ;
      Map(const Map* orig) ;
      Map(const Map& orig) ;
      ~Map() ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<Map> ;
      friend class MapIter ;
      friend class SymbolTable ; //TEMP

      // type determination predicates
      static bool isMap_(const Object*) { return true ; }
      static const char *typeName_(const Object*) { return "Map" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object* obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*, size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*, char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
			        size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return static_cast<const Map*>(obj)->size() ; }
      static bool empty_(const Object* obj) { return static_cast<const Map*>(obj)->empty() ; }

      // *** standard access functions ***
      static Object* front_(Object* obj) { return static_cast<const Map*>(obj)->front() ; }
      static const Object* front_(const Object *obj) { return static_cast<const Map*>(obj)->front() ; }

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

      // *** iterator support ***

   private: // static members
      static Allocator s_allocator ;
   protected:
      size_t       m_size ;
      ObjHashTable m_map ;
   } ;

//----------------------------------------------------------------------------

//inline Object* MapIter::operator* () { return m_map->getIndex(m_index) ; }
//inline const Object* MapIter::operator* () const { return m_map->getIndex(m_index) ; }
inline Object* MapIter::operator* () { return nullptr ; } //FIXME
inline const Object* MapIter::operator* () const { return nullptr ; } //FIXME

} ; // end namespace Fr

/************************************************************************/
/************************************************************************/

namespace FramepaC
{

extern template class FramepaC::Object_VMT<Fr::Map> ;

} ; // end namespace FramepaC

#endif /* !_Fr_MAP_H_INCLUDED */

// end of file map.h //
