/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-24					*/
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
#include "framepac/utility.h"
#include "framepac/words.h"

namespace Fr
{

SignalHandler* ClusteringAlgoBase::s_sigint = nullptr ;
std::sig_atomic_t ClusteringAlgoBase::abort_requested = 0 ;

/************************************************************************/
/************************************************************************/

static const char* base_clustering_options[] =
   {
   "a",
   "alpha",
   "b",
   "beta",
   "epsilon",
   "gamma",
   "hardlimit",
   "ignoreextra",
   "it",
   "iterations",
   "k",
   "measure",
   "minpoints",
   "minpts",
   "numclusters",
   "points",
   "pts",
   "representative",
   "singletons",
   "thr",
   "threshold",
   "v",
   "verbosity"
   } ;

/************************************************************************/
/*	Methods for class ClusteringAlgoBase				*/
/************************************************************************/

bool ClusteringAlgoBase::parseOptions(const char* optlist, bool validate_only)
{
   if (!optlist)
      return true ;
   WordSplitterDelimiter splitter(optlist,':') ;
   ListPtr options = splitter.allWords() ;
   bool all_parsed = true ;
   PrefixMatcher matcher(base_clustering_options) ;
   for (const auto option : *options)
      {
      const String* optstr = static_cast<const String*>(option) ;
      const char* opt = skip_whitespace(optstr->c_str()) ;
      char optflag = '\0' ;
      if (*opt == '+')
	 {
	 optflag = '+' ;
	 ++opt ;
	 }
      else if (*opt == '-')
	 {
	 optflag = '-' ;
	 ++opt ;
	 }
      else if (*opt == '^' || *opt == '!')
	 {
	 optflag = '!' ;
	 ++opt ;
	 }
      size_t len = 0 ;
      while (opt[len] && opt[len] != '=')
	 len++ ;
      CharPtr optname { dup_string_n(opt,len) } ;
      lowercase_string((char*)optname) ;
      opt += len ;
      const char* canon_name = matcher.match(optname) ;
      if (canon_name)
	 {
	 if (*opt == '=')
	    ++opt ;
	 opt = skip_whitespace(opt) ;
	 // pass the option name and value down to the actual clustering algorithm to be used as it sees fit
	 if (!validate_only)
	    all_parsed &= applyOption(canon_name,opt,optflag) ;
	 }
      else
	 {
	 if (matcher.status() == PrefixMatcher::NoMatch)
	    {
	    cerr << "Option " << *optname << " is not recognized" << endl ;
	    }
	 else if (matcher.status() == PrefixMatcher::Ambiguous)
	    {
	    cerr << "Option " << *optname << " is ambiguous, and matches:" ;
	    ListPtr matches = matcher.enumerateMatches(optname) ;
	    for (auto match : *matches)
	       {
	       cerr << ' ' << match->printableName() ;
	       }
	    cerr << endl ;
	    }
	 }
      }
   return all_parsed ;
}

//----------------------------------------------------------------------------

static void set_flag(bool& flag, char option, bool def = true)
{
   if (option == '-')
      flag = false ;
   else if (option  == '+')
      flag = true ;
   else if (option == '!')
      flag = !flag ;
   else
      flag = def ;
   return ;
}

//----------------------------------------------------------------------------

bool ClusteringAlgoBase::applyOption(const char* optname, const char* optvalue, char optflag)
{
   if (strcmp(optname,"a") == 0 || strcmp(optname,"alpha") == 0)
      {
      return convert_string(optvalue,m_alpha) ;
      }
   else if (strcmp(optname,"b") == 0 || strcmp(optname,"beta") == 0)
      {
      return convert_string(optvalue,m_beta) ;
      }
   else if (strcmp(optname,"gamma") == 0)
      {
      return convert_string(optvalue,m_gamma) ;
      }
   else if (strcmp(optname,"hardlimit") == 0)
      {
      set_flag(m_hard_limit,optflag) ;
      }
   else if (strcmp(optname,"ignoreextra") == 0)
      {
      set_flag(m_ignore_extra,optflag) ;
      }
   else if (strcmp(optname,"singletons") == 0)
      {
      set_flag(m_allow_singletons,optflag) ;
      }
   else if (strcmp(optname,"representative") == 0)
      {
      //TODO
      }
   else if (strcmp(optname,"measure") == 0)
      {
      //TODO
      }
   else if (strcmp(optname,"k") == 0 || strcmp(optname,"numclusters") == 0)
      {
      return convert_string(optvalue,m_desired_clusters) ;
      }
   else if (strcmp(optname,"it") == 0 || strcmp(optname,"iter") == 0)
      {
      return convert_string(optvalue,m_max_iterations) ;
      }
   else if (strcmp(optname,"eps") == 0 || strcmp(optname,"thr") == 0 || strcmp(optname,"threshold") == 0)
      {
      return convert_string(optvalue,m_threshold) ;
      }
   else if (strcmp(optname,"minpts") == 0 || strcmp(optname,"pts") == 0)
      {
      return convert_string(optvalue,m_min_points) ;
      }
   else if (strcmp(optname,"v") == 0 || strcmp(optname,"verbosity") == 0)
      {
      if (optflag == '-')
	 m_verbosity = 0 ;
      else if (optflag == '+' && !*optvalue)
	 m_verbosity++ ;
      else
	 return convert_string(optvalue,m_verbosity) ;
      }
   return false ;
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
   if (!clusters)
      return ;
   for (size_t i = 0 ; i < num_clusters ; ++i)
      {
      if (clusters[i])
	 clusters[i]->free() ;
      }
   delete[] clusters ;
   return ;
}

//----------------------------------------------------------------------------

void ClusteringAlgoBase::setLoggingPrefix(const char* pre)
{
   delete[] m_logprefix ;
   m_logprefix = dup_string(pre).move() ;
   return ;
}

//----------------------------------------------------------------------------

void ClusteringAlgoBase::log(int level, const char* fmt, ...) const
{
   if (level <= m_verbosity)
      {
      va_list args ;
      va_start(args,fmt) ;
      if (m_logprefix)
	 {
	 CharPtr msg = vaprintf(fmt,args) ;
	 SystemMessage::status("%s%s",m_logprefix,*msg) ;
	 }
      else
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
