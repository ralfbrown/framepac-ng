/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-16					*/
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
#include "framepac/cstring.h"
#include "framepac/file.h"
#include "framepac/message.h"
#include "framepac/threadpool.h"
#include "framepac/timer.h"

using namespace Fr ;

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

static void indent_to(size_t level, ostream& out)
{
   for (size_t i = 0 ; i < 2*level ; ++i)
      out << ' ' ;
   return ;
}

//----------------------------------------------------------------------------

static void show_clusters(const ClusterInfo* clusters, size_t level)
{
   indent_to(level,cout) ;
   if (clusters->label())
      {
      cout << "Cluster " << clusters->label() << ':' << endl ;
      }
   else
      {
      cout << "Cluster:" << endl ;
      }
   if (clusters->members())
      {
      indent_to(level+1,cout) ;
      cout << "Direct members:" << endl ;
      size_t indent = 2*(level+2) ;
      for (auto v : *clusters->members())
	 {
	 auto vector = static_cast<Vector<float>*>(v) ;
	 CharPtr printed { vector->cString(0,indent,indent+1) } ;
	 cout << *printed << endl ;
	 }
      }
   if (clusters->subclusters())
      {
      indent_to(level+1,cout) ;
      cout << "Subclusters:" << endl ;
      for (auto sub : *clusters->subclusters())
	 {
	 show_clusters(static_cast<const ClusterInfo*>(sub),level+2) ;
	 }
      }
   return  ;
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
   const char* algo_name { "k-means" } ;
   const char* vecsim_name { "cosine" } ;
   const char* cluster_options { "" } ;
   const char* vector_file { nullptr } ;
   bool use_sparse_vectors { false } ;
   bool dump_vectors { false } ;
   int threads { -1 } ;

   Fr::Initialize() ;
   ArgParser cmdline_flags ;
   cmdline_flags
      .add(algo_name,"a","algorithm","name of clustering algorithm to use (k-means, etc.)")
      .add(dump_vectors,"D","dump","output the vectors to be clustered")
      .add(threads,"j","threads","number of worker threads to use (default=number of cores)")
      .add(vecsim_name,"m","measure","name of similarity measure (cosine, etc.)")
      .add(cluster_options,"O","options","options to pass to clustering algorithm")
      .add(use_sparse_vectors,"s","sparse","use sparse vectors instead of dense vectors")
      .add(vector_file,"V","vectors","file containing vectors to be clustered")
      .addHelp("h","help","show usage summary") ;
   if (!cmdline_flags.parseArgs(argc,argv))
      {
      cmdline_flags.showHelp() ;
      return 1 ;
      }
//   VectorSimilarityMeasure vecsim = parse_vector_measure_name(vecsim_name) ;
//   ClusteringAlgorithm algo = parse_cluster_algo_name(algo_name) ;
   auto clusterer = ClusteringAlgo<uint32_t,float>::instantiate(algo_name,cluster_options) ;
   ScopedObject<Array> vectors ;
   if (vector_file)
      {
      CInputFile vecfile(vector_file) ;
      if (!vecfile)
	 {
	 SystemMessage::error("unable to open file") ;
	 }
      else
	 {
	 for ( ; ; )
	    {
	    CharPtr line = vecfile.getTrimmedLine() ;
	    if (!line)
	       break  ;
	    if (!**line) continue ;
	    Vector<float>* v;
	    if (use_sparse_vectors)
	       {
	       v = SparseVector<uint32_t,float>::create(line) ;
	       }
	    else
	       {
	       v = DenseVector<float>::create(line) ;
	       }
	    if (v)
	       {
	       vectors->append(v) ;
	       v->free() ;
	       }
	    }
	 }
      }
   else
      {
      // generate some random vectors
      //TODO
      }
   if (dump_vectors)
      {
      cout << vectors->size() << " vectors to be clustered" << endl ;
      for (auto vec : *vectors)
	 {
	 cout << vec << endl ;
	 }
      cout << "   ====   " << endl ;
      }
   if (threads >= 0)
      {
      ThreadPool::defaultPool(new ThreadPool(threads)) ;
      }
   cout << "Starting " << clusterer->algorithmName() << " clustering using " << clusterer->measureName()
	<< " similarity" << endl ;
   ClusterInfo* clusters = clusterer->cluster(vectors->begin(),vectors->end()) ;
   delete clusterer ;
   if (!clusters)
      {
      cout << "Clustering FAILED!" << endl ;
      clusters = ClusterInfo::create() ;
      }
   if (dump_vectors)
      {
      CharPtr printed { clusters->cString() } ;
      cout << "Resulting ClusterInfo structure: " << endl << *printed << endl << endl ;
      auto members = clusters->allMembers() ;
      if (members)
	 {
	 for (auto v : *members)
	    {
	    cout << v << endl ;
	    }
	 }
      }
   show_clusters(clusters,0) ;
   clusters->free() ;
   return 0 ;
}

// end of file clustertest.C //
