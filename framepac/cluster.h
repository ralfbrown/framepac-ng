/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.11, last edit 2018-09-11					*/
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

#include <csignal>
#include "framepac/array.h"
#include "framepac/list.h"
#include "framepac/symbol.h"
#include "framepac/vecsim.h"

namespace Fr {

//----------------------------------------------------------------------------

class ProgressIndicator ;
class SignalHandler ;

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
   kmedioids,
   multipass_single_link,	// INCR2 from first-gen FramepaC
   optics,
   single_link,			// INCR from first-gen FramepaC
   snn,				// Shared Nearest Neighbors
   tight			// TIGHT from first-gen FramepaC
   } ;

//----------------------------------------------------------------------------

enum class ClusterRep
   {
   none,
   centroid,
   medioid,
   prototype,			// first vector in cluster
   // the following are not a fixed representative, but depend on the history of additions to the cluster
   newest,			// most recently added member, e.g. members(numMembers()-1)
   // the following are not a fixed representative, but are relative to some other cluster
   average,
   furthest,
   nearest,
   rms
   } ;

//----------------------------------------------------------------------------

class ClusterInfo : public Object
   {
   public:	// types
      typedef Object super ;
      enum Flags
	 {
	 flat,				// no subclusters
	 group				// no direct members, subclusters only
	 } ;

   public:
      // *** object factories ***
      static ClusterInfo* create() { return new ClusterInfo ; }
      static ClusterInfo* create(const List* members, const List* subclusters = nullptr) ;
      static ClusterInfo* create(Object** vectors, size_t num_vectors) ;
      static ClusterInfo* create(ClusterInfo** subclus, size_t num_subclus) ;
      static ClusterInfo* create(Array* subclus) ;
      static ClusterInfo* create(const ClusterInfo** subclus, size_t num_subclus) ;
      static ClusterInfo* createSingletonClusters(const Array* vectors) ;
      static ClusterInfo* createSingleton(const Object* vector) ;

      // *** cluster manipulation
      void addMember(Object* vector) ;
      void shrink_to_fit() ; // remove any null pointers and update sizes
      ClusterInfo* merge(const ClusterInfo* other, bool flatten = false) const ;
      bool merge(size_t clusternum1, size_t clusternum2, bool flatten = false) ;
      bool flattenSubclusters() ;
      bool labelSubclusterPaths(bool (*setlabel_fn)(Object* vec,const char* label),
	 const char* path_prefix = "", const char* sep = " ") ;

      Ptr<RefArray> allMembers() const ;
      Ptr<Array> allKeys() const ;

      // note: only compares direct members, and requires them to have been sorted (as with sortMembers)
      void compareMembership(const ClusterInfo* other, ObjectOrderingFn*,
	 size_t& intersection, size_t& union_) const ;

      template <typename IdxT, typename ValT>
      double similarity(ClusterInfo* other, VectorMeasure<IdxT,ValT>* vm) ;
      template <typename IdxT, typename ValT>
      double similarity(const Vector<IdxT,ValT>* other, VectorMeasure<IdxT,ValT>* vm) ;
      template <typename IdxT, typename ValT>
      double reverseSimilarity(const Vector<IdxT,ValT>* other, VectorMeasure<IdxT,ValT>* vm) ;
      template <typename IdxT, typename ValT>
      double distance(ClusterInfo* other, VectorMeasure<IdxT,ValT>* vm) ;
      template <typename IdxT, typename ValT>
      double distance(const Vector<IdxT,ValT>* other, VectorMeasure<IdxT,ValT>* vm) ;
      template <typename IdxT, typename ValT>
      double reverseDistance(const Vector<IdxT,ValT>* other, VectorMeasure<IdxT,ValT>* vm) ;
      template <typename IdxT, typename ValT>
      void updateRepresentative(Vector<IdxT,ValT>*, VectorMeasure<IdxT,ValT>* vm) ;
      template <typename IdxT, typename ValT>
      void setRepresentative(VectorMeasure<IdxT,ValT>* vm) ;

      bool addGeneratedLabel() ;  // call genLabel() if no label yet
      NODISCARD static Symbol* genLabel() ;
      static Symbol* numberLabel() ;
      static bool isGeneratedLabel(const char* name) ;
      static bool isGeneratedLabel(const Symbol *name) { return name ? isGeneratedLabel(name->c_str()) : false ; }
      bool isGeneratedLabel() const { return label() ? isGeneratedLabel(label()) : false ; }
      static bool isNumberLabel(const char* name) ;
      static bool isNumberLabel(const Symbol* name) ;

      void setLabel(Symbol* l) { m_label = l ; }
      Symbol* label() const { return m_label ; }

      // *** standard info functions ***
      //inherited: size_t size() const ;
      //inherited: bool empty() const ;
      operator bool () const { return !this->empty() ; }

      // *** iterator support ***
      ArrayIter begin() const { return members()->begin() ; }
      ConstArrayIter cbegin() const { return members()->cbegin() ; }
      ArrayIter end() { return members()->end() ; }
      ConstArrayIter cend() { return members()->cend() ; }

      // *** utility functions ***
      bool contains(const Object*) const ; // is Object a member of the cluster?

      // *** access to internal state ***
      const Object* representative() const { return m_rep ; }
      ClusterRep repType() const { return  m_cluster_rep ; }
      const Array* members() const { return m_members ; }
      size_t numMembers() const { return m_members ? m_members->size() : 0 ; }
      const Array* subclusters() const { return m_subclusters ; }
      size_t numSubclusters() const { return m_subclusters ? m_subclusters->size() : 0 ; }
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
      template <typename IdxT, typename ValT>
      SparseVector<IdxT,ValT>* createSparseCentroid(const Array* vectors) const ;

      template <typename IdxT, typename ValT>
      DenseVector<IdxT,ValT>* createDenseCentroid() const ;
      template <typename IdxT, typename ValT>
      DenseVector<IdxT,ValT>* createDenseCentroid(const Array* vectors) const ;

      void sortMembers(ObjectOrderingFn*) ;
      void sortSubclusters(ObjectOrderingFn*) ;

   protected:
      bool allMembers(RefArray* mem) const ;
      bool allKeys(Array* mem) const ;

   protected: // construction/destruction
      void* operator new(size_t) { return s_allocator.allocate() ; }
      void operator delete(void* blk,size_t) { s_allocator.release(blk) ; }
      ClusterInfo() {}
      ~ClusterInfo() ;
      ClusterInfo& operator= (const ClusterInfo&) = delete ;

   protected: // implementation functions for virtual methods
      friend class FramepaC::Object_VMT<ClusterInfo> ;
      
      // type determination predicates
      static bool isCluster_(const Object*) { return true ; }
      static Symbol* label_(const Object* obj) { return static_cast<const ClusterInfo*>(obj)->m_label ; }

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

   protected:
      typedef Fr::Initializer<ClusterInfo> Initializer ;

   protected:
      RefArray* m_members { nullptr } ;		// individual vectors in this cluster
      ArrayPtr  m_subclusters ;			// sub-clusters (if any) of this cluster
      ObjectPtr m_rep ;				// representative element: centroid/mediod/etc.
      Symbol*   m_label { nullptr } ;		// cluster label
      uint32_t  m_size { 0 } ;			// number of elements in this cluster
      uint16_t m_flags { 0 } ;
      ClusterRep m_cluster_rep { ClusterRep::centroid } ; // what is the representative element for the cluster?

   private: // static members
      static Allocator s_allocator ;
      static Initializer s_init ;
      static const char s_typename[] ;
   } ;

//----------------------------------------------------------------------------

class ClusteringAlgoBase
   {
   public:
      typedef ProgressIndicator* makePIFunc(size_t) ;
   public:
      ClusteringAlgoBase() {}
      virtual ~ClusteringAlgoBase() { delete[] m_logprefix ; }

      bool parseOptions(const char* opt, bool validate_only = false) ;
      virtual bool validateOption(const char* optname, const char* optvalue, char optflag) const ;
      virtual bool applyOption(const char* optname, const char* optvalue, char optflag) ;

      static bool checkSparseOrDense(const Array* vectors) ;
      static void freeClusters(ClusterInfo** clusters, size_t num_clusters) ;
      [[gnu::format(gnu_printf,3,4)]]
      void log(int level, const char* fmt, ...) const ;
      ProgressIndicator* makeProgressIndicator(size_t limit, const char* prefix = nullptr) const ;

      void trapSigInt() const ;
      void untrapSigInt() const ;

      void setLoggingPrefix(const char* pre) ;
      void clusterThreshold(double thr) { m_threshold = thr ; }
      void desiredClusters(size_t N) { m_desired_clusters = N ; }
      void maxIterations(size_t N) { m_max_iterations = N ; }
      void verbosity(int v) { m_verbosity = v ; }
      void fastInitialization(bool fast) { m_fast_init = fast ; }
      void limitClusters(bool limit) { m_hard_limit = limit ; }
      void useSparseVectors(bool use) { m_use_sparse_vectors = use ; }
      void ignoreExtraClusters(bool ignore) { m_ignore_extra = ignore ; }
      void backoffSet(double b) { m_backoff = b ; }
      void excludeSingletons(bool excl) { m_allow_singletons = !excl ; }
      void setMakePIFunc(makePIFunc* fn) { m_makepi = fn ; }
      bool registerOption(const char* optname) ;  // used by derived classes to add private options to the parser
      ClusterRep clusteringRep() const { return  m_representative ; }
      VectorSimilarityMeasure similarityMeasure() const { return m_similarity ; }

      const char* loggingPrefix() const { return m_logprefix ; }
      double clusterThreshold() const { return m_threshold ; }
      size_t desiredClusters() const { return m_desired_clusters ; }
      size_t maxIterations() const { return m_max_iterations ; }
      int verbosity() const { return m_verbosity ; }
      bool usingSparseVectors() const { return m_use_sparse_vectors ; }
      bool doFastInitialization() const { return m_fast_init ; }
      bool hardLimitOnClusters() const { return m_hard_limit ; }
      bool ignoringExtraClusters() const { return m_ignore_extra ; }
      bool excludingSingletons() const { return !m_allow_singletons ; }
      double backoffStep() const { return m_backoff ; }
      bool abortRequested() const { return abort_requested ; }

   protected: // methods
      static void sigint_handler(int) ;

   protected: // data members
      char*       m_logprefix { nullptr } ;
      makePIFunc* m_makepi { nullptr } ;
      double      m_alpha { 0 } ;
      double      m_beta { 0 } ;
      double      m_gamma { 0 } ;
      double	  m_threshold { 0.2 } ;
      double      m_backoff { 0.05 } ;
      size_t      m_desired_clusters { 2 } ;
      size_t      m_min_points { 0 } ;
      size_t      m_max_iterations { 5 } ;
      int	  m_verbosity { 0 } ;
      bool	  m_use_sparse_vectors { false } ;
      bool        m_fast_init { false } ; 	// perform faster but less precise initialization
      bool        m_hard_limit { false } ;	// don't allow more than desired # of clusters no matter what
      bool        m_ignore_extra { false } ;
      bool        m_allow_singletons { true } ;
      ClusterRep  m_representative { ClusterRep::centroid } ;
      VectorSimilarityMeasure m_similarity { VectorSimilarityMeasure::cosine } ;

   protected: // static data members
      static SignalHandler* s_sigint ;
      static std::sig_atomic_t abort_requested ;
} ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class ClusteringAlgo : public ClusteringAlgoBase
   {
   public:
      static ClusteringAlgo* instantiate(const char* algo_name, const char* options,
	 VectorMeasure<IdxT,ValT>* measure = nullptr) ;
      virtual ~ClusteringAlgo() {}

      virtual const char* algorithmName() const = 0 ;
      const char* measureName() const { return m_measure ? m_measure->canonicalName() : "(none)" ; }

      void setMeasure(VectorMeasure<IdxT,ValT>* meas)
	 {
	 if (m_measure) m_measure->free() ;
	 m_measure = meas ;
	 }

      ClusterInfo* cluster(ObjectIter& first, ObjectIter& past_end) const ;
      ClusterInfo* cluster(ArrayIter first, ArrayIter past_end) const ;
      virtual ClusterInfo* cluster(const Array* vectors) const = 0 ;

      static Vector<IdxT,ValT>* nearestNeighbor(const Vector<IdxT,ValT>* vector, const Array* centers,
	 VectorMeasure<IdxT,ValT>* measure, double threshold = -1.0) ;
   protected: //methods
      ClusteringAlgo() {}

      Vector<IdxT,ValT>* nearestNeighbor(const Vector<IdxT,ValT>* vector, const Array* centers,
	 double threshold = -1.0) const
	 { return nearestNeighbor(vector,centers,m_measure,threshold) ; }
      size_t assignToNearest(const Array* vectors, const Array* centers, ProgressIndicator *prog = nullptr,
	 double threshold = -HUGE_VAL) const ;
      double findNearestCluster(const Array* clusters, const Vector<IdxT,ValT>* vector, size_t& best_cluster,
	 ProgressIndicator* prog = nullptr) const ;
      bool separateSeeds(const Array* vectors, RefArray*& seed, RefArray*& nonseed) const ;
      bool extractClusters(const Array* vectors, ClusterInfo**& clusters, size_t& num_clusters,
	 RefArray* unassigned = nullptr) const ;

   protected: // data
      VectorMeasure<IdxT,ValT>* m_measure { nullptr } ;
   } ;

/************************************************************************/
/************************************************************************/

ListPtr enumerate_cluster_algo_names(const char* prefix = nullptr) ;
ClusteringAlgorithm parse_cluster_algo_name(const char* name) ;
ListPtr enumerate_cluster_rep_names(const char* prefix = nullptr) ;
ClusterRep parse_cluster_rep_name(const char* name) ;

} ; // end of namespace Fr

#endif /* !__FrCLUSTER_H_INCLUDED */

// end of file cluster.h //
