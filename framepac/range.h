/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.13, last edit 2018-09-18					*/
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

#ifndef _Fr_RANGE_H_INCLUDED
#define _Fr_RANGE_H_INCLUDED

namespace Fr
{

template <typename T>
class RangeIter
   {
   public:
      RangeIter(T it) : m_it(it) {}

      RangeIter& operator++ () { ++m_it ; return *this ; }
      RangeIter& operator++ (int) { ++m_it ; return *this ; }
      RangeIter& operator-- () { --m_it ; return *this ; }
      T operator* () const { return m_it ; }

      bool operator== (const RangeIter& other) const { return m_it == other.m_it ; }
      bool operator!= (const RangeIter& other) const { return m_it != other.m_it ; }
      
   private:
      T m_it ;
   } ;

//----------------------------------------------------------------------------

template <typename T>
class Range
   {
   public:
      typedef RangeIter<T> iterator ;
   public:
      Range() {}
      Range(T b, T e) : m_first(b), m_last(e) {}
      Range(const Range&) = default ;

      Range& operator= (const Range&) = default ;

      T first() const { return m_first ; }
      T last() const { return m_last ; }

      void setFirst(T f) { m_first = f ; }
      void setLast(T l) { m_last = l ; }
      void incrFirst() { ++m_first ; }
      void decrLast() { --m_last ; }

      iterator begin() const { return iterator(m_first) ; }
      iterator end() const { return iterator(m_last) ; }

      bool operator== (const Range& other) const { return m_first == other.m_first && m_last == other.m_last ; }
      bool operator!= (const Range& other) const { return m_first != other.m_first || m_last != other.m_last ; }

   private:
      T m_first ;
      T m_last ;
   } ;

} // end namespace Fr


#endif /* !_Fr_RANGE_H_INCLUDED */

// end of file range.h //
