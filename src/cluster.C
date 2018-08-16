/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-15					*/
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
#include "framepac/convert.h"
#include "framepac/cstring.h"
#include "framepac/message.h"
#include "framepac/progress.h"
#include "framepac/signal.h"
#include "framepac/texttransforms.h"

namespace Fr
{

SignalHandler* ClusteringAlgoBase::s_sigint = nullptr ;
std::sig_atomic_t ClusteringAlgoBase::abort_requested = 0 ;

/************************************************************************/
/*	Methods for class ClusteringAlgoBase				*/
/************************************************************************/

bool ClusteringAlgoBase::parseOptions(const char* opt)
{
   if (!opt)
      return true ;
   bool all_parsed = true ;
   while (*opt)
      {
      opt = skip_whitespace(opt) ;
      size_t len = 0 ;
      while (opt[len] && opt[len] != '=' && opt[len] != ':')
	 len++ ;
      CharPtr optname { dup_string_n(opt,len) } ;
      lowercase_string((char*)optname) ;
      opt += len ;
      char* optvalue_orig ;
      if (*opt == '=')
	 {
	 opt = skip_whitespace(opt+1) ;
	 len = 0 ;
	 while (opt[len] && opt[len] != ':')
	    len++ ;
	 optvalue_orig = dup_string_n(opt,len).move() ;
	 opt += len ;
	 }
      else
	 optvalue_orig = dup_string("").move() ;
      const char* optvalue = optvalue_orig ;
      if (strcmp(optname,"a") == 0 || strcmp(optname,"alpha") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_alpha) ;
	 }
      else if (strcmp(optname,"b") == 0 || strcmp(optname,"beta") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_beta) ;
	 }
      else if (strcmp(optname,"gamma") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_gamma) ;
	 }
      else if (strcmp(optname,"k") == 0 || strcmp(optname,"numclusters") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_desired_clusters) ;
	 }
      else if (strcmp(optname,"it") == 0 || strcmp(optname,"iter") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_max_iterations) ;
	 }
      else if (strcmp(optname,"eps") == 0 || strcmp(optname,"thr") == 0 || strcmp(optname,"threshold") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_threshold) ;
	 }
      else if (strcmp(optname,"minpts") == 0 || strcmp(optname,"pts") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_min_points) ;
	 }
      else if (strcmp(optname,"v") == 0 || strcmp(optname,"verbosity") == 0)
	 {
	 all_parsed &= convert_string(optvalue,m_verbosity) ;
	 }
      //TODO: any other standard options to be parsed?
      // pass the option name and value down to the actual clustering algorithm to be used as it sees fit
      if (!applyOption(optname,optvalue))
	 all_parsed = false ;
      delete[] optvalue_orig ;
      if (*opt == ':')
	 opt++ ;
      }
   return all_parsed ;
}

//----------------------------------------------------------------------------

bool ClusteringAlgoBase::checkSparseOrDense(const Array* vectors)
{
   if (vectors->size() <= 1) return true ;
   Object* o = vectors->getNth(0) ;
   bool sparse = o && o->isSparseVector() ;
   for (size_t i = 1 ; i < vectors->size() ; ++i)
      {
      o = vectors->getNth(i) ;
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

void ClusteringAlgoBase::sigint_handler(int)
{
   if (!abort_requested)
      {
      abort_requested = 1 ;
      SystemMessage::status("*** User interrupt: clustering will be terminated after the current iteration ***");
      }
   return ;
}

//----------------------------------------------------------------------------

void ClusteringAlgoBase::trapSigInt() const
{
   if (!s_sigint)
      {
      abort_requested = 0 ;
      s_sigint = new SignalHandler(SIGINT,this->sigint_handler) ;
      }
   return ;
}

//----------------------------------------------------------------------------

void ClusteringAlgoBase::untrapSigInt() const
{
   delete s_sigint ;
   s_sigint = nullptr ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

// end of file cluster.C //
