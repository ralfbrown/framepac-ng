/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-02					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

#include "template/hashtable.cc"

namespace Fr
{

// request explicit instantiation
template class HashTable<uint32_t,NullObject> ;

template <>
bool HashTable<uint32_t,NullObject>::isEqual(const char* name, size_t namelen, uint32_t key)
{
   (void)name; (void)namelen; (void)key;
   return false ; //FIXME
}

//----------------------------------------------------------------------------

template <>
size_t HashTable<uint32_t,NullObject>::hashVal(const char* name, size_t* namelen)
{
   (void)name ;
   if (namelen) *namelen = 0 ;
   return 0 ;  //FIXME
}


} // end namespace Fr

// end of file hashset_u32.C //
