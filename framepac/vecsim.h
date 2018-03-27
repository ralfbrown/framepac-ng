/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-25					*/
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

#ifndef __FrVECSIM_H_INCLUDED
#define __FrVECSIM_H_INCLUDED

#include <cmath>
#include "framepac/vector.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

class VectorSimilarityOptions
   {
   public:
      VectorSimilarityOptions() ;
      bool parseOptions(const char* options) ;

   public:
      double power ;
      double alpha ;
      double beta ;
      double smoothing ;
      int    normalize ;
      bool   use_similarity ;
   } ;

//----------------------------------------------------------------------------

/*class*/
enum VectorSimilarityMeasure
   {
      none,
      anti_dice,
      benini,
      bhattacharyya,
      binary_anti_dice,
      binary_benini,
      binary_braun_blanquet,
      binary_bray_curtis,
      binary_cocogaston,
      binary_cody,
      binary_dice,
      binary_fager_mcgowan,
      binary_gamma,
      binary_gilbert,
      binary_gini,
      binary_harrison,
      binary_jaccard,
      binary_kulczynski1,
      binary_kulczynski2,
      binary_lance_williams,
      binary_lande,
      binary_lennon,
      binary_lennon2,
      binary_legendre2,
      binary_maarel,
      binary_magurran,
      binary_mcconnagh,
      binary_modgini,
      binary_mountford,
      binary_ochiai,
      binary_routledge1,
      binary_routledge2,
      binary_simpson,
      binary_sokal_sneath,
      binary_sorgenfrei,
      binary_tripartite,
      binary_whittaker,
      binary_williams,
      binary_williams2,
      binary_wilsonsnmida,
      braun_blanquet,
      bray_curtis,		// https://en.wikipedia.org/wiki/Bray%E2%80%93Curtis_dissimilarity 
      canberra,
      circle_product,
      clark,
      cocogaston,
      cody,
      cosine,
      dice,			// aka Sorenson, Sorenson-Dice, Czekanowski
      euclidean,
      fager_mcgowan,
      fidelity,
      fisher_info_metric,	//TODO
      gamma,
      gilbert,
      gini,
      harrison,
      hellinger,
      jaccard,
      jensen,
      jensen_shannon,
      kulczynski1,
      kulczynski2,
      kullback_leibler,		// https://en.wikipedia.org/wiki/Kullback%E2%80%93Leibler_divergence
      kumar_johnson,
      l0,
      lance_williams,
      lande,
      lennon,
      lennon2,
      legendre2,
      linf,
      lorentzian,
      l_norm,			// requires passing additional data to similarity function
      maarel,
      magurran,
      mahalanobis,		// requires passing additional data to similarity function
      manhattan,
      matusita,
      mcconnagh,
      modified_gini,
      morisita,			//TODO: https://en.wikipedia.org/wiki/Morisita%27s_overlap_index
      mountford,
      ochiai,
      robinson,
      routledge1,
      routledge2,
      sangvi,
      similarity_ratio,
      simpson,
      soergel,
      sokal_sneath,
      soft_cosine,		//TODO
      sorgenfrei,
      squared_chord,
      squared_euclidean,
      taneja,
      topsoe,			// twice the Jensen-Shannon distance
      tripartite,
      tversky,			//TODO: https://en.wikipedia.org/wiki/Tversky_index
      wave_hedges,
      whittaker,
      williams,
      williams2,
      wilson_shmida,
   } ;

/* Other measures covered on Wikipedia:

    https://en.wikipedia.org/wiki/Tversky_index
    Tversky: asymmetric measure:
       S(X,Y) = |intersect(X,Y)| / (|intersect(X,Y)| + alpha*|X-Y| + beta*|Y-X|
       X==prototype, Y==variant
       alpha is weight of prototype, beta is weight of variant
       alpha=beta=1 yields Tanimoto/Jaccard
       alpha=beta=0.5 yields Dice

       symmetric variant:
          a = min(|X-Y|,|Y-X|)
	  b = max(|X-Y|,|Y-X|)
	  S(X,Y) = |intersect(X,Y)| / (|intersect(X,Y)| + beta*(alpha*a + (1-alpha)b))

    https://en.wikipedia.org/wiki/Hellinger_distance 
    (aka Bhattacharyya distance)
       H(P,Q) = 1/sqrt(2) * sqrt(sum_1_k(sqrt(p_i) - sqrt(q_i))^2)
    equiv to Euclidean norm of diff of sqrt vectors:
       H(P,Q) = 1/sqrt(2) * Euclid(sqrt(P)-sqrt(Q))
    related to Bh. coefficient by
       H(P,Q) = sqrt(1 - BC(P,Q))

    https://en.wikipedia.org/wiki/Morisita%27s_overlap_index
    CsubD = 2 sum_i_S(x_i*y_i) / ((D_x + D_y)XY
       x_i = number of times species i represented in total X from one sample
       y_i = number of times species i represented in total Y from another sample
       D_x = Simpson's index: sum_1_R((p_i)^2)
       S = number of unique species
    Horn's modification:
       CsubH = 2 sum_1_S(x_i*y_i) / (((sum_1_S((x_i)^2)/X^2 + (sum_1_S((y_i)^2)/Y^2))XY)

    https://en.wikipedia.org/wiki/Bhattacharyya_distance
       DsubB(p,q) = -ln(BC(p,q))
    Bhattacharyya coefficient
       BC(p,q) = sum(sqrt(p(x)*q(x)))

    https://en.wikipedia.org/wiki/Qualitative_variation   -- various "indices"

    Soft cosine:   'https://en.wikipedia.org/wiki/Cosine_similarity#Soft_cosine_measure' 

    Monge-Elkan:   http://www.cs.cmu.edu/%7Epradeepr/papers/ijcai03.pdf 
    Ratcliff-Obershelp:   http://xlinux.nist.gov/dads/HTML/ratcliffObershelp.html 

    https://en.wikipedia.org/wiki/Matthews_correlation_coefficient

*/

//----------------------------------------------------------------------------
//  base class for all of the vector similarity and distance measures

template <typename IdxT, typename ValT>
class VectorMeasure
   {
   public:
      typedef double SimFunc(const Vector<ValT>*, const Vector<ValT>*) ;

   public:
      static const char* canonicalName() { return "Identity" ; }

      // dispatch to either the general implementation that can take any vector type, or the
      //   more efficient specialization for two dense vectors
      static double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	    if (!v1 || !v2) return -1 ;
	    if (v1->isSparseVector() || v2->isSparseVector())
	       return sparseSimilarity(reinterpret_cast<SparseVector<IdxT, ValT>>(v1),
		  reinterpret_cast<SparseVector<IdxT, ValT>>(v2)) ;
	    else
	       return denseSimilarity(v1,v2) ;
	 }
      static double distance(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	    if (!v1 || !v2) return -1 ;
	    if (v1->isSparseVector() || v2->isSparseVector())
	       return sparseDistance(reinterpret_cast<SparseVector<IdxT, ValT>>(v1),
		  reinterpret_cast<SparseVector<IdxT, ValT>>(v2)) ;
	    else
	       return denseDistance(v1,v2) ;
	 }

      // base version is just an identity comparison
      static double sparseSimilarity(const SparseVector<IdxT, ValT>* v1, const SparseVector<IdxT, ValT>* v2)
	 { return v1 == v2 ? 1 : 0 ; }
      static double sparseDistance(const SparseVector<IdxT, ValT>* v1, const SparseVector<IdxT, ValT>* v2)
	 { return v1 != v2 ? 1 : 0 ; }

      // by default, we don't have a specialization for dense vectors
      static double denseSimilarity(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	 return sparseSimilarity(v1,v2) ;
	 }
      static double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	 return sparseDistance(v1,v2) ;
	 }
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a similarity metric and computes the distance as 1-sim

template <typename IdxT, typename ValT>
class SimilarityMeasure : public VectorMeasure<IdxT, ValT>
   {
   public:
      static double distance(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a similarity metric and computes the distance as 1/sim

template <typename IdxT, typename ValT>
class SimilarityMeasureReciprocal : public VectorMeasure<IdxT, ValT>
   {
   public:
      static double distance(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	    double sim = similarity(v1,v2) ;
	    return sim ? 1.0 / sim : HUGE_VAL ;
	 }
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a distance metric and computes the similarity as 1-dist

template <typename IdxT, typename ValT>
class DistanceMeasure : public VectorMeasure<IdxT, ValT>
   {
   public:
      static double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }

   } ;

//----------------------------------------------------------------------------
// a vector measure which has a distance metric and computes the similarity as 1/dist

template <typename IdxT, typename ValT>
class DistanceMeasureReciprocal : public VectorMeasure<IdxT, ValT>
   {
   public:
      static double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2)
	 {
	    double dist = distance(v1,v2) ;
	    return dist ? 1.0 / dist : HUGE_VAL ;
	 }

   } ;

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
class VectorSimilarity : public VectorSimilarityOptions
   {
   public:
      typedef typename VectorMeasure<IdxT, ValT>::SimFunc SimFunc ;
      
   public:
      VectorSimilarity(const char *simtype) ;
      VectorSimilarity(const VectorSimilarity&) = default ;
      ~VectorSimilarity() {}
      VectorSimilarity& operator= (const VectorSimilarity&) = default ;

      bool parseConfig(const char* options) ;

      double score(const Object *v1, const Object *v2) const ;

      double similarity(Vector<ValT>* v1,Vector<ValT>* v2) const { return m_sim(v1,v2) ; }
      double distance(Vector<ValT>* v1,Vector<ValT>* v2) const { return m_dist(v1,v2) ; }

   protected:
      VectorSimilarityMeasure	m_measure ;
      SimFunc*			m_sim ;
      SimFunc*			m_dist ;
   } ;


/*
    Other algos to check:

	https://en.wikipedia.org/wiki/Fisher_information_metric

        string alignment:
        https://en.wikipedia.org/wiki/Smith%E2%80%93Waterman_algorithm

        clustering:
	https://en.wikipedia.org/wiki/Ward%27s_method#Lance.E2.80.93Williams_algorithms

*/

//----------------------------------------------------------------------------

} ; // end of namespace Fr

#endif /* !__FrVECSIM_H_INCLUDED */

// end of file vecsim.h //
