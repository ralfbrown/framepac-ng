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

#include "framepac/object.h"
#include "framepac/charget.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

class Number ;

typedef Object* ObjectReaderFunc(const class ObjectReader*, CharGetter&) ;

class ObjectReader
   {
   protected:
      static ObjectReader* s_current ;
      static ObjectReaderFunc* s_defaultdispatch[256] ;
      ObjectReaderFunc* m_dispatch[256] ;// function to call for any given starting byte
   public:
      ObjectReader() ;
      ~ObjectReader() ;

      static bool initialize() ;
      static ObjectReader *current() { return s_current ; }

      Object* read(CharGetter&) const ;
      Object* readObject(istream&) const ;
      Object* readObject(FILE*) const ;
      Object* readObject(char*&) const ;
      Object* readObject(const char*&) const ;
      // unlike the above, the following reader does not provide any way to find out the position of left-over
      //   input, so it is only useable for a single object per string
      Object* readObject(const std::string&) const ;

      Number* readNumber(CharGetter&) const ;
      
      ObjectReaderFunc* getDispatcher(unsigned char index) const
         { return m_dispatch[index] ; }
      void registerDispatcher(unsigned index, ObjectReaderFunc* fn)
         { m_dispatch[index] = fn ; }
   } ;

//----------------------------------------------------------------------------
// a very thing wrapper around ObjectReader that simply changes the dispatchers
//   to handle data in JSON format instead of Lisp format

class JSONReader : public ObjectReader
   {
   public:
      JSONReader() ;
      ~JSONReader() ;
   } ;

//----------------------------------------------------------------------------

istream& operator >> (istream&, ObjectPtr&) ;

inline istream& operator >> (istream& in, Object*& obj)
{
   obj = ObjectReader::current()->readObject(in) ;
   return in ;
}

} ; // end namespace Fr

// end of file objreader.h //
