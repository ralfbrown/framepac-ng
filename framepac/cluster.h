/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-18					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017,2018 Carnegie Mellon University		*/
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

#ifndef __FrCLUSTER_H_INCLUDED
#define __FrCLUSTER_H_INCLUDED

#include "framepac/array.h"
#include "framepac/list.h"
#include "framepac/symbol.h"
#include "framepac/vecsim.h"

namespace Fr {

//----------------------------------------------------------------------------

class ProgressIndicator ;

//----------------------------------------------------------------------------

enum class ClusteringAlgorithm
   {
   none,
   agglomerative,		// AGGLOM from first-gen FramepaC
   annealing,
   brown,
   dbscan,
   growseeds,
   kmeans,
   kmediods,
   multipass_single_link,	// INCR2 from first-gen FramepaC
   optics,
   single_link			// INCR from first-gen FramepaC
   } ;

//----------------------------------------------------------------------------
//   due to the circular dependencies, we can't actually define all of
//   the functions inline in the iterator class definition; the rest
//   will be defined after the underlying class has been declared

class ClusterInfo ;

class ClusterInfoIter
   {
   private:
      Array*	 m_members ;
      size_t	 m_index ;
   public:
      ClusterInfoIter() : m_members(nullptr), m_index(~0U) {}
      ClusterInfoIter(ClusterInfo* inf) ;
      ClusterInfoIter(const ClusterInfo* inf) ;
      ClusterInfoIter(const ClusterInfoIter &it) = default ;
      ~ClusterInfoIter() = default ;

      inline Object* operator* () const ;
      const Array* operator-> () const { return m_members ; }
      inline ClusterInfoIter& operator++ () ;
      bool operator== (const ClusterInfoIter& other) const { return m_members == other.m_members ; }
      bool operator!= (const ClusterInfoIter& other) const { return m_members != other.m_members ; }
   } ;

//----------------------------------------------------------------------------

class ClusterInfo : public Object
   {
   protected:
      typedef Fr::Initializer<ClusterInfo> Initializer ;

   public:
      enum Flags
	 {
	 flat,				// no subclusters
	 group				// no direct members, subclusters only
	 } ;

   public:
      void setLabel(Symbol* label) { m_label = label ; }
      Symbol* label() const { return m_label ; }

   public:
      // *** object factories ***
      static ClusterInfo* create() { return new ClusterInfo ; }
      static ClusterInfo* create(const List* members, const List* subclusters = nullptr) ;
      static ClusterInfo* create(ClusterInfo** subclus, size_t num_subclus) ;
      static ClusterInfo* create(const ClusterInfo** subclus, size_t num_subclus) ;
      static ClusterInfo* createSingletonClusters(const Array* vectors) ;
      static ClusterInfo* createSingleton(const Object* vector) ;

      ClusterInfo* merge(const ClusterInfo* other) const ;

      static Symbol* genLabel() ;

      // *** standard info functions ***
      //inherited: size_t size() const ;
      //inherited: bool empty() const ;
      operator bool () const { return !this->empty() ; }

      // *** iterator support ***
      ClusterInfoIter begin() const { return ClusterInfoIter(this) ; }
      ClusterInfoIter cbegin() const { return ClusterInfoIter(this) ; }
      static ClusterInfoIter end() { return ClusterInfoIter() ; }
      static ClusterInfoIter cend() { return ClusterInfoIter() ; }

      // *** utility functions ***
      bool contains(const Object*) const ; // is Object a member of the cluster?

      // *** access to internal state ***
      const Array* members() const { return m_members ; }
      const Array* subclusters() const { return m_subclusters ; }
      size_t numSubclusters() const { return m_subclusters->size() ; }
      uint32_t flags() const { return m_flags ; }
      bool hasFlag(Flags f) const ;

      // *** modifiers ***
      void setFlag(Flags f) ;
      void clearFlag(Flags f) ;

      bool addVector(Object*) ;
      bool addVectors(const RefArray*) ;
      bool addVectors(const RefArray& a) { return addVectors(&a) ; }

      template <typename IdxT, typename ValT>
      SparseVector<IdxT,ValT>* createSparseCentroid() const ;

      template <typename ValT>
      DenseVector<ValT>* createDenseCentroid() const ;
      
   protected:
      RefArray* m_members { nullptr } ;	// individual vectors in this cluster
      Array* m_subclusters { nullptr };	// sub-clusters (if any) of this cluster
      Object* m_rep { nullptr } ;	// representative element: centroid/mediod/etc.
      Symbol* m_label { nullptr } ;	// cluster label
      uint32_t m_size { 0 } ;		// number of elements in this cluster
      uint32_t m_flags { 0 } ;

   private: // static members
      static Allocator s_allocator ;
      static Initializer s_init ;

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      ClusterInfo() {}
      ~ClusterInfo() {}
      ClusterInfo& operator= (const ClusterInfo&) = delete ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<ClusterInfo> ;
      
      // type determination predicates
      static bool isCluster_(const Object*) { return true ; }
      static const char* typeName_(const Object*) { return "ClusterInfo" ; }

      // *** copying ***
      static ObjectPtr clone_(const Object*) ;
      static Object* shallowCopy_(const Object*obj) { return clone_(obj) ; }
      static ObjectPtr subseq_int(const Object*, size_t start, size_t stop) ;
      static ObjectPtr subseq_iter(const Object*, ObjectIter start, ObjectIter stop) ;

      // *** destroying ***
      static void free_(Object* obj) { delete static_cast<ClusterInfo*>(obj) ; }
      // use shallowFree() on a shallowCopy()
      static void shallowFree_(Object* obj) { return free_(obj) ; }

      // *** I/O ***
      // generate printed representation into a buffer
      static size_t cStringLength_(const Object*,size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static char* toCstring_(const Object*,char* buffer, size_t buflen,
			     size_t wrap_at, size_t indent, size_t wrapped_indent) ;
      static size_t jsonStringLength_(const Object*, bool wrap, size_t indent) ;
      static bool toJSONString_(const Object*, char* buffer, size_t buflen, bool wrap,
				size_t indent) ;

      // *** standard info functions ***
      static size_t size_(const Object* obj) { return reinterpret_cast<const ClusterInfo*>(obj)->m_size ; }
      static bool empty_(const Object* obj) { return size_(obj) == 0 ; }

      // *** standard access functions ***
      static Object* front_(Object*) ;
      static const Object* front_(const Object*) ;

      // *** comparison functions ***
      static size_t hashValue_(const Object*) ;
      static bool equal_(const Object* obj, const Object* other) ;
      static int compare_(const Object* obj, const Object* other) ;
      static int lessThan_(const Object* obj, const Object* other) ;

   public:
      // *** startup/shutdown functions ***
      static void StaticInitialization() ;
      static void StaticCleanup() ;
   } ;

//----------------------------------------------------------------------------
// deferred definitions of functions subject to circular dependencies

inline ClusterInfoIter::ClusterInfoIter(ClusterInfo* inf)
   : m_members(const_cast<Array*>(inf->members())), m_index(0)
{
   return  ;
}

//----------------------------------------------------------------------------

inline ClusterInfoIter::ClusterInfoIter(const ClusterInfo* inf)
   : m_members(const_cast<Array*>(inf->members())), m_index(0)
{
   return ;
}

//----------------------------------------------------------------------------

class ClusteringAlgoBase
   {
   public:
      ClusteringAlgoBase() {}
      ~ClusteringAlgoBase() {}

      static bool checkSparseOrDense(const Array* vectors) ;
      static void freeClusters(ClusterInfo** clusters, size_t num_clusters) ;
      void log(int level, const char* fmt, ...) const ;
      ProgressIndicator* makeProgressIndicator(size_t limit) const ;

      void useSparseVectors(bool use) { m_use_sparse_vectors = use ; }
      void clusterThreshold(double thr) { m_threshold = thr ; }
      void desiredClusters(size_t N) { m_desired_clusters = N ; }
      void verbosity(int v) { m_verbosity = v ; }
      void maxIterations(size_t N) { m_max_iterations = N ; }

      double clusterThreshold() const { return m_threshold ; }
      size_t desiredClusters() const { return m_desired_clusters ; }
      size_t maxIterations() const { return m_max_iterations ; }
      int verbosity() const { return m_verbosity ; }
      bool usingSparseVectors() const { return m_use_sparse_vectors ; }

   protected:
      double	m_threshold { -999.99 } ;
      size_t    m_desired_clusters { 2 } ;
      size_t    m_max_iterations { 5 } ;
      int	m_verbosity { 0 } ;
      bool	m_use_sparse_vectors { false } ;
   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class ClusteringAlgo : public ClusteringAlgoBase
   {
   public:
      static ClusteringAlgo* instantiate(const char* algo_name, const char* options) ;
      virtual ~ClusteringAlgo() {}

      virtual const char* algorithmName() const = 0 ;
      const char* measureName() const { return m_measure ? m_measure->canonicalName() : "(none)" ; }

      ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) const ;
      ClusterInfo* cluster(ArrayIter first, ArrayIter past_end) const ;
      virtual ClusterInfo* cluster(const Array* vectors) const = 0 ;

      static Vector<ValT>* nearestNeighbor(const Vector<ValT>* vector, const Array* centers,
	 VectorMeasure<IdxT,ValT>* measure, double threshold = -1.0) ;
   protected: //methods
      ClusteringAlgo() {}


      Vector<ValT>* nearestNeighbor(const Vector<ValT>* vector, const Array* centers, double threshold = -1.0) const
	 { return nearestNeighbor(vector,centers,m_measure,threshold) ; }
      size_t assignToNearest(const Array* vectors, const Array* centers, ProgressIndicator *prog = nullptr,
	 double threshold = -1.0) const ;
      bool extractClusters(const Array* vectors, ClusterInfo**& clusters, size_t& num_clusters,
	 RefArray* unassigned = nullptr) const ;

   protected: // data
      VectorMeasure<IdxT,ValT>* m_measure { nullptr } ;
   } ;

/************************************************************************/
/************************************************************************/

ClusteringAlgorithm parse_cluster_algo_name(const char* name) ;

} ; // end of namespace Fr

#endif /* !__FrCLUSTER_H_INCLUDED */

// end of file cluster.h //
