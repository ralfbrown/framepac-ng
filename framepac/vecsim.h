/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-03-28					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2016,2017 Carnegie Mellon University			*/
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

#include "framepac/vector.h"

namespace Fr {

/************************************************************************/
/************************************************************************/

class VectorSimilarityOptions
   {
   public:
      double power ;
      double alpha ;
      double beta ;
      int    normalize ;
      bool   use_similarity ;
   public:
      bool parseOptions(const char *options) ;

   } ;

//----------------------------------------------------------------------------

/*class*/
enum VectorSimilarityMeasure
   {
      none,
	 anti_dice,
	 benini,
	 braun_blanquet,
	 bray_curtis,		// https://en.wikipedia.org/wiki/Bray%E2%80%93Curtis_dissimilarity 
	 canberra,
	 circle_product,
	 cocogaston,
	 cody,
	 cosine,
	 dice,			// aka Sorenson, Sorenson-Dice, Czekanowski
	 euclidean,
	 fager_mcgowan,
	 gamma,
	 gilbert,
	 gini,
	 harrison,
	 jaccard,
	 jensen_shannon,
	 kulczynski1,
	 kulczynski2,
	 kullback_leibler,	// https://en.wikipedia.org/wiki/Kullback%E2%80%93Leibler_divergence
	 l0,
	 lance_williams,
	 lande,
	 lennon,
	 lennon2,
	 legendre2,
	 linf,
	 l_norm,		// requires passing additional data to similarity function
	 maarel,
	 magurran,
	 mahalanobis,		// requires passing additional data to similarity function
	 manhattan,
	 mcconnagh,
	 modified_gini,
	 mountford,
	 ochiai,
	 robinson,
	 routledge1,
	 routledge2,
	 similarity_ratio,
	 simpson,
	 sokal_sneath,
	 sorgenfrei,
	 squared_euclidean,
	 tripartite,
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

template <typename IdxT, typename ValT>
class VectorSimilarity : public VectorSimilarityOptions
   {
   public:
      VectorSimilarity(const char *simtype) ;
      VectorSimilarity(const VectorSimilarity&) = default ;
      ~VectorSimilarity() {}
      VectorSimilarity& operator= (const VectorSimilarity&) = default ;

      bool parseConfig(const char* options) ;

      double score(const Object *v1, const Object *v2) const ;

      double similarity(Vector<ValT>* v1,Vector<ValT>* v2) const { return m_sim_d_d(v1,v2) ; }
      double similarity(Vector<ValT>* v1,SparseVector<IdxT,ValT>* v2) const { return m_sim_d_s(v1,v2) ; }
      double similarity(SparseVector<IdxT,ValT>* v1,Vector<ValT>* v2) const { return m_sim_s_d(v1,v2) ; }
      double similarity(SparseVector<IdxT,ValT>* v1,SparseVector<IdxT,ValT>* v2) const { return m_sim_s_s(v1,v2) ; }

      double distance(Vector<ValT>* v1,Vector<ValT>* v2) const { return m_dist_d_d(v1,v2) ; }
      double distance(Vector<ValT>* v1,SparseVector<IdxT,ValT>* v2) const { return m_dist_d_s(v1,v2) ; }
      double distance(SparseVector<IdxT,ValT>* v1,Vector<ValT>* v2) const { return m_dist_s_d(v1,v2) ; }
      double distance(SparseVector<IdxT,ValT>* v1,SparseVector<IdxT,ValT>* v2) const { return m_dist_s_s(v1,v2) ; }

   protected:
      typedef double Sim_Dense_Dense_Fn(const Vector<ValT>*, const Vector<ValT>*) ;
      typedef double Sim_Dense_Sparse_Fn(const Vector<ValT>*, const SparseVector<IdxT,ValT>*) ;
      typedef double Sim_Sparse_Dense_Fn(const SparseVector<IdxT,ValT>*, const Vector<ValT>*) ;
      typedef double Sim_Sparse_Sparse_Fn(const SparseVector<IdxT,ValT>*, const SparseVector<IdxT,ValT>*) ;

      struct FuncPtrs {
	 VectorSimilarityMeasure measure ;
	 Sim_Dense_Dense_Fn*     sim_d_d ;
	 Sim_Dense_Sparse_Fn*    sim_d_s ;
	 Sim_Sparse_Dense_Fn*    sim_s_d ;
	 Sim_Sparse_Sparse_Fn*   sim_s_s ;
	 Sim_Dense_Dense_Fn*     dist_d_d ;
	 Sim_Dense_Sparse_Fn*    dist_d_s ;
	 Sim_Sparse_Dense_Fn*    dist_s_d ;
	 Sim_Sparse_Sparse_Fn*   dist_s_s ;
         } ;

      static const FuncPtrs *m_funcptrs ;

      Sim_Dense_Dense_Fn*   m_sim_d_d ;
      Sim_Dense_Sparse_Fn*  m_sim_d_s ;
      Sim_Sparse_Dense_Fn*  m_sim_s_d ;
      Sim_Sparse_Sparse_Fn* m_sim_s_s ;

      Sim_Dense_Dense_Fn*   m_dist_d_d ;
      Sim_Dense_Sparse_Fn*  m_dist_d_s ;
      Sim_Sparse_Dense_Fn*  m_dist_s_d ;
      Sim_Sparse_Sparse_Fn* m_dist_s_s ;
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
