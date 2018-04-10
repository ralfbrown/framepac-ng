/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.04, last edit 2018-04-09					*/
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

#include "framepac/argparser.h"
#include "framepac/cluster.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

int main(int argc, char** argv)
{
   const char* algo_name ;
   const char* vecsim_name ;
   const char* cluster_options ;

   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(algo_name,"a","algorithm","name of clustering algorithm to use (k-means, etc.)","k-means")
      .add(vecsim_name,"m","measure","name of similarity measure (cosine, etc.)","cosine")
      .add(cluster_options,"O","options","options to pass to clustering algorithm","")
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
//   VectorSimilarityMeasure vecsim = parse_vector_measure_name(vecsim_name) ;
//   ClusteringAlgorithm algo = parse_cluster_algo_name(algo_name) ;
   auto clusterer = ClusteringAlgo<uint32_t,float>::instantiate(algo_name,cluster_options) ;
   //TODO

   delete clusterer ;
   return 0 ;
}

// end of file clustertest.C //
