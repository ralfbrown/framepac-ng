/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-04					*/
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

#include "framepac/contextcoll.h"

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::ContextVectorCollection()
{

   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::~ContextVectorCollection()
{

   return ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
bool ContextVectorCollection<KeyT,IdxT,ValT,sparse>::setTermVector(const KeyT term, typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type vector)
{
   //TODO
   (void)term; (void)vector;
   return true ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::getTermVector(const KeyT term) const
{
   //TODO
   (void)term;
   return nullptr ;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
bool ContextVectorCollection<KeyT,IdxT,ValT,sparse>::updateContextVector(const KeyT key, const KeyT term, double wt)
{
   //TODO
   (void)key; (void)term; (void)wt;
   return true;
}

//----------------------------------------------------------------------------

template <typename KeyT, typename IdxT, typename ValT, bool sparse>
typename ContextVectorCollection<KeyT,IdxT,ValT,sparse>::context_type
ContextVectorCollection<KeyT,IdxT,ValT,sparse>::getContextVector(const KeyT key) const
{
   //TODO
   (void)key;
   return nullptr ;
}

//----------------------------------------------------------------------------


} // end namespace Fr

// end of file contextcoll.cc //
