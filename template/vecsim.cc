/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-09-18					*/
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

#include <cmath>
#include <float.h>
#include "framepac/vecsim.h"

/************************************************************************/
/************************************************************************/

namespace FramepaC
{
namespace vecsim
{

using Fr::VectorSimilarityOptions ;
using Fr::DenseVector ;

template <typename ValT>
ValT p_log_p(ValT p)
{
   return p ? p * std::log(p) : 0 ;
}

//----------------------------------------------------------------------------

template <typename VecT>
typename VecT::value_type sum_of_weights(const VecT* v)
{
   typename VecT::value_type sum(0.0) ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      sum += std::abs(v->elementValue(i)) ;
   // since we use the returned value as a divisor, force it to be nonzero
   // a zero sum means we'll be dividing only zeros by the return
   //   value, so we can pick anything without changing the results; a
   //   value of one is convenient
   return sum ? sum : 1 ;
}

//----------------------------------------------------------------------------

template <typename VecT>
typename VecT::value_type max_weight(const VecT* v)
{
   typename VecT::value_type max(0.0) ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      {
      typename VecT::value_type val = std::abs(v->elementValue(i)) ;
      if (val > max)
	 max = val ;
      }
   // since we use the returned value as a divisor, force it to be nonzero
   // a zero sum means we'll be dividing only zeros by the return
   //   value, so we can pick anything without changing the results; a
   //   value of one is convenient
   return max ? max : 1 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
void normalization_weights(const VecT1* v1, const VecT2* v2, int normalization,
			   typename VecT1::value_type &wt1, typename VecT1::value_type &wt2)
{
   wt1 = wt2 = 1.0 ;
   if (normalization == 1)
      {
      // L1-normalization: divide by sum of element values
      wt1 = sum_of_weights(v1) ;
      wt2 = sum_of_weights(v2) ;
      }
   else if (normalization == 2)
      {
      // L2-normalization: divide by vector's Euclidean length
      wt1 = v1->length() ;
      wt2 = v2->length() ;
      if (!wt1) wt1 = 1.0 ;
      if (!wt2) wt2 = 1.0 ;
      }
   else if (normalization == 3)
      {
      // Linf-normalization: divide by maximum element value
      wt1 = max_weight(v1) ;
      wt2 = max_weight(v2) ;
      }
   return ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
void contingency_table(const VecT1* v1, const VecT2* v2,
		       const VectorSimilarityOptions& opt,
		       typename VecT1::value_type& a,
		       typename VecT1::value_type& b,
		       typename VecT1::value_type& c)
{
   a = b = c = 0 ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1 = v1->elementIndex(pos1) ;
      auto elt2 = v2->elementIndex(pos2) ;
      if (elt1 < elt2)
	 {
	 b += v1->elementValue(pos1++) ;
	 }
      else if (elt1 > elt2)
	 {
	 c += v2->elementValue(pos2++) ;
	 }
      else // if (elt1 == elt2)
	 {
	 typename VecT1::value_type val1(v1->elementValue(pos1++)) ;
	 typename VecT1::value_type val2(v2->elementValue(pos2++)) ;
	 typename VecT1::value_type com(std::min(val1/wt1,val2/wt2)) ;
	 a += com ;
	 b += (val1 - com) ;
	 c += (val2 - com) ;
	 }
      }
   // handle any leftovers from the first vector (if the second is shorter or zero)
   for ( ; pos1 < elts1 ; )
      {
      b += v1->elementValue(pos1++) ;
      }
   // handle any leftovers from the second vector (if the first is shorter or zero)
   for ( ; pos2 < elts2 ; )
      {
      c += v2->elementValue(pos2++) ;
      }
   b /= wt1 ;
   c /= wt2 ;
   return;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
void contingency_table(const DenseVector<ValT>* v1, const DenseVector<ValT>* v2,
		       const VectorSimilarityOptions& opt,
		       ValT &a, ValT &b, ValT &c)
{
   a = b = c = 0 ;
   ValT wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   // figure out how many elements to process in both vectors
   size_t minlen = std::min(v1->numElements(),v2->numElements()) ;
   // if either vector is all zeros, we don't need to treat it in
   //   common with the other one
   if (wt1 == 0.0 || wt2 == 0.0) minlen = 0 ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT val1(v1->elementValue(i)) ;
      ValT val2(v2->elementValue(i)) ;
      ValT com(std::min(val1/wt1,val2/wt2)) ;
      a += com ;
      b += (val1 - com) ;
      c += (val2 - com) ;
      }
   // handle any leftovers from the first vector (if the second is shorter or zero)
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      b += v1->elementValue(i) ;
      }
   // handle any leftovers from the second vector (if the first is shorter or zero)
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      c += v2->elementValue(i) ;
      }
   b /= wt1 ;
   c /= wt2 ;
   return;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
void binary_contingency_table(const VecT1* v1, const VecT2* v2,
			      size_t& both, size_t& v1_only, size_t& v2_only, size_t& neither)
{
   both = v1_only = v2_only = neither = 0 ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1 = v1->elementIndex(pos1) ;
      auto elt2 = v2->elementIndex(pos2) ;
      if (elt1 < elt2)
	 {
	 if (v1->elementValue(pos1++))
	    v1_only++ ;
	 else
	    neither++ ;
	 }
      else if (elt1 > elt2)
	 {
	 if (v2->elementValue(pos2++))
	    v2_only++ ;
	 else
	    neither++ ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) ;
	 auto val2 = v2->elementValue(pos2++) ;
	 if (val1 && val2)
	    both++ ;
	 else if (val1)
	    v1_only++ ;
	 else if (val2)
	    v2_only++ ;
	 else
	    neither++ ;
	 }
      }
   // handle any leftovers from the first vector (if the second is shorter)
   for ( ; pos1 < elts1 ; )
      {
      if (v1->elementValue(pos1++))
	 v1_only++ ;
      else
	 neither++ ;
      }
   // handle any leftovers from the second vector (if the first is shorter)
   for ( ; pos2 < elts2 ; )
      {
      if (v2->elementValue(pos2++))
	 v2_only++ ;
      else
	 neither++ ;
      }
   return;
}

//----------------------------------------------------------------------------
// a more efficient specialization for matching two dense vectors

template <typename ValT>
void binary_contingency_table(const DenseVector<ValT>* v1, const DenseVector<ValT>* v2,
			      size_t& both, size_t& v1_only, size_t& v2_only, size_t& neither)
{
   both = v1_only = v2_only = neither = 0 ;
   // figure out how many elements to process in both vectors
   size_t minlen = std::min(v1->numElements(),v2->numElements()) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT val1 = v1->elementValue(i) ;
      ValT val2 = v2->elementValue(i) ;
      if (val1 && val2)
	 both++ ;
      else if (val1)
	 v1_only++ ;
      else if (val2)
	 v2_only++ ;
      else
	 neither++ ;
      }
   // handle any leftovers from the first vector (if the second is shorter)
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      if (v1->elementValue(i))
	 v1_only++ ;
      else
	 neither++ ;
      }
   // handle any leftovers from the second vector (if the first is shorter)
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      if (v2->elementValue(i))
	 v2_only++ ;
      else
	 neither++ ;
      }
   return;
}

//----------------------------------------------------------------------------
//  return a as 'both', (b+c) as 'disagree', and d as 'neither'

template <typename VecT1, typename VecT2>
void binary_agreement(const VecT1* v1, const VecT2* v2, size_t& both, size_t& disagree, size_t& neither)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   size_t stats[3] { 0, 0, 0 } ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1 = v1->elementIndex(pos1) ;
      auto elt2 = v2->elementIndex(pos2) ;
      if (elt1 < elt2)
	 {
	 bool val(v1->elementValue(pos1++)) ;
	 stats[val]++ ;
	 }
      else if (elt1 > elt2)
	 {
	 bool val(v2->elementValue(pos2++)) ;
	 stats[val]++ ;
	 }
      else // if (elt1 == elt2)
	 {
	 bool val1(v1->elementValue(pos1++)) ;
	 bool val2(v2->elementValue(pos2++)) ;
	 stats[val1+val2]++ ;
	 }
      }
   // handle any leftovers from the first vector (if the second is shorter)
   for ( ; pos1 < elts1 ; )
      {
      bool val(v1->elementValue(pos1++)) ;
      stats[val]++ ;
      }
   // handle any leftovers from the second vector (if the first is shorter)
   for ( ; pos2 < elts2 ; )
      {
      bool val(v2->elementValue(pos2++)) ;
      stats[val]++ ;
      }
   neither = stats[0] ;
   disagree = stats[1] ;
   both = stats[2] ;
   return;
}

//----------------------------------------------------------------------------
// a more efficient specialization for matching two dense vectors

template <typename ValT>
void binary_agreement(const DenseVector<ValT>* v1, const DenseVector<ValT>* v2,
		      size_t& both, size_t& disagree, size_t& neither)
{
   size_t stats[3] { 0, 0, 0 } ;
   // figure out how many elements to process in both vectors
   size_t minlen = std::min(v1->numElements(),v2->numElements()) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      bool val1(v1->elementValue(i)) ;
      bool val2(v2->elementValue(i)) ;
      stats[val1+val2]++ ;
      }
   // handle any leftovers from the first vector (if the second is shorter)
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      bool val(v1->elementValue(i)) ;
      stats[val]++ ;
      }
   // handle any leftovers from the second vector (if the first is shorter)
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      bool val(v2->elementValue(i)) ;
      stats[val]++ ;
      }
   neither = stats[0] ;
   disagree = stats[1] ;
   both = stats[2] ;
   return;
}

//----------------------------------------------------------------------------

//============================================================================

template <typename VecT1, typename VecT2>
double cosine_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   double prod_lengths((v1->length() * v2->length())) ;
   if (!prod_lengths)
      return 0.0 ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   typename VecT1::value_type dotprod(0) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 ++pos1 ;
	 }
      else if (elt1 > elt2)
	 {
	 ++pos2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 dotprod += (v1->elementValue(pos1++) * v2->elementValue(pos2++)) ;
	 }
      }
   return dotprod / prod_lengths ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double cosine_sim(const DenseVector<ValT>* v1, const DenseVector<ValT>* v2,
		  const VectorSimilarityOptions&)
{
   double prod_lengths((v1->length() * v2->length())) ;
   if (!prod_lengths)
      return 0.0 ;
   size_t len(std::min(v1->size(),v2->size())) ;
   ValT dotprod(0) ;
   for (size_t i = 0 ; i < len ; ++i)
      {
      dotprod += (v1->elementValue(i) * v2->elementValue(i)) ;
      }
   return dotprod / prod_lengths ;
}

//============================================================================
//   Measures based on a 2x2 contingency table			            //
//============================================================================

//============================================================================

template <typename VecT1, typename VecT2>
double antidice_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type denom(both + 2.0*(v1_only + v2_only)) ;
   return (denom > 0) ? both / (double)denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double antidice_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - antidice_sim(v1,v2,opt) ;
}

//============================================================================
// measure first published in
//      Sokal, R.R. and Sneath, P.H.A. (1963) Principles of Numerical
//      Taxononmy.  San Franciso: Freeman.  pp.129.
// and credited to
//      Anderberg, M.R. (1973) Cluster Analysis for Applications.  New
//      York: Academic Press.
//  a / (a + 2*(b+c))
// Rogers&Tanimoto(1960) used (a+d)/((a+d)+2(b+c))

template <typename VecT1, typename VecT2>
double binary_antidice_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   double denom(both + 2.0 * only_1) ;
   return (denom > 0.0) ? both / denom : 1.0 ; // (all-zero vectors are defined to be identical)
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_antidice_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_antidice_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
// measure first published in
//      Benini, R. (1901) "Principii di demografie".  No. 29 of
//      manuali Barbera di science giuridiche sociali e poltiche.
//      Firenzi: G. Barbera

template <typename VecT1, typename VecT2>
double benini_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type N(both + v1_only + v2_only) ;
   both /= N ;
   v1_only /= N ;
   v2_only /= N ;
   typename VecT1::value_type prod((both+v1_only)*(both+v2_only)) ;
   return (both - prod) / (both + std::min(v1_only,v2_only) - prod) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double benini_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - benini_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
// measure first published in
//      Benini, R. (1901) "Principii di demografie".  No. 29 of
//      manuali Barbera di science giuridiche sociali e poltiche.
//      Firenzi: G. Barbera

template <typename VecT1, typename VecT2>
double binary_benini_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double N(both + v1_only + v2_only + neither) ;
   double total1((both+v1_only)/N) ;
   double total2((both+v2_only)/N) ;
   double prod(total1*total2) ;
   double match(both/N) ;
   return (match - prod) / (match + std::min(v1_only,v2_only)/N - prod) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_benini_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_benini_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double braun_blanquet_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type larger(std::max(both+v1_only,both+v2_only)) ;
   return larger > 0 ? both / larger : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double braun_blanquet_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - braun_blanquet_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_braun_blanquet_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t both, v1_only, v2_only, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double larger(std::max(both+v1_only,both+v2_only)) ;
   return larger > 0 ? both / larger : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_braun_blanquet_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_braun_blanquet_sim(v1,v2,opt) ;
}

//============================================================================
//  original publication:
//     Bray, J.R. and Curtis, J.T. (1957) "An ordination of upland
//     forest communities of southern Wisconsin". Ecological
//     Monographs 27:325-349.
// aka Czekanowski's quantitative index: sum_i(min(x_i,y_i))/sum_i(x_i+y_i)

template <typename VecT1, typename VecT2>
double bray_curtis_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type sum(2.0 * both + v1_only + v2_only) ;
   return sum ? (v1_only + v2_only) / sum : 1.0 ;
}

//----------------------------------------------------------------------------
// aka Czekanowski's quantitative index: sum_i(min(x_i,y_i))/sum_i(x_i+y_i)

template <typename VecT1, typename VecT2>
double bray_curtis_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - bray_curtis_dis(v1,v2,opt) ;
}

//============================================================================
//  original publication:
//     Bray, J.R. and Curtis, J.T. (1957) "An ordination of upland
//     forest communities of southern Wisconsin". Ecological
//     Monographs 27:325-349.
// (b+c)/(2a + b + c)

template <typename VecT1, typename VecT2>
double binary_bray_curtis_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   size_t sum(2.0 * both + only_1) ;
   return sum ? only_1 / sum : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_bray_curtis_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_bray_curtis_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Colwell&Coddington (1948) and Gaston (2001)
// (b+c)/(2a + b + c)

template <typename VecT1, typename VecT2>
double cocogaston_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type common, v1_only, v2_only ;
   contingency_table(v1,v2,opt,common,v1_only,v2_only) ;
   typename VecT1::value_type dif(v1_only + v2_only) ;
   if (common + dif == 0)
      return 1.0 ;			// all-zero vectors are defined to be identical
   return dif / (2.0 * common + dif) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double cocogaston_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - cocogaston_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Colwell&Coddington (1948) and Gaston (2001)
// (b+c)/(2a + b + c)

template <typename VecT1, typename VecT2>
double binary_cocogaston_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   if (both + only_1 == 0)
      return 1.0 ; // all-zero vectors are defined to be identical
   return only_1 / (2.0 * both + only_1) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_cocogaston_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_cocogaston_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Cody (1993)

template <typename VecT1, typename VecT2>
double cody_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type total1(both + v1_only) ;
   typename VecT1::value_type total2(both + v2_only) ;
   if (total1 * total2 <= 0)
      return 1.0 ;			// all-zero vectors are defined to be identical
   return both * (total1 + total2) / (2.0 * total1 * total2) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Cody (1993)

template <typename VecT1, typename VecT2>
double cody_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - cody_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Cody (1993)

template <typename VecT1, typename VecT2>
double binary_cody_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double total1(both + v1_only) ;
   double total2(both + v2_only) ;
   double denom(2.0 * total1 * total2) ;
   if (!denom)
      return 1.0 ; // all-zero vectors are defined to be identical
   return both * (total1 + total2) / denom ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Cody (1993)

template <typename VecT1, typename VecT2>
double binary_cody_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_cody_sim(v1,v2,opt) ;
}

//============================================================================
//  2a / (2a + b + c)
// aka Czekanowski
// = 1 - Harte dissim,
//      Harte, J. & Kinzig, A. (1997) "On the implications of
//      species-area relationships for endemism, spatial turnover and
//      food web patterns". Oikos 80.

template <typename VecT1, typename VecT2>
double dice_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   if (both + v1_only + v2_only <= 0.0)
      return 1.0 ;			// all-zero vectors are defined to be identical
   return 2.0 * both / (2.0 * both + v1_only + v2_only) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double dice_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - dice_sim(v1,v2,opt) ;
}

//============================================================================
//  measure published in
//     Czekanowski, J. (1932) '"Coefficient of racial likeness" und
//     "durchschnittliche Differenz"'.  Anthropologischer Anzeiger
//     9:227-249.
//  and
//     Dice, L.R. (1945) "Measures of the amount of ecologic
//     association between species".  Ecology 26:297-302.
// 2a / (2a + b + c)

template <typename VecT1, typename VecT2>
double binary_dice_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   both *= 2 ;
   double denom(both + only_1) ;
   return denom ? both / denom : 1.0 ;// all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_dice_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - bnary_dice_sim(v1,v2,opt) ;
}

//============================================================================
//  a / (sqrt(a+b)(a+c)) - 0.5*max(b,c)

template <typename VecT1, typename VecT2>
double fager_mcgowan_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type prod((both+v1_only)*(both+v2_only)) ;
   double denom(std::sqrt(prod) - std::max(v1_only,v2_only)/2) ;
   return denom ? both / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double fager_mcgowan_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - fager_mcgowan_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_fager_mcgowan_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t both, v1_only, v2_only, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double prod((both+v1_only)*(both+v2_only)) ;
   double denom(std::sqrt(prod) - std::max(v1_only,v2_only)/2) ;
   return denom ? both / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_fager_mcgowan_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_fager_mcgowan_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double gamma_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type common, v1_only, v2_only ;
   contingency_table(v1,v2,opt,common,v1_only,v2_only) ;
   typename VecT1::value_type N(common + v1_only + v2_only) ;
   typename VecT1::value_type neither(0) ; //FIXME
   double concordance(common / N * neither / N) ;
   double discordance(v1_only / N * v2_only / N) ;
   return (concordance + discordance > 0) ? (concordance - discordance) / (concordance + discordance) : 1.0 ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double gamma_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - gamma_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_gamma_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   if (!elts1 && !elts2)
      return 1.0 ;			// two zero-length vectors are identical
   else if (!elts1 || !elts2)
      return -1.0 ;			// maximal difference if only one vector zero-length
   size_t both, v1_only, v2_only, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double N(both + v1_only + v2_only + neither) ;
   double concordance(both / N * neither / N) ;
   double discordance(v1_only / N * v2_only / N) ;
   return (concordance + discordance > 0) ? (concordance - discordance) / (concordance + discordance) : 1.0 ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_gamma_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_gamma_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double gilbert_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type N(both + v1_only + v2_only) ;
   both /= N ;
   v1_only /= N ;
   v2_only /= N ;
   typename VecT1::value_type prod((both+v1_only)*(both+v2_only)) ;
   return (both - prod) / (both + v1_only + v2_only - prod) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double gilbert_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - gilbert_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double binary_gilbert_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double N(both + v1_only + v2_only + neither) ;
   double total1((both+v1_only)/N) ;
   double total2((both+v2_only)/N) ;
   double prod(total1*total2) ;
   double match(both/N) ;
   return (match - prod) / (total1 + total2 - match - prod) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double binary_gilbert_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_gilbert_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double gini_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type N(both + v1_only + v2_only) ;
   both /= N ;
   v1_only /= N ;
   v2_only /= N ;
   typename VecT1::value_type total1(both+v1_only) ;
   typename VecT1::value_type total2(both+v2_only) ;
   return (both - total1*total2) / std::sqrt((1.0 - total1*total1)*(1.0 - total2*total2)) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double gini_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - gini_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double binary_gini_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double N(both + v1_only + v2_only + neither) ;
   double total1((both+v1_only)/N) ;
   double total2((both+v2_only)/N) ;
   double prod(total1*total2) ;
   return (both/N - prod) / std::sqrt((1.0 - total1*total1)*(1.0 - (total2*total2))) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double binary_gini_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_gini_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Harrison (1988)
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double harrison_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double denom(both + std::max(v1_only,v2_only)) ;
   // all-zero vectors are defined to be identical
   return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
}

//============================================================================
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double harrison_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - harrison_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Harrison (1988)
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_harrison_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double denom(both + std::max(v1_only,v2_only)) ;
   // all-zero vectors are defined to be identical
   return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
}

//============================================================================
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_harrison_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_harrison_dis(v1,v2,opt) ;
}

//============================================================================
//  first published in 
//      Jaccard, P. (1912) "The distribution of flora of the alpine
//      zone". New Phytologist 11:37-50.
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double jaccard_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double either(both + v1_only + v2_only) ;
   return either ? both / either : 1.0 ; // all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double jaccard_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - jaccard_sim(v1,v2,opt) ;
}

//============================================================================
//  first published in 
//      Jaccard, P. (1912) "The distribution of flora of the alpine
//      zone". New Phytologist 11:37-50.
//  range: 0 ... 1
//  a / (a + b + c)

template <typename VecT1, typename VecT2>
double binary_jaccard_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   double either(both + only_1) ;
   return either ? both / either : 1.0 ; // all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_jaccard_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_jaccard_sim(v1,v2,opt) ;
}

//============================================================================
//  a/(b+c)
// "simba" package calls (a+d)/(b+c) sokal4 from Sokal&Sneath(1963)
// same as Tanimoto from old FramepaC

template <typename VecT1, typename VecT2>
double kulczynski1_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double only_1(v1_only+v2_only) ;
   return only_1 ? both / only_1 : HUGE_VAL ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double kulczynski1_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 / (1.0 + kulczynski1_sim(v1,v2,opt)) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_kulczynski1_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   // both += neither ;   // "simba"s sokal4
   return only_1 ? both / only_1 : HUGE_VAL ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_kulczynski1_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 / (1.0 + binary_kulczynski1_sim(v1,v2,opt)) ;
}

//============================================================================
// measure first published in
//    Kulczynski, S. (1927) "Die Planzenassoziationen der Pieninen",
//    Bulletin International de l'Academie Poloaise des Sciences et
//    des Letters, Classe des Sciences Mathematiques et Naturells, B
//    (Sciences Naturelles), Suppl. II: 57-203.  Polish article with
//    German summary.
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double kulczynski2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   if (both + v1_only + v2_only == 0)
      return 1.0 ; // both all-zero vectors are defined to be identical
   double total1(both + v1_only) ;
   double total2(both + v2_only) ;
   if (total1 == 0 || total2 == 0)
      return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
   return (both / total1 + both / total2) / 2.0 ;
}

//============================================================================
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double kulczynski2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - kulczynski2_sim(v1,v2,opt) ;
}

//============================================================================
// measure first published in
//    Kulczynski, S. (1927) "Die Planzenassoziationen der Pieninen",
//    Bulletin International de l'Academie Poloaise des Sciences et
//    des Letters, Classe des Sciences Mathematiques et Naturells, B
//    (Sciences Naturelles), Suppl. II: 57-203.  Polish article with
//    German summary.
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_kulczynski2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   if (both + v1_only + v2_only == 0)
      return 1.0 ; // both all-zero vectors are defined to be identical
   double total1(both + v1_only) ;
   double total2(both + v2_only) ;
   if (total1 == 0 || total2 == 0)
      return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
   return (both / total1 + both / total2) / 2.0 ;
}

//============================================================================
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_kulczynski2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_kulczynski2_sim(v1,v2,opt) ;
}

//============================================================================
//  (b+c) / (2a + b + c)

template <typename VecT1, typename VecT2>
double lance_williams_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double denom(2.0*both + v1_only + v2_only) ;
   return denom ? (v1_only + v2_only) / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double lance_williams_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - lance_williams_sim(v1,v2,opt) ;
}

//============================================================================
// (b+c) / (2a + b + c)

template <typename VecT1, typename VecT2>
double binary_lance_williams_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   double denom(2.0*both + only_1) ;
   return denom ? only_1 / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_lance_williams_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_lance_williams_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lande (1996)
// (b+c)/2

template <typename VecT1, typename VecT2>
double lande_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   return (v1_only + v2_only) / 2.0 ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double lande_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - lande_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lande (1996)
// (b+c)/2

template <typename VecT1, typename VecT2>
double binary_lande_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   return only_1 / 2.0 ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_lande_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_lande_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 2
//  lennon = 2|b-c| / (2a + b + c)

template <typename VecT1, typename VecT2>
double lennon_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type total1(both + v1_only) ;
   typename VecT1::value_type total2(both + v2_only) ;
   double denom(total1 + total2) ;
   // all-zero vectors are defined to be identical
   return denom ? (2.0 * std::abs(v1_only - v2_only) / denom) : 0.0 ;
}

//============================================================================
//  range: -1 ... 1

template <typename VecT1, typename VecT2>
double lennon_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - lennon_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 2

template <typename VecT1, typename VecT2>
double binary_lennon_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double total1(both + v1_only) ;
   double total2(both + v2_only) ;
   double denom(total1 + total2) ;
   // all-zero vectors are defined to be identical
   return denom ? (2.0 * std::abs((ssize_t)(v1_only - v2_only)) / denom) : 0.0 ;
}

//============================================================================
//  range: -1 ... 1

template <typename VecT1, typename VecT2>
double binary_lennon_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_lennon_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double lennon2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double sum(both + v1_only + v2_only) ;
   return sum ? std::log2((both+sum)/sum) : 1.0 ;
}

//----------------------------------------------------------------------------
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double lennon2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - lennon2_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 1
// log2((2a+b+c)/(a+b+c))

template <typename VecT1, typename VecT2>
double binary_lennon2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   double sum(both + only_1) ;
   return sum ? std::log2((both+sum)/sum) : 1.0 ;
}

//----------------------------------------------------------------------------
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_lennon2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_lennon2_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Legendre & Legendre (1998)
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double legendre2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   both *= 3 ;
   double denom(both + v1_only + v2_only) ;
   return denom ? both / denom : 1.0 ;
}

//----------------------------------------------------------------------------
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double legendre2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - legendre2_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Legendre & Legendre (1998)
//  range: 0 ... 1
// leg2 = 3a / (3a + b + c)

template <typename VecT1, typename VecT2>
double binary_legendre2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   both *= 3 ;
   double denom(both + only_1) ;
   return denom ? both / denom : 1.0 ;
}

//----------------------------------------------------------------------------
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_legendre2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_legendre2_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     van der Maarel (1969)
// range: -1 ... 1
// maarel = (2a - b - c) / (2a + b + c)

template <typename VecT1, typename VecT2>
double maarel_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   both *= 2 ;
   double denom(both + v1_only + v2_only) ;
   return denom ? ((both - v1_only - v2_only) / denom) : 1.0 ;
}

//============================================================================
// range: -1 ... 1

template <typename VecT1, typename VecT2>
double maarel_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - maarel_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     van der Maarel (1969)
// range: -1 ... 1
// maarel = (2a - b - c) / (2a + b + c)

template <typename VecT1, typename VecT2>
double binary_maarel_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   both *= 2 ;
   double denom(both + only_1) ;
   return denom ? ((both - only_1) / denom) : 1.0 ;
}

//============================================================================
// range: -1 ... 1

template <typename VecT1, typename VecT2>
double binary_maarel_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_maarel_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Magurran (1988)
// mag = (2a + b + c) * (1 - (a / (a + b + c)))
//     = N * (1 - Jaccard)

template <typename VecT1, typename VecT2>
double magurran_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double sum(both + v1_only + v2_only) ;
   // all-zero vectors are defined to be identical
   return sum ? ((both + sum) * (1.0 - (both / sum))) : 1.0 ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double magurran_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - magurran_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Magurran (1988)

template <typename VecT1, typename VecT2>
double binary_magurran_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   double sum(both + only_1) ;
   // all-zero vectors are defined to be identical
   return sum ? ((both + sum) * (1.0 - (both / sum))) : 1.0 ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_magurran_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_magurran_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Hubalek (1982)
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double mcconnagh_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type total1(both + v1_only) ;
   typename VecT1::value_type total2(both + v2_only) ;
   double denom(total1 * total2) ;
   return denom ? (both * both - v1_only * v2_only) / denom : 1.0 ;
}

//============================================================================
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double mcconnagh_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - mcconagh_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Hubalek (1982)
// range: 0 ... 1
// mcc = (aa - bc) / ((a+b)(a+c))
// aka McConnaughey

template <typename VecT1, typename VecT2>
double binary_mcconnagh_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double total1(both + v1_only) ;
   double total2(both + v2_only) ;
   double denom(total1 * total2) ;
   return denom ? (both * both - v1_only * v2_only) / denom : 1.0 ;
}

//============================================================================
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_mcconnagh_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_mcconagh_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double modgini_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type N(both + v1_only + v2_only) ;
   both /= N ;
   v1_only /= N ;
   v2_only /= N ;
   typename VecT1::value_type total1(both+v1_only) ;
   typename VecT1::value_type total2(both+v2_only) ;
   return (both - total1*total2) / (1.0 - std::abs(v1_only-v2_only)/2 - total1*total2) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double modgini_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - modgini_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double binary_modgini_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double N(both + v1_only + v2_only + neither) ;
   double prod((both+v1_only)/N*(both+v2_only)/N) ;
   return (both/N - prod) / (1.0 - std::abs((ssize_t)(v1_only-v2_only))/(2.0*N) - prod) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_modgini_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_modgini_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package

template <typename VecT1, typename VecT2>
double mountford_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   if (v1_only + v2_only == 0)
      return 1.0 ; // vectors with no disagreements are defined to be identical
   return 2.0 * both / (double)(both*(v1_only + v2_only) + 2.0 * v1_only * v2_only) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double mountford_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - mountford_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package

template <typename VecT1, typename VecT2>
double binary_mountford_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   if (v1_only + v2_only == 0)
      return 1.0 ; // vectors with no disagreements are defined to be identical
   return 2.0 * both / (double)(both*(v1_only + v2_only) + 2.0 * v1_only * v2_only) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_mountford_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_mountford_sim(v1,v2,opt) ;
}

//============================================================================
// measure first published in
//    Ochiai, A. (1957) "Zoogeographic studies on the soleoid fishes
//    found in Japan and its neighboring regions", Bulletin of the
//    Japanese Society of Scientific Fisheries 22:526-530.  Japanese
//    article with English summary.

template <typename VecT1, typename VecT2>
double ochiai_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   if (both + v1_only + v2_only == 0)
      return 1.0 ; // both all-zero vectors are defined to be identical
   else if (both + v1_only == 0 || both + v2_only == 0)
      return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
   return both / std::sqrt((both + v1_only) * (both + v2_only)) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double ochiai_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - ochiai_sim(v1,v2,opt) ;
}

//============================================================================
// measure first published in
//    Ochiai, A. (1957) "Zoogeographic studies on the soleoid fishes
//    found in Japan and its neighboring regions", Bulletin of the
//    Japanese Society of Scientific Fisheries 22:526-530.  Japanese
//    article with English summary.

template <typename VecT1, typename VecT2>
double binary_ochiai_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   if (both + v1_only + v2_only == 0)
      return 1.0 ; // both all-zero vectors are defined to be identical
   else if (both + v1_only == 0 || both + v2_only == 0)
      return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
   return both / std::sqrt((both + v1_only) * (both + v2_only)) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_ochiai_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_ochiai_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename VecT1, typename VecT2>
double routledge1_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type sum(both + v1_only + v2_only) ;
   sum *= sum ;
   double denom(sum - 2.0 * v1_only * v2_only) ;
   return denom ? (sum / denom - 1.0) : 1.0 ;// all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double routledge1_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - routledge1_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename VecT1, typename VecT2>
double binary_routledge1_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double sum(both + v1_only + v2_only) ;
   sum *= sum ;
   double denom(sum - 2.0 * v1_only * v2_only) ;
   return denom ? (sum / denom - 1.0) : 1.0 ;// all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_routledge1_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_routledge1_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename VecT1, typename VecT2>
double routledge2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double denom(2.0 * both + v1_only + v2_only) ;
   if (!denom)
      return 0.0 ;			// two all-zero vectors are identical
   typename VecT1::value_type total1(both+v1_only) ;
   typename VecT1::value_type total2(both+v2_only) ;
   double num1(2 * both * log(2)) ;
   double num2(p_log_p(total1) + p_log_p(total2)) ;
   return log(denom) - (num1 + num2) / denom ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double routledge2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - routledge2_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename VecT1, typename VecT2>
double binary_routledge2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double denom(2.0 * both + v1_only + v2_only) ;
   if (!denom)
      return 0.0 ;			// two all-zero vectors are identical
   size_t total1(both+v1_only) ;
   size_t total2(both+v2_only) ;
   double num1(2 * both * log(2)) ;
   double num2(p_log_p(total1) + p_log_p(total2)) ;
   return log(denom) - (num1 + num2) / denom ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_routledge2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_routledge2_dis(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
//  a / min(a+b,a+c) == a / (a + min(b,c))

template <typename VecT1, typename VecT2>
double simpson_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double denom(both + std::min(v1_only,v2_only)) ;
   return denom ? both/denom : 1.0 ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double simpson_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - simpson_sim(v1,v2,opt) ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double binary_simpson_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double denom(both + std::min(v1_only,v2_only)) ;
   return denom ? both/denom : 1.0 ;
}

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename VecT1, typename VecT2>
double binary_simpson_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_simpson_sim(v1,v2,opt) ;
}

//============================================================================
// https://en.wikipedia.org/wiki/Qualitative_variation says
//  2(a+d) / (2(a+d) + b + c)

template <typename VecT1, typename VecT2>
double sokal_sneath_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double denom(2*both + v1_only + v2_only) ;
   return denom ? 2*both / denom : 1.0 ; 	// all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double sokal_sneath_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - sokal_sneath_sim(v1,v2,opt) ;
}

//============================================================================
// https://en.wikipedia.org/wiki/Qualitative_variation says
//  2(a+d) / (2(a+d) + b + c)

template <typename VecT1, typename VecT2>
double binary_sokal_sneath_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   both += neither ;
   both *= 2 ;
   double denom(both + only_1) ;
   return denom ? both/denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_sokal_sneath_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_sokal_sneath_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Sorgenfrei (1959)
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double sorgenfrei_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type total1(both + v1_only) ;
   typename VecT1::value_type total2(both + v2_only) ;
   double denom(total1 * total2) ;
   return denom ? (both * both) / denom : 1.0 ;
}

//============================================================================
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double sorgenfrei_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - sorgenfrei_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Sorgenfrei (1959)
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_sorgenfrei_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double total1(both + v1_only) ;
   double total2(both + v2_only) ;
   double denom(total1 * total2) ;
   return denom ? (both * both) / denom : 1.0 ;
}

//============================================================================
// range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_sorgenfrei_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_sorgenfrei_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double tripartite_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double min1 = std::min(v1_only,v2_only) ;
   double max1 = std::max(v1_only,v2_only) ;
   double U = (both+max1) ? std::log2(1.0 + (both+min1) / (both+max1)) : 1.0 ;
   double S = 1.0 / std::sqrt(std::log2(2.0 + min1/(both+1.0))) ;
   double R_1 = (both+v1_only) ? std::log2(1.0 + both / (both+v1_only)) : 1.0 ;
   double R_2 = (both+v2_only) ? std::log2(1.0 + both / (both+v2_only)) : 1.0 ;
   double R = R_1 * R_2 ;
   return std::sqrt(U * S * R) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double tripartite_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - tripartite_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double binary_tripartite_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t both, v1_only, v2_only, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double min1 = std::min(v1_only,v2_only) ;
   double max1 = std::max(v1_only,v2_only) ;
   double U = (both+max1) ? std::log2(1.0 + (both+min1) / (both+max1)) : 1.0 ;
   double S = 1.0 / std::sqrt(std::log2(2.0 + min1/(both+1.0))) ;
   double R_1 = (both+v1_only) ? std::log2(1.0 + both / (both+v1_only)) : 1.0 ;
   double R_2 = (both+v2_only) ? std::log2(1.0 + both / (both+v2_only)) : 1.0 ;
   double R = R_1 * R_2 ;
   return std::sqrt(U * S * R) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_tripartite_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_tripartite_sim(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Whittaker (1960)
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double whittaker_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double sum(both + v1_only + v2_only) ;
   return sum ? (2.0 * sum / (both + sum)) - 1.0 : 1.0 ;
}

//============================================================================
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double whittaker_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - whittaker_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Whittaker (1960)
//  range: 0 ... 1
// 2(a + b + c) / (2a + b + c) - 1

template <typename VecT1, typename VecT2>
double binary_whittaker_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   size_t sum(both + only_1) ;
   return sum ? (2.0 * sum / (both + sum)) - 1.0 : 1.0 ;
}

//============================================================================
//  range: 0 ... 1

template <typename VecT1, typename VecT2>
double binary_whittaker_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_whittaker_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename VecT1, typename VecT2>
double williams_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double denom(both + v1_only + v2_only) ;
   return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double williams_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - williams_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename VecT1, typename VecT2>
double binary_williams_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double denom(both + v1_only + v2_only) ;
   return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_williams_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_williams_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename VecT1, typename VecT2>
double williams2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   typename VecT1::value_type sum(both + v1_only + v2_only) ;
   double denom(sum*sum - sum) ;
   return denom ? (2.0 * v1_only * v2_only + 1.0) / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double williams2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - williams2_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename VecT1, typename VecT2>
double binary_williams2_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t v1_only, v2_only, both, neither ;
   binary_contingency_table(v1,v2,both,v1_only,v2_only,neither) ;
   double sum(both + v1_only + v2_only) ;
   double denom(sum*sum - sum) ;
   return denom ? (2.0 * v1_only * v2_only + 1.0) / denom : 1.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_williams2_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_williams2_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Wilson & Shmida (1984)

template <typename VecT1, typename VecT2>
double wilsonshmida_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type both, v1_only, v2_only ;
   contingency_table(v1,v2,opt,both,v1_only,v2_only) ;
   double denom(2.0 * both + v1_only + v2_only) ;
   return denom ? (v1_only + v2_only) / denom : 1.0 ; // all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double wilsonshmida_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - wilsonshmida_dis(v1,v2,opt) ;
}

//============================================================================
//  see documentation for R-project "simba" package
//  published in Wilson & Shmida (1984)
// (b+c)/(2a+b+c)

template <typename VecT1, typename VecT2>
double binary_wilsonshmida_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions&)
{
   size_t both, only_1, neither ;
   binary_agreement(v1,v2,both,only_1,neither) ;
   double denom(2.0 * both + only_1) ;
   return denom ? only_1 / denom : 1.0 ; // all-zero vectors are defined to be identical
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double binary_wilsonshmida_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - binary_wilsonshmida_dis(v1,v2,opt) ;
}

//============================================================================
//   Measures NOT based on a 2x2 contingency table			    //
//============================================================================

//============================================================================
//  sum of absolute value of element-wise difference over sum of absolute values of elements

template <typename VecT1, typename VecT2>
double canberra_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   typename VecT1::value_type absdiff(0) ;
   typename VecT1::value_type sumabs(0) ;
   for( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 typename VecT1::value_type val1(std::abs(v1->elementValue(pos1++))) ;
	 absdiff += val1 ;
	 sumabs += val1 ;
	 }
      else if (elt1 > elt2)
	 {
	 typename VecT2::value_type val2(std::abs(v2->elementValue(pos2++))) ;
	 absdiff += val2 ;
	 sumabs += val2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 typename VecT1::value_type val1(v1->elementValue(pos1++)) ;
	 typename VecT2::value_type val2(v2->elementValue(pos2++)) ;
	 absdiff += std::abs(val1 - val2) ;
	 sumabs += std::abs(val1) + std::abs(val2) ;
	 }
      }
   return sumabs > 0 ? absdiff / (double)sumabs : 0.0 ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double canberra_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
		    const VectorSimilarityOptions& opt)
{
   ValT absdiff(0) ;
   ValT sumabs(0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT val1(v1->elementValue(i)) ;
      ValT val2(v2->elementValue(i)) ;
      absdiff += std::abs(val1 - val2) ;
      sumabs += std::abs(val1) + std::abs(val2) ;
      }
   // handle any leftovers from the first vector
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      absdiff += std::abs(v1->elementValue(i)) ;
      }
   // handle any leftovers from the second vector
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      absdiff += std::abs(v2->elementValue(i)) ;
      }
   return sumabs ? absdiff / (double)sumabs : 0.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double canberra_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - canberra_dis(v1,v2,opt) ;
}

//============================================================================
// Soergel: sum of element-wise difference over element-wise maximum
//     sum(abs(P_i-Q_i)) / sum(max(P_i,Q_i))
// from: Monev V., Introduction to Similarity Searching in Chemistry,
//   MATCH Commun. Math. Comput. Chem. 51 pp. 7-38 , 2004

template <typename VecT1, typename VecT2>
double soergel_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double total_diff(0) ;
   double total_max(0) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) ;
	 total_diff += std::abs(val1) ;
	 total_max += val1 ;
	 }
      else if (elt1 > elt2)
	 {
	 auto val2 = v2->elementValue(pos2++) ;
	 total_diff += std::abs(val2) ;
	 total_max += val2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) ;
	 auto val2 = v2->elementValue(pos2++) ;
	 total_diff += std::abs(val1 - val2) ;
	 total_max += std::max(val1 + val2) ;
	 }
      }   
   return total_max ? total_diff / total_max : -1.0 ;
}

//============================================================================
// sum of logarithmic differences
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename VecT1, typename VecT2>
double lorentzian_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 sum += std::log(1.0 + std::abs(v1->elementValue(pos1++))) ;
	 }
      else if (elt1 > elt2)
	 {
	 sum += std::log(1.0 + std::abs(v2->elementValue(pos2++))) ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) ;
	 auto val2 = v2->elementValue(pos2++) ;
	 sum += std::log(1.0 + std::abs(val1 - val2)) ;
	 }
      }
   return sum ;
}

//============================================================================
// Fidelity, aka Bhattacharyya coefficient or Hellinger affinity
//     sum(sqrt(P*Q))

template <typename VecT1, typename VecT2>
double fidelity_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 ++pos1 ;
	 }
      else if (elt1 > elt2)
	 {
	 ++pos2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) / wt1 ;
	 auto val2 = v2->elementValue(pos2++) / wt2 ;
	 sum += std::sqrt(val1 * val2) ;
	 }
      }
   return sum ;
}

//============================================================================
// Bhattacharyya distance
//    -ln(fidelity)

template <typename VecT1, typename VecT2>
double bhattacharyya_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   double fidelity = fidelity_sim(v1,v2,opt) ;
   return fidelity > 0 ? -std::log(fidelity) : HUGE_VAL ;
}

//============================================================================
// Hellinger distance
//    2*sqrt(1-fidelity)

template <typename VecT1, typename VecT2>
double hellinger_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   double fidelity = fidelity_sim(v1,v2,opt) ;
   return 2 * std::sqrt(1 - fidelity) ;
}

//============================================================================
// Matusita distance
//    sqrt(2 - 2*fidelity)

template <typename VecT1, typename VecT2>
double matusita_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   double fidelity = fidelity_sim(v1,v2,opt) ;
   return std::sqrt(2 - 2 * fidelity) ;
}

//============================================================================
// squared-chord distance
//	sum((sqrt(P_i) - sqrt(Q_i))^2)
// note: required non-negative elements!
// from: Gavin D.G., Oswald W.W., Wahl, E.R., and Williams J.W., A statistical
//   approach to evaluating distance metrics and analog assignments for pollen
//   records, Quaternary Research 60, pp 356–367, 2003

template <typename VecT1, typename VecT2>
double squared_chord_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) / wt1 ;
	 sum += std::abs(val1) ;
	 }
      else if (elt1 > elt2)
	 {
	 auto val2 = v2->elementValue(pos2++) / wt2 ;
	 sum += std::abs(val2) ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) / wt1 ;
	 auto val2 = v2->elementValue(pos2++) / wt2 ;
	 double value = std::sqrt(val1) - std::sqrt(val2) ;
	 sum += (value * value) ;
	 }
      }
   return sum ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double squared_chord_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1 - squared_chord_dis(v1,v2,opt) ;
   // equivalent to 2*fidelity_sim() - 1
}

//============================================================================
// Clark
//     sqrt(sum((absdiff/sum)**2))
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename VecT1, typename VecT2>
double clark_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 ++pos1 ;
	 sum += 1.0 ;
	 }
      else if (elt1 > elt2)
	 {
	 ++pos2 ;
	 sum += 1.0 ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) / wt1 ;
	 auto val2 = v2->elementValue(pos2++) / wt2 ;
	 double value = std::abs(val1-val2) / (val1 + val2) ;
	 sum += (value * value) ;
	 }
      }
   return std::sqrt(sum) ;
}

//============================================================================
// probabilistic symmetric chi-squared / Sangvi chi-squared between populations
//     2*sum((diff**2)/(sum))
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename VecT1, typename VecT2>
double sangvi_chisquared_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 sum += std::abs(v1->elementValue(pos1++)) / wt1 ;
	 }
      else if (elt1 > elt2)
	 {
	 sum += std::abs(v2->elementValue(pos2++)) / wt2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1 = v1->elementValue(pos1++) / wt1 ;
	 auto val2 = v2->elementValue(pos2++) / wt2 ;
	 double diff = val1 - val2 ;
	 double elt_sum = val1 + val2 ;
	 if (elt_sum) sum += (diff * diff / elt_sum) ;
	 }
      }
   return std::sqrt(sum) ;
}

//============================================================================
// normalized sum of element-wise minimums

template <typename VecT1, typename VecT2>
double circle_product_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 ++pos1 ;
	 }
      else if (elt1 > elt2)
	 {
	 ++pos2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 sum += std::min(v1->elementValue(pos1++) / wt1,v2->elementValue(pos2++) / wt2) ;
	 }
      }
   size_t total(v1->numElements() + v2->numElements()) ;
   return total ? sum / total : 0.0 ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double circle_product_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
			  const VectorSimilarityOptions& opt)
{
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   double sum(0.0) ;
   if (opt.normalize)
      {
      ValT wt1, wt2 ;
      normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
      for (size_t i = 0 ; i < minlen ; ++i)
	 {
	 sum += std::min(v1->elementValue(i)/wt1,v2->elementValue(i)/wt2) ;
	 }
      }
   else
      {
      for (size_t i = 0 ; i < minlen ; ++i)
	 {
	 sum += std::min(v1->elementValue(i),v2->elementValue(i)) ;
	 }
      }
   // no need to handle leftovers; since we assume elements are always non-negative,
   //   the minimums for the "missing" element comparisons will always be zero
   size_t total(v1->numElements() + v2->numElements()) ;
   return total ? sum / total : 0.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double circle_product_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - circle_product_dis(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double euclidean_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return std::sqrt(squared_euclidean_dis(v1,v2,opt)) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double euclidean_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - euclidean_dis(v1,v2,opt) ; //FIXME
}

//============================================================================

template <typename VecT1, typename VecT2>
double jaro_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   // string similarity measure between strings s1 and s2
   // for m=#matching_chars
   //   d = 0 if m = 0
   //   d = (m/|s1| + m/|s2| + (m-(#transpositions/2))/m) / 3
   //
   // chars are considered matches if the same and not more than floor(max(|s1|,|s2|)/2)-1
   //   positions apart
   return 0.0 ; //FIXME
}

//============================================================================

template <typename VecT1, typename VecT2>
double jaro_winkler_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   // for strings s1 and s2:
   // d = jaro_distance(s1,s2) + (prefixlen*scale_factor*(1 - jaro_distance(s1,s2))) ;
   // where prefixlen=min(|common_prefix(s1,s2)|,4) ;
   // scale_factor <= 0.25; standard value 0.1
   // see: http://web.archive.org/web/20100227020019/http://www.census.gov/geo/msb/stand/strcmp.c
   return 0.0 ; //FIXME
}

//============================================================================
// Jensen difference: sum of average of entropies less entropy of average

template <typename VecT1, typename VecT2>
double jensen_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type totalwt1, totalwt2 ;
   normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0.0) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      double val1(0) ;
      double val2(0) ;
      if (elt1 < elt2)
	 {
	 val1 = v1->elementValue(pos1++) ;
	 }
      else if (elt1 > elt2)
	 {
	 val2 = v2->elementValue(pos2++) ;
	 }
      else // if (elt1 == elt2)
	 {
	 val1 = v1->elementValue(pos1++) ;
	 val2 = v2->elementValue(pos2++) ;
	 }
      double ent1 = p_log_p(val1) ;
      double ent2 = p_log_p(val2) ;
      double ent_avg = p_log_p((val1+val2)/2) ;
      sum += (ent1+ent2)/2 - ent_avg ;
      }
   return sum ;
}

//============================================================================
//      JS(a,b) == (KL(a|avg(a,b)) + KL(b|avg(a,b))) / 2
//      KL(a|b) == sum_y( a(y)(log a(y) - log b(y)) )

template <typename ValT>
ValT KL_term(ValT a, ValT b)
{
   return (a>0 && b>0) ? a * (std::log2(a) - std::log2(b)) : 0.0 ;
}

//----------------------------------------------------------------------------

template <typename ValT>
ValT JS_term(ValT a, ValT b)
{
   ValT avg = (a + b) / 2 ;
   return KL_term(a,avg) + KL_term(b,avg) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double jensen_shannon_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   typename VecT1::value_type totalwt1, totalwt2 ;
   normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0.0) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 double val1(v1->elementValue(pos1++) / totalwt1) ;
	 sum += JS_term(val1,static_cast<typename VecT2::value_type>(0)) ;
	 }
      else if (elt1 > elt2)
	 {
	 double val2(v2->elementValue(pos2++) / totalwt2) ;
	 sum += JS_term(static_cast<typename VecT1::value_type>(0),val2) ;
	 }
      else // if (elt1 == elt2)
	 {
	 double val1(v1->elementValue(pos1++) / totalwt1) ;
	 double val2(v2->elementValue(pos2++) / totalwt2) ;
	 sum += JS_term(val1,val2) ;
	 }
      }
   return sum / 2.0 ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double jensen_shannon_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
			  const VectorSimilarityOptions& opt)
{
   ValT totalwt1, totalwt2 ;
   normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
   double sum(0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT val1(v1->elementValue(i)/totalwt1) ;
      ValT val2(v2->elementValue(i)/totalwt2) ;
      sum += JS_term(val1,val2) ;
      }
   // handle leftover elements in the first vector (if second is shorter)
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      ValT val1(v1->elementValue(i)/totalwt1) ;
      sum += JS_term(val1,0) ;
      }
   // handle leftover elements in the second vector (if first is shorter)
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      ValT val2(v2->elementValue(i)/totalwt2) ;
      sum += JS_term(0,val2) ;
      }
   return sum / 2.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double jensen_shannon_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - jensen_shannon_dis(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double kullback_leibler_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   typename VecT1::value_type totalwt1, totalwt2 ;
   normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
   double sum(0) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 auto val1(v1->elementValue(pos1++)/totalwt1) ;
	 sum += KL_term(val1,0) ;
	 }
      else if (elt1 > elt2)
	 {
	 auto val2(v2->elementValue(pos2++)/totalwt2) ;
	 sum += KL_term(0,val2) ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto val1(v1->elementValue(pos1++)/totalwt1) ;
	 auto val2(v2->elementValue(pos2++)/totalwt2) ;
	 sum += KL_term(val1,val2) ;
	 }
      }
   return sum ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double kullback_leibler_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
			    const VectorSimilarityOptions& opt)
{
   ValT totalwt1, totalwt2 ;
   normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
   double sum(0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT val1(v1->elementValue(i)/totalwt1) ;
      ValT val2(v2->elementValue(i)/totalwt2) ;
      sum += KL_term(val1,val2) ;
      }
   // handle leftover elements in the first vector (if second is shorter)
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      ValT val1(v1->elementValue(i)/totalwt1) ;
      sum += KL_term(val1,0) ;
      }
   // handle leftover elements in the second vector (if first is shorter)
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      ValT val2(v2->elementValue(i)/totalwt2) ;
      sum += KL_term(0,val2) ;
      }
   return sum ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double kullback_leibler_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - kullback_leibler_dis(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double l0_norm_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   // count the non-zero elements of the difference between the two vectors
   size_t differences(0) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 ++pos1 ;
	 ++differences ;
	 }
      else if (elt1 > elt2)
	 {
	 ++pos2 ;
	 ++differences ;
	 }
      else // if (elt1 == elt2)
	 {
	 if (v1->elementValue(pos1++) != v2->elementValue(pos2++)) ++differences ;
	 }
      }
   size_t total(v1->numElements() + v2->numElements()) ;
   return total ? differences/total : 0.0 ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double l0_norm_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
		   const VectorSimilarityOptions& opt)
{
   size_t differences(0) ;
   size_t minlen = std::min(v1->numElements(),v2->numElements()) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      if (v1->elementValue(i) != v2->elementValue(i)) ++differences ;
      }
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      if (v1->elementValue(i)) ++differences ;
      }
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      if (v2->elementValue(i)) ++differences ;
      }
   size_t total(v1->numElements() + v2->numElements()) ;
   return total ? 2.0 * differences / total : 0.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double l0_norm_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - l0_norm_dis(v1,v2,opt) ;
}

//============================================================================
// L-norm for L(infinity) = maximum elementwise difference
// aka Chebyshev

template <typename VecT1, typename VecT2>
double linf_norm_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   typename VecT1::value_type maxdiff(0) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 typename VecT1::value_type val1(std::abs(v1->elementValue(pos1++))) ;
	 if (val1 > maxdiff) maxdiff = val1 ;
	 }
      else if (elt1 > elt2)
	 {
	 typename VecT1::value_type val2(std::abs(v2->elementValue(pos2++))) ;
	 if (val2 > maxdiff) maxdiff = val2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 typename VecT1::value_type dif(std::abs(v1->elementValue(pos1++) - v2->elementValue(pos2++))) ;
	 if (dif > maxdiff) maxdiff = dif ;
	 }
      }
   return maxdiff ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double linf_norm_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
		   const VectorSimilarityOptions& opt)
{
   ValT maxdiff(0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT diff = std::abs(v1->elementValue(i) - v2->elementValue(i)) ;
      if (diff > maxdiff) maxdiff = diff ;
      }
   // handle any leftovers from the first vector
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      ValT diff = std::abs(v1->elementValue(i)) ;
      if (diff > maxdiff) maxdiff = diff ;
      }
   // handle any leftovers from the second vector
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      ValT diff = std::abs(v2->elementValue(i)) ;
      if (diff > maxdiff) maxdiff = diff ;
      }
   return (double)maxdiff ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double linf_norm_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - linf_norm_dis(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double mahalanobis_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
//FIXME
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 ++pos1 ;
	 }
      else if (elt1 > elt2)
	 {
	 ++pos2 ;
	 }
      else // if (elt1 == elt2)
	 {
	 ++pos1; ++pos2;
	 }
      }
   return 0.0 ; //FIXME
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double mahalanobis_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
		       const VectorSimilarityOptions& opt)
{
//FIXME   
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      }
   // handle any leftovers from the first vector
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      }
   // handle any leftovers from the second vector
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      }
   return 1.0 ; //FIXME
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double mahalanobis_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - mahalanobis_dis(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double manhattan_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   typename VecT1::value_type sum(0) ;
   if (opt.normalize)
      {
      typename VecT1::value_type wt1, wt2 ;
      normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
      for ( ; pos1 < elts1 && pos2 < elts2 ; )
	 {
	 auto elt1(v1->elementIndex(pos1)) ;
	 auto elt2(v2->elementIndex(pos2)) ;
	 if (elt1 < elt2)
	    {
	    sum += std::abs(v1->elementValue(pos1++)/wt1) ;
	    }
	 else if (elt1 > elt2)
	    {
	    sum += std::abs(v2->elementValue(pos2++)/wt2) ;
	    }
	 else // if (elt1 == elt2)
	    {
	    sum += std::abs(v1->elementValue(pos1++)/wt1 - v2->elementValue(pos2++)/wt2) ;
	    }
	 }
      }
   else // no normalization
      {
      for ( ; pos1 < elts1 && pos2 < elts2 ; )
	 {
	 auto elt1(v1->elementIndex(pos1)) ;
	 auto elt2(v2->elementIndex(pos2)) ;
	 if (elt1 < elt2)
	    {
	    sum += std::abs(v1->elementValue(pos1++)) ;
	    }
	 else if (elt1 > elt2)
	    {
	    sum += std::abs(v2->elementValue(pos2++)) ;
	    }
	 else // if (elt1 == elt2)
	    {
	    sum += std::abs(v1->elementValue(pos1++) - v2->elementValue(pos2++)) ;
	    }
	 }
      }
   return (double)sum ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double manhattan_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
		       const VectorSimilarityOptions& opt)
{
   double sum(0.0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   if (opt.normalize)
      {
      ValT wt1, wt2 ;
      normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
      for (size_t i = 0 ; i < minlen ; ++i)
	 {
	 sum += std::abs(v1->elementValue(i)/wt1 - v2->elementValue(i)/wt2) ;
	 }
      // handle any leftovers from the first vector
      for (size_t i = minlen ; i < v1->numElements() ; ++i)
	 {
	 sum += std::abs(v1->elementValue(i)/wt1) ;
	 }
      // handle any leftovers from the second vector
      for (size_t i = minlen ; i < v2->numElements() ; ++i)
	 {
	 sum += std::abs(v2->elementValue(i)/wt2) ;
	 }
      }
   else
      {
      // no normalization
      for (size_t i = 0 ; i < minlen ; ++i)
	 {
	 sum += std::abs(v1->elementValue(i) - v2->elementValue(i)) ;
	 }
      // handle any leftovers from the first vector
      for (size_t i = minlen ; i < v1->numElements() ; ++i)
	 {
	 sum += std::abs(v1->elementValue(i)) ;
	 }
      // handle any leftovers from the second vector
      for (size_t i = minlen ; i < v2->numElements() ; ++i)
	 {
	 sum += std::abs(v2->elementValue(i)) ;
	 }
      }
   return (double)sum ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double manhattan_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - manhattan_dis(v1,v2,opt) ;//FIXME
}

//============================================================================
// this is the measure from the original FramepaC, and differs from that
// described in
//    Robinson, W.S. 1951 A method for chronologically ordering
//    archaeological deposits. American Antiquity, 16: 293-301.

template <typename VecT1, typename VecT2>
double robinson_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   double sum(0) ;
   typename VecT1::value_type totalwt1, totalwt2 ;
   normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 double val1(v1->elementValue(pos1++) / totalwt1) ;
	 sum += std::abs(val1) ;
	 }
      else if (elt1 > elt2)
	 {
	 double val2(v2->elementValue(pos2++) / totalwt2) ;
	 sum += std::abs(val2) ;
	 }
      else // if (elt1 == elt2)
	 {
	 double val1(v1->elementValue(pos1++) / totalwt1) ;
	 double val2(v2->elementValue(pos2++) / totalwt2) ;
	 sum += std::abs(val1 - val2) ;
	 }
      }
   return sum / 2.0 ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double robinson_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
		    const VectorSimilarityOptions& opt)
{
   ValT sum(0) ;
   ValT totalwt1, totalwt2 ;
   normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT val1(v1->elementValue(i) / totalwt1) ;
      ValT val2(v2->elementValue(i) / totalwt2) ;
      sum += std::abs(val1 - val2) ;
      }
   // handle any leftovers from the first vector
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      ValT val1(v1->elementValue(i) / totalwt1) ;
      sum += std::abs(val1) ;
      }
   // handle any leftovers from the second vector
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      ValT val2(v2->elementValue(i) / totalwt2) ;
      sum += std::abs(val2) ;
      }
   return sum / 2.0 ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double robinson_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - robinson_dis(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double similarity_ratio_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   double sumsq(0) ;
   double prod(0) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   typename VecT1::value_type wt1, wt2 ;
   normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 double val1(v1->elementValue(pos1++) / wt1) ;
	 sumsq += (val1 * val1) ;
	 }
      else if (elt1 > elt2)
	 {
	 double val2(v2->elementValue(pos2++) / wt2) ;
	 sumsq += (val2 * val2) ;
	 }
      else // if (elt1 == elt2)
	 {
	 double val1(v1->elementValue(pos1++) / wt1) ;
	 double val2(v2->elementValue(pos2++) / wt2) ;
	 sumsq += (val1 * val1) + (val2 * val2) ;
	 prod += (val1 * val2) ;
	 }
      }
   return (sumsq == prod) ? DBL_MAX : prod / (sumsq - prod) ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double similarity_ratio_sim(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
			    const VectorSimilarityOptions& opt)
{
   double sumsq(0) ;
   double prod(0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   if (opt.normalize)
      {
      ValT wt1, wt2 ;
      normalization_weights(v1,v2,opt.normalize,wt1,wt2) ;
      for (size_t i = 0 ; i < minlen ; ++i)
	 {
	 double val1(v1->elementValue(i) / wt1) ;
	 double val2(v2->elementValue(i) / wt2) ;
	 sumsq += (val1 * val1) + (val2 * val2) ;
	 prod += (val1 * val2) ;
	 }
      for (size_t i = minlen ; i < v1->numElements() ; ++i)
	 {
	 double val1(v1->elementValue(i) / wt1) ;
	 sumsq += (val1 * val1) ;
	 }
      for (size_t i = minlen ; i < v2->numElements() ; ++i)
	 {
	 double val2(v2->elementValue(i) / wt2) ;
	 sumsq += (val2 * val2) ;
	 }
      }
   else
      {
      for (size_t i = 0 ; i < minlen ; ++i)
	 {
	 ValT val1(v1->elementValue(i)) ;
	 ValT val2(v2->elementValue(i)) ;
	 sumsq += (val1 * val1) + (val2 * val2) ;
	 prod += (val1 * val2) ;
	 }
      for (size_t i = minlen ; i < v1->numElements() ; ++i)
	 {
	 ValT val1(v1->elementValue(i)) ;
	 sumsq += (val1 * val1) ;
	 }
      for (size_t i = minlen ; i < v2->numElements() ; ++i)
	 {
	 ValT val2(v2->elementValue(i)) ;
	 sumsq += (val2 * val2) ;
	 }
      }
   return (sumsq == prod) ? DBL_MAX : prod  / (sumsq - prod) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double similarity_ratio_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - similarity_ratio_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double squared_euclidean_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   double sum(0) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 auto diff(v1->elementValue(pos1++)) ;
	 sum += (diff * diff) ;
	 }
      else if (elt1 > elt2)
	 {
	 auto diff(v2->elementValue(pos2++)) ;
	 sum += (diff * diff) ;
	 }
      else // if (elt1 == elt2)
	 {
	 auto diff(v1->elementValue(pos1++) - v2->elementValue(pos2++)) ;
	 sum += (diff * diff) ;
	 }
      }
   return sum ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double squared_euclidean_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
			     const VectorSimilarityOptions& opt)
{
   ValT sum(0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT diff = v1->elementValue(i) - v2->elementValue(i) ;
      sum += (diff * diff) ;
      }
   // handle any leftovers from the first vector
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      ValT diff = v1->elementValue(i) ;
      sum += (diff * diff) ;
      }
   // handle any leftovers from the second vector
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      ValT diff = v2->elementValue(i) ;
      sum += (diff * diff) ;
      }
   return (double)sum  ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double squared_euclidean_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - squared_euclidean_sim(v1,v2,opt) ;
}

//============================================================================

template <typename VecT1, typename VecT2>
double l_norm_dis(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   double power(opt.power) ;
   if (power <= 0.0)
      return l0_norm_dis(v1,v2,opt) ;
   else if (power == 1.0)
      return manhattan_dis(v1,v2,opt) ;
   else if (power == 2.0)
      return euclidean_dis(v1,v2,opt) ;
   double sum(0.0) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   for ( ; pos1 < elts1 && pos2 < elts2 ; )
      {
      auto elt1(v1->elementIndex(pos1)) ;
      auto elt2(v2->elementIndex(pos2)) ;
      if (elt1 < elt2)
	 {
	 double diff(std::abs(v1->elementValue(pos1++))) ;
	 sum += std::pow(diff,power) ;
	 }
      else if (elt1 > elt2)
	 {
	 double diff(std::abs(v2->elementValue(pos2++))) ;
	 sum += std::pow(diff,power) ;
	 }
      else // if (elt1 == elt2)
	 {
	 double diff(std::abs(v1->elementValue(pos1++) - v2->elementValue(pos2++))) ;
	 sum += std::pow(diff,power) ;
	 }
      }
   return std::pow(sum,1.0/power) ;
}

//----------------------------------------------------------------------------
// a more efficient specialization for two dense vectors

template <typename ValT>
double l_norm_dis(const DenseVector<ValT> *v1, const DenseVector<ValT> *v2,
		   const VectorSimilarityOptions& opt)
{
   double power(opt.power) ;
   if (power <= 0.0)
      return l0_norm_dis(v1,v2,opt) ;
   else if (power == 1.0)
      return manhattan_dis(v1,v2,opt) ;
   else if (power == 2.0)
      return euclidean_dis(v1,v2,opt) ;
   double sum(0.0) ;
   size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
   for (size_t i = 0 ; i < minlen ; ++i)
      {
      ValT diff = std::abs(v1->elementValue(i) - v2->elementValue(i)) ;
      sum += std::pow(diff,power) ;
      }
   // handle any leftovers from the first vector
   for (size_t i = minlen ; i < v1->numElements() ; ++i)
      {
      ValT diff = std::abs(v1->elementValue(i)) ;
      sum += std::pow(diff,power) ;
      }
   // handle any leftovers from the second vector
   for (size_t i = minlen ; i < v2->numElements() ; ++i)
      {
      ValT diff = std::abs(v2->elementValue(i)) ;
      sum += std::pow(diff,power) ;
      }
   return std::pow(sum,1.0/power) ;
}

//----------------------------------------------------------------------------

template <typename VecT1, typename VecT2>
double l_norm_sim(const VecT1* v1, const VecT2* v2, const VectorSimilarityOptions& opt)
{
   return 1.0 - l_norm_dis(v1,v2,opt) ;
}

//============================================================================


} // end namespace FramepaC::vecsim
} // end namespace FramepaC

/************************************************************************/
/************************************************************************/

namespace Fr
{



} // end namespace Fr

// end of file vecsim.cc //
