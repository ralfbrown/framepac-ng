/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-25					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2018 Carnegie Mellon University			*/
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

#ifndef _Fr_SMARTPTR_H_INCLUDED
#define _Fr_SMARTPTR_H_INCLUDED

namespace Fr
{

//----------------------------------------------------------------------------

template <typename T>
class NewPtr
   {
   public:
      typedef T* pointer ;
      typedef T element_type ;
   public:
      NewPtr() { m_string = nullptr ; }
      NewPtr(unsigned N) { m_string = new T[N] ; }
      NewPtr(unsigned N, const T* s, unsigned copyN)
	 { m_string = new T[N] ; std::copy(s,s+copyN,m_string) ; }
      NewPtr(T* s) { m_string = s ; }
      NewPtr(const NewPtr&) = delete ;
      NewPtr(NewPtr& orig) { m_string = orig.move() ; }
      NewPtr(NewPtr&& orig) { m_string = orig.move() ; }
      ~NewPtr() { reset(nullptr) ; }
      NewPtr& operator= (const NewPtr&) = delete ;
      NewPtr& operator= (NewPtr& orig) { reset(orig.release()) ; return *this ; }
      NewPtr& operator= (NewPtr&& orig) { reset(orig.release()) ; return *this ; }
      NewPtr& operator= (T* new_s) { reset(new_s) ; return *this ; }

      T* get() const noexcept { return m_string ; }
      T* move() { T* s = m_string ; clear() ; return s ; }
      void clear() { m_string = nullptr ; }
      void reset(T* ptr) { T* old = m_string ; m_string = ptr ; delete[] old ; }
      T* release() { T* ptr = m_string ; clear() ; return ptr ; }

      T* operator-> () { return m_string ; }
      const T* operator-> () const { return m_string ; }
      T* operator* () { return m_string ; }
      const T* operator* () const { return m_string ; }
      explicit operator T* () const { return m_string ; }
      operator const T* () const { return m_string ; }
      T& operator[] (size_t N) { return m_string[N] ; }
      const T& operator[] (size_t N) const { return m_string[N] ; }
      explicit operator bool () const { return m_string != nullptr ; }
      bool operator ! () const { return m_string == nullptr ; }
   protected:
      T* m_string ;
   } ;

} // end of namespace Fr



#endif /* !_Fr_SMARTPTR_H_INCLUDED */

// end of file smartptr.h //
