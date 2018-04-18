/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-17					*/
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

#include "framepac/cluster.h"
#include "framepac/message.h"

namespace Fr
{

/************************************************************************/
/*	Methods for class ClusteringAlgoBase				*/
/************************************************************************/

bool ClusteringAlgoBase::checkSparseOrDense(const Array* vectors)
{
   if (vectors->size() <= 1) return true ;
   Object* o = vectors->getNth(0) ;
   bool sparse = o && o->isSparseVector() ;
   for (size_t i = 1 ; i < vectors->size() ; ++i)
      {
      /*Object* */o = vectors->getNth(i) ;
      if (o && o->isSparseVector() != sparse)
	 return false ;			// array contains both sparse and dense vectors
      }
   return true ;			// vectors are all of the same type
}

//----------------------------------------------------------------------------

void ClusteringAlgoBase::freeClusters(ClusterInfo** clusters, size_t num_clusters)
{
   for (size_t i = 0 ; i < num_clusters ; ++i)
      {
      if (clusters[i])
	 clusters[i]->free() ;
      }
   delete[] clusters ;
   return ;
}

//----------------------------------------------------------------------------

void ClusteringAlgoBase::log(int level, const char* fmt, ...) const
{
   if (level <= m_verbosity)
      {
      va_list args ;
      va_start(args,fmt) ;
      SystemMessage::status(fmt,args) ;
      }
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file cluster.C //
