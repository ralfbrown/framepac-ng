/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.06, last edit 2018-04-28					*/
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
#include "framepac/progress.h"
#include "template/bufbuilder.cc"

namespace Fr
{

// explicitly instantiate
bool convert_string(const char*&,ClusteringAlgoOption)
{ return false ; }
template class BufferBuilder<ClusteringAlgoOption> ;

/************************************************************************/
/*	Methods for class ClusteringAlgoBase				*/
/************************************************************************/

ClusteringAlgoOption* ClusteringAlgoBase::parseOptions(const char* opt)
{
   BufferBuilder<ClusteringAlgoOption> options ;
   while (opt && *opt)
      {

      //FIXME
      opt++;
      }
   return options.finalize() ;
}

//----------------------------------------------------------------------------

void ClusteringAlgoBase::freeOptions(ClusteringAlgoOption* options)
{
   delete[] options ;
   return ;
}

//----------------------------------------------------------------------------

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

ProgressIndicator* ClusteringAlgoBase::makeProgressIndicator(size_t limit) const
{
   if (this->verbosity() >= 0)
      {
      ProgressIndicator* prog = new ConsoleProgressIndicator(1,limit,50,"","") ;
      prog->showElapsedTime(true) ;
      prog->showRemainingTime(true) ;
      return prog ;
      }
   else
      return new NullProgressIndicator ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file cluster.C //
