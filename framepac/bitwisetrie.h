/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-08					*/
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

#ifndef __Fr_BITWISETRIE_H_INCLUDED
#define __Fr_BITWISETRIE_H_INCLUDED

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename T, unsigned bits = 4, typename IdxT = uint32_t>
class BitwiseTrieNode
   {
   public:
      void *operator new(size_t, void *where) { return where ; }
      BitwiseTrieNode() ;
      ~BitwiseTrieNode() ;

      // accessors
      bool isLeaf() const { return m_leaf ; }
      bool hasChildren() const ;
      bool childPresent(unsigned N) const ;
      IdxT childIndex(unsigned N) const ;
      T value() const { return m_value ; }

      // modifiers
      void markAsLeaf() { m_leaf = true ; }
      void setValue(T v) { m_value = v ; }
      void setChild(unsigned N, IdxT child_idx) { m_children[N] = child_idx ; }
      bool insertChild(unsigned N, ...) ;

   protected:
      IdxT  m_children[1<<bits] ;
      T     m_value ;
      bool  m_leaf ;
   } ;

//----------------------------------------------------------------------------

template <typename T, unsigned bits = 4, typename IdxT = uint32_t>
class BitwiseTrie
   {
   public:
      BitwiseTrie() ;
      ~BitwiseTrie() ;

   protected:

   } ;

} // end namespace Fr

#endif /* !__Fr_BITWISETRIE_H_INCLUDED */

// end of file bitwisetrie.h //
