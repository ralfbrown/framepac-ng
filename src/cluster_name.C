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
#include "framepac/utility.h"

namespace Fr
{

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

struct ClusteringType
   {
      const char* name ;
      ClusteringAlgorithm algo ;
   } ;

struct RepType
   {
      const char* name ;
      ClusterRep  rep ;
   } ;

/************************************************************************/
/*	Global data for this module					*/
/************************************************************************/

static ClusteringType clustering_types[] = {
   { "Agglomerative", ClusteringAlgorithm::agglomerative },
   { "Annealing", ClusteringAlgorithm::annealing },
   { "Brown", ClusteringAlgorithm::brown },
   { "DBScan", ClusteringAlgorithm::dbscan },
   { "GrowSeeds", ClusteringAlgorithm::growseeds },
   { "INCR", ClusteringAlgorithm::single_link },
   { "INCR2", ClusteringAlgorithm::multipass_single_link },
   { "K-Means", ClusteringAlgorithm::kmeans },
   { "K-Medioids", ClusteringAlgorithm::kmedioids },
   { "KMeans", ClusteringAlgorithm::kmeans },
   { "KMedioids", ClusteringAlgorithm::kmedioids },
   { "Multi-Single-Link", ClusteringAlgorithm::multipass_single_link },
   { "MultiSingleLink", ClusteringAlgorithm::multipass_single_link },
   { "OPTICS", ClusteringAlgorithm::optics },
   { "Single-Link", ClusteringAlgorithm::single_link },
   { "SingleLink", ClusteringAlgorithm::single_link },
   { "SNN", ClusteringAlgorithm::snn },
   { "SharedNN", ClusteringAlgorithm::snn },
   { "Shared-NN", ClusteringAlgorithm::snn },
   { "Shared-Neighbors", ClusteringAlgorithm::snn },
   // the end-of-array sentinel
   { nullptr, ClusteringAlgorithm::none }
   } ;

static RepType rep_types[] = {
   { "Average", ClusterRep::average },
   { "Centroid", ClusterRep::centroid },
   { "Furthest", ClusterRep::furthest },
   { "Medioid", ClusterRep::medioid },
   { "Nearest", ClusterRep::nearest },
   { "Newest", ClusterRep::newest },
   { "Prototype", ClusterRep::prototype },
   { "RMS", ClusterRep::rms },
   // the end-of-array sentinel
   { nullptr, ClusterRep::none },
   } ;

/************************************************************************/
/************************************************************************/

static const void* next_name(const void* ptr)
{
   const ClusteringType* name = reinterpret_cast<const ClusteringType*>(ptr) ;
   ++name ;
   return  (name->name == nullptr) ? nullptr : name ;
}

//----------------------------------------------------------------------------

static const char* get_key(const void* ptr)
{
   const ClusteringType* name = reinterpret_cast<const ClusteringType*>(ptr) ;
   return name->name ;
}

//----------------------------------------------------------------------------

ListPtr enumerate_cluster_algo_names(const char* prefix)
{
   PrefixMatcher matcher(get_key,next_name) ;
   if (prefix && *prefix)
      return matcher.enumerateMatches(prefix,clustering_types) ;
   else
      return matcher.enumerateKeys(clustering_types) ;
}

//----------------------------------------------------------------------------

ClusteringAlgorithm parse_cluster_algo_name(const char* name)
{
   if (!name || !*name)
      return ClusteringAlgorithm::none ;
   PrefixMatcher matcher(get_key,next_name) ;
   const void* found = matcher.match(name,clustering_types) ;
   return found ? reinterpret_cast<const ClusteringType*>(found)->algo : ClusteringAlgorithm::none ;
}

//----------------------------------------------------------------------------

ListPtr enumerate_cluster_rep_names(const char* prefix)
{
   PrefixMatcher matcher(get_key,next_name) ;
   if (prefix && *prefix)
      return matcher.enumerateMatches(prefix,rep_types) ;
   else
      return matcher.enumerateKeys(rep_types) ;
}

//----------------------------------------------------------------------------

ClusterRep parse_cluster_rep_name(const char* name)
{
   if (!name || !*name)
      return ClusterRep::none ;
   PrefixMatcher matcher(get_key,next_name) ;
   const void* found = matcher.match(name,rep_types) ;
   return found ? reinterpret_cast<const RepType*>(found)->rep : ClusterRep::none ;
}

} // end namespace Fr

// end of file cluster_name.C //
