/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-27					*/
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

#include <cmath>
#include <float.h>
#include "framepac/vecsim.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

template <typename IdxT, typename ValT>
void VectorMeasure<IdxT,ValT>::contingencyTable(const Vector<ValT>* v1, const Vector<ValT>* v2,
   ValT& a, ValT& b, ValT& c) const
{
   a = b = c = 0 ;
   if (!v1 || !v2)
      {
      return ;
      }
   ValT wt1, wt2 ;
   normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   if (!v1->isSparseVector() && !v2->isSparseVector())
      {
      const DenseVector<ValT>* dv1 = static_cast<const DenseVector<ValT>*>(v1) ;
      const DenseVector<ValT>* dv2 = static_cast<const DenseVector<ValT>*>(v2) ;
      // figure out how many elements to process in both vectors
      size_t minlen = std::min(elts1,elts2) ;
      // if either vector is all zeros, we don't need to treat it in
      //   common with the other one
      if (wt1 == 0.0 || wt2 == 0.0) minlen = 0 ;
      for ( ; pos1 < minlen ; ++pos1)
	 {
	 ValT val1(dv1->elementValue(pos1)) ;
	 ValT val2(dv2->elementValue(pos1)) ;
	 ValT com(std::min(val1/wt1,val2/wt2)) ;
	 a += com ;
	 b += (val1 - wt1*com) ;
	 c += (val2 - wt2*com) ;
	 }
      pos2 = pos1 ;
      }
   else
      {
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
	    auto val1(v1->elementValue(pos1++)) ;
	    auto val2(v2->elementValue(pos2++)) ;
	    auto com(std::min(val1/wt1,val2/wt2)) ;
	    a += com ;
	    b += (val1 - wt1*com) ;
	    c += (val2 - wt2*com) ;
	    }
	 }
      }
   // handle any leftovers from the first vector (if the second is shorter or zero)
   while (pos1 < elts1)
      {
      b += v1->elementValue(pos1++) ;
      }
   // handle any leftovers from the second vector (if the first is shorter or zero)
   while (pos2 < elts2)
      {
      c += v2->elementValue(pos2++) ;
      }
   b /= wt1 ;
   c /= wt2 ;
   return;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
void VectorMeasure<IdxT,ValT>::contingencyTable(const Vector<ValT>* v1, const Vector<ValT>* v2,
   size_t& both, size_t& v1_only, size_t& v2_only, size_t& neither) const
{
   both = v1_only = v2_only = neither = 0 ;
   if (!v1 || !v2)
      {
      return ;
      }
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   if (!v1->isSparseVector() && !v2->isSparseVector())
      {
      const DenseVector<ValT>* dv1 = static_cast<const DenseVector<ValT>*>(v1) ;
      const DenseVector<ValT>* dv2 = static_cast<const DenseVector<ValT>*>(v2) ;
      // figure out how many elements to process in both vectors
      size_t minlen = std::min(elts1,elts2) ;
      for ( ; pos1 < minlen ; ++pos1)
	 {
	 ValT val1 = dv1->elementValue(pos1) ;
	 ValT val2 = dv2->elementValue(pos1) ;
	 if (val1 && val2)
	    both++ ;
	 else if (val1)
	    v1_only++ ;
	 else if (val2)
	    v2_only++ ;
	 else
	    neither++ ;
	 }
      pos2 = pos1 ;
      }
   else
      {
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

template <typename IdxT, typename ValT>
void VectorMeasure<IdxT,ValT>::binaryAgreement(const Vector<ValT>* v1, const Vector<ValT>* v2,
   size_t& both, size_t& disagree, size_t& neither) const
{
   if (!v1 || !v2)
      {
      neither = disagree = both = 0 ;
      return ;
      }
   size_t pos1(0) ;
   size_t pos2(0) ;
   size_t elts1(v1->numElements()) ;
   size_t elts2(v2->numElements()) ;
   size_t stats[3] { 0, 0, 0 } ;
   if (!v1->isSparseVector() && !v2->isSparseVector())
      {
      // dense vectors
      // figure out how many elements to process in both vectors
      size_t minlen = std::min(elts1,elts2) ;
      for ( ; pos1 < minlen ; ++pos1)
	 {
	 bool val1(v1->elementValue(pos1)) ;
	 bool val2(v2->elementValue(pos1)) ;
	 stats[val1+val2]++ ;
	 }
      pos2 = pos1 ;
      }
   else
      {
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
      }
   // handle any leftovers from the first vector (if the second is shorter)
   while (pos1 < elts1)
      {
      bool val(v1->elementValue(pos1++)) ;
      stats[val]++ ;
      }
   // handle any leftovers from the second vector (if the first is shorter)
   while (pos2 < elts2)
      {
      bool val(v2->elementValue(pos2++)) ;
      stats[val]++ ;
      }
   neither = stats[0] ;
   disagree = stats[1] ;
   both = stats[2] ;
   return ;
}

//----------------------------------------------------------------------------

} // end namespace Fr

//============================================================================

namespace FramepaC
{
namespace vecsim
{

using Fr::VectorSimilarityOptions ;
using Fr::Vector ;
using Fr::DenseVector ;
using Fr::SparseVector ;

template <typename ValT>
ValT p_log_p(ValT p)
{
   return p ? p * std::log(p) : 0 ;
}

//----------------------------------------------------------------------------

double safe_div(double num, double denom)
{
   return denom ? num / denom : (num ? HUGE_VAL : 0) ;
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

//============================================================================
//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureCosine : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Cosine" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    double prod_lengths((v1->length() * v2->length())) ;
	    if (!prod_lengths)
	       return 0.0 ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT dotprod(0) ;
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
      virtual double denseSimilarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;
      
//============================================================================
//   Measures based on a 2x2 contingency table			            //
//============================================================================

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureAntiDice : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Anti-Dice" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT denom(both + 2.0*(v1_only + v2_only)) ;
	    return (denom > 0) ? both / (double)denom : 1.0 ;
	 }
   } ;

//============================================================================
// measure first published in
//      Sokal, R.R. and Sneath, P.H.A. (1963) Principles of Numerical
//      Taxononmy.  San Franciso: Freeman.  pp.129.
// and credited to
//      Anderberg, M.R. (1973) Cluster Analysis for Applications.  New
//      York: Academic Press.
//  a / (a + 2*(b+c))
// Rogers&Tanimoto(1960) used (a+d)/((a+d)+2(b+c))

template <typename IdxT, typename ValT>
class VectorMeasureBinaryAntiDice : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Anti-Dice" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    double denom(both + 2.0 * only_1) ;
	    return (denom > 0.0) ? both / denom : 1.0 ; // (all-zero vectors are defined to be identical)
	 }
      //virtual double denseSimilarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
// measure first published in
//      Benini, R. (1901) "Principii di demografie".  No. 29 of
//      manuali Barbera di science giuridiche sociali e poltiche.
//      Firenzi: G. Barbera

template <typename IdxT, typename ValT>
class VectorMeasureBenini : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Benini" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT N(both + v1_only + v2_only) ;
	    both /= N ;
	    v1_only /= N ;
	    v2_only /= N ;
	    ValT prod((both+v1_only)*(both+v2_only)) ;
	    return (both - prod) / (both + std::min(v1_only,v2_only) - prod) ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
// measure first published in
//      Benini, R. (1901) "Principii di demografie".  No. 29 of
//      manuali Barbera di science giuridiche sociali e poltiche.
//      Firenzi: G. Barbera

template <typename IdxT, typename ValT>
class VectorMeasureBinaryBenini : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Benini" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double N(both + v1_only + v2_only + neither) ;
	    double total1((both+v1_only)/N) ;
	    double total2((both+v2_only)/N) ;
	    double prod(total1*total2) ;
	    double match(both/N) ;
	    return (match - prod) / (match + std::min(v1_only,v2_only)/N - prod) ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBraunBlanquet : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Braun-Blanquet" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT larger(std::max(both+v1_only,both+v2_only)) ;
	    return larger > 0 ? both / larger : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryBraunBlanquet : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Braun-Blanquet" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double larger(std::max(both+v1_only,both+v2_only)) ;
	    return larger > 0 ? both / larger : 1.0 ;
	 }
   } ;

//============================================================================
//  original publication:
//     Bray, J.R. and Curtis, J.T. (1957) "An ordination of upland
//     forest communities of southern Wisconsin". Ecological
//     Monographs 27:325-349.
// aka Czekanowski's quantitative index: sum_i(min(x_i,y_i))/sum_i(x_i+y_i)

template <typename IdxT, typename ValT>
class VectorMeasureBrayCurtis : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Bray-Curtis" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT sum(2.0 * both + v1_only + v2_only) ;
	    return sum ? (v1_only + v2_only) / sum : 1.0 ;
	 }
   } ;

//============================================================================
//  original publication:
//     Bray, J.R. and Curtis, J.T. (1957) "An ordination of upland
//     forest communities of southern Wisconsin". Ecological
//     Monographs 27:325-349.
// (b+c)/(2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryBrayCurtis : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Bray-Curtis" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    size_t sum(2.0 * both + only_1) ;
	    return sum ? only_1 / sum : 1.0 ;
	 }
      //virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Colwell&Coddington (1948) and Gaston (2001)
// (b+c)/(2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureCocogaston : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Cocogaston" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT dif(v1_only + v2_only) ;
	    if (both + dif == 0)
	       return 1.0 ;			// all-zero vectors are defined to be identical
	    return dif / (2.0 * both + dif) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Colwell&Coddington (1948) and Gaston (2001)
// (b+c)/(2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryCocogaston : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Cocogaston" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    if (both + only_1 == 0)
	       return 1.0 ; // all-zero vectors are defined to be identical
	    return only_1 / (2.0 * both + only_1) ;
	 }
      //virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Cody (1993)

template <typename IdxT, typename ValT>
class VectorMeasureCody : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Cody" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT total1(both + v1_only) ;
	    ValT total2(both + v2_only) ;
	    if (total1 * total2 <= 0)
	       return 1.0 ;			// all-zero vectors are defined to be identical
	    return both * (total1 + total2) / (2.0 * total1 * total2) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Cody (1993)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryCody : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Cody" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double total1(both + v1_only) ;
	    double total2(both + v2_only) ;
	    double denom(2.0 * total1 * total2) ;
	    if (!denom)
	       return 1.0 ; // all-zero vectors are defined to be identical
	    return both * (total1 + total2) / denom ;
	 }
   } ;

//============================================================================
//  2a / (2a + b + c)
// aka Czekanowski
// = 1 - Harte dissim,
//      Harte, J. & Kinzig, A. (1997) "On the implications of
//      species-area relationships for endemism, spatial turnover and
//      food web patterns". Oikos 80.

template <typename IdxT, typename ValT>
class VectorMeasureDice : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Dice" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    if (both + v1_only + v2_only <= 0.0)
	       return 1.0 ;			// all-zero vectors are defined to be identical
	    return 2.0 * both / (2.0 * both + v1_only + v2_only) ;
	 }
   } ;

//============================================================================
//  measure published in
//     Czekanowski, J. (1932) '"Coefficient of racial likeness" und
//     "durchschnittliche Differenz"'.  Anthropologischer Anzeiger
//     9:227-249.
//  and
//     Dice, L.R. (1945) "Measures of the amount of ecologic
//     association between species".  Ecology 26:297-302.
// 2a / (2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryDice : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Dice" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    both *= 2 ;
	    double denom(both + only_1) ;
	    return denom ? both / denom : 1.0 ;// all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  a / (sqrt(a+b)(a+c)) - 0.5*max(b,c)

template <typename IdxT, typename ValT>
class VectorMeasureFagerMcGowan : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Fager-McGowan" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT prod((both+v1_only)*(both+v2_only)) ;
	    double denom(std::sqrt(prod) - std::max(v1_only,v2_only)/2) ;
	    return denom ? both / denom : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryFagerMcGowan : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Fager-McGowan" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double prod((both+v1_only)*(both+v2_only)) ;
	    double denom(std::sqrt(prod) - std::max(v1_only,v2_only)/2) ;
	    return denom ? both / denom : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureGamma : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Gamma" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT N(both + v1_only + v2_only) ;
	    ValT neither(0) ; //FIXME
	    double concordance(both / N * neither / N) ;
	    double discordance(v1_only / N * v2_only / N) ;
	    return (concordance + discordance > 0) ? (concordance - discordance) / (concordance + discordance) : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryGamma : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Gamma" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (!elts1 && !elts2)
	       return 1.0 ;			// two zero-length vectors are identical
	    else if (!elts1 || !elts2)
	       return -1.0 ;			// maximal difference if only one vector zero-length
	    size_t both, v1_only, v2_only, neither ;
	    contingencyTable(v1,v2,both,v1_only,v2_only,neither) ;
	    double N(both + v1_only + v2_only + neither) ;
	    double concordance(both / N * neither / N) ;
	    double discordance(v1_only / N * v2_only / N) ;
	    return (concordance + discordance > 0) ? (concordance - discordance) / (concordance + discordance) : 1.0 ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureGilbert : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Gilbert" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT N(both + v1_only + v2_only) ;
	    both /= N ;
	    v1_only /= N ;
	    v2_only /= N ;
	    ValT prod((both+v1_only)*(both+v2_only)) ;
	    return (both - prod) / (both + v1_only + v2_only - prod) ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureBinaryGilbert : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Gilbert" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double N(both + v1_only + v2_only + neither) ;
	    double total1((both+v1_only)/N) ;
	    double total2((both+v2_only)/N) ;
	    double prod(total1*total2) ;
	    double match(both/N) ;
	    return (match - prod) / (total1 + total2 - match - prod) ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureGini : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Gini" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT N(both + v1_only + v2_only) ;
	    both /= N ;
	    v1_only /= N ;
	    v2_only /= N ;
	    ValT total1(both+v1_only) ;
	    ValT total2(both+v2_only) ;
	    return (both - total1*total2) / std::sqrt((1.0 - total1*total1)*(1.0 - total2*total2)) ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureBinaryGini : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Gini" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double N(both + v1_only + v2_only + neither) ;
	    double total1((both+v1_only)/N) ;
	    double total2((both+v2_only)/N) ;
	    double prod(total1*total2) ;
	    return (both/N - prod) / std::sqrt((1.0 - total1*total1)*(1.0 - (total2*total2))) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Harrison (1988)
//  range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureHarrison : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Harrison" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(both + std::max(v1_only,v2_only)) ;
	    // all-zero vectors are defined to be identical
	    return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Harrison (1988)
//  range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureBinaryHarrison : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Harrison" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double denom(both + std::max(v1_only,v2_only)) ;
	    // all-zero vectors are defined to be identical
	    return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  first published in 
//      Jaccard, P. (1912) "The distribution of flora of the alpine
//      zone". New Phytologist 11:37-50.
//  range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureJaccard : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Jaccard" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double either(both + v1_only + v2_only) ;
	    return either ? both / either : 1.0 ; // all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  first published in 
//      Jaccard, P. (1912) "The distribution of flora of the alpine
//      zone". New Phytologist 11:37-50.
//  range: 0 ... 1
//  a / (a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryJaccard : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Jaccard" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    double either(both + only_1) ;
	    return either ? both / either : 1.0 ; // all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  a/(b+c)
// "simba" package calls (a+d)/(b+c) sokal4 from Sokal&Sneath(1963)
// same as Tanimoto from old FramepaC

template <typename IdxT, typename ValT>
class VectorMeasureKulczyinski1 : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Kulczynski1" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double only_1(v1_only+v2_only) ;
	    return only_1 ? both / only_1 : HUGE_VAL ;
	 }
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 / (1.0 + similarity(v1,v2)) ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryKulczyinski1 : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Kulczynski1" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    // both += neither ;   // "simba"s sokal4
	    return only_1 ? both / only_1 : HUGE_VAL ;
	 }
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 / (1.0 + similarity(v1,v2)) ;
	 }
   } ;

//============================================================================
// measure first published in
//    Kulczynski, S. (1927) "Die Planzenassoziationen der Pieninen",
//    Bulletin International de l'Academie Poloaise des Sciences et
//    des Letters, Classe des Sciences Mathematiques et Naturells, B
//    (Sciences Naturelles), Suppl. II: 57-203.  Polish article with
//    German summary.
// range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureKulczyinski2 : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Kulczynski2" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    if (both + v1_only + v2_only == 0)
	       return 1.0 ; // both all-zero vectors are defined to be identical
	    double total1(both + v1_only) ;
	    double total2(both + v2_only) ;
	    if (total1 == 0 || total2 == 0)
	       return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
	    return (both / total1 + both / total2) / 2.0 ;
	 }
   } ;

//============================================================================
// measure first published in
//    Kulczynski, S. (1927) "Die Planzenassoziationen der Pieninen",
//    Bulletin International de l'Academie Poloaise des Sciences et
//    des Letters, Classe des Sciences Mathematiques et Naturells, B
//    (Sciences Naturelles), Suppl. II: 57-203.  Polish article with
//    German summary.
// range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureBinaryKulczyinski2 : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Kulczynski2" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    if (both + v1_only + v2_only == 0)
	       return 1.0 ; // both all-zero vectors are defined to be identical
	    double total1(both + v1_only) ;
	    double total2(both + v2_only) ;
	    if (total1 == 0 || total2 == 0)
	       return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
	    return (both / total1 + both / total2) / 2.0 ;
	 }
   } ;

//============================================================================
//  (b+c) / (2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureLanceWilliams : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Lance-Williams" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(2.0*both + v1_only + v2_only) ;
	    return denom ? (v1_only + v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
// (b+c) / (2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryLanceWilliams : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Lance-Williams" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    double denom(2.0*both + only_1) ;
	    return denom ? only_1 / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lande (1996)
// (b+c)/2

template <typename IdxT, typename ValT>
class VectorMeasureLande : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Lande" ; }
      virtual double scoreContingencyTable(ValT /*both*/, ValT v1_only, ValT v2_only) const
	 {
	    return (v1_only + v2_only) / 2.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lande (1996)
// (b+c)/2

template <typename IdxT, typename ValT>
class VectorMeasureBinaryLande : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Lande" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    return only_1 / 2.0 ;
	 }
      //virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 2
//  lennon = 2|b-c| / (2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureLennon : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Lennon" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT total1(both + v1_only) ;
	    ValT total2(both + v2_only) ;
	    double denom(total1 + total2) ;
	    // all-zero vectors are defined to be identical
	    return denom ? (2.0 * std::abs(v1_only - v2_only) / denom) : 0.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 2

template <typename IdxT, typename ValT>
class VectorMeasureBinaryLennon : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Lennon" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double total1(both + v1_only) ;
	    double total2(both + v2_only) ;
	    double denom(total1 + total2) ;
	    // all-zero vectors are defined to be identical
	    return denom ? (2.0 * std::abs((ssize_t)(v1_only - v2_only)) / denom) : 0.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureLennon2 : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Lennon2" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double sum(both + v1_only + v2_only) ;
	    return sum ? std::log2((both+sum)/sum) : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lennon, J.J, Kileff, P., Greenwood, J.J.D, and
//    Gaston, K.J. (2001) "The geographical structure of British bird
//    distributions: diversity, spatial turnover an scale".  J Anim
//    Ecology 70:966-979.
//  range: 0 ... 1
// log2((2a+b+c)/(a+b+c))

template <typename IdxT, typename ValT>
class VectorMeasureBinaryLennon2 : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Lennon2" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    double sum(both + only_1) ;
	    return sum ? std::log2((both+sum)/sum) : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Legendre & Legendre (1998)
//  range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureLegendre : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Legendre" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    both *= 3 ;
	    double denom(both + v1_only + v2_only) ;
	    return denom ? both / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Legendre & Legendre (1998)
//  range: 0 ... 1
// leg2 = 3a / (3a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryLegendre : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Legendre" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    both *= 3 ;
	    double denom(both + only_1) ;
	    return denom ? both / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     van der Maarel (1969)
// range: -1 ... 1
// maarel = (2a - b - c) / (2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureMaarel : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Maarel" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    both *= 2 ;
	    double denom(both + v1_only + v2_only) ;
	    return denom ? ((both - v1_only - v2_only) / denom) : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     van der Maarel (1969)
// range: -1 ... 1
// maarel = (2a - b - c) / (2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryMaarel : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Maarel" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    both *= 2 ;
	    double denom(both + only_1) ;
	    return denom ? ((both - only_1) / denom) : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Magurran (1988)
// mag = (2a + b + c) * (1 - (a / (a + b + c)))
//     = N * (1 - Jaccard)

template <typename IdxT, typename ValT>
class VectorMeasureMagurran : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Magurran" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double sum(both + v1_only + v2_only) ;
	    // all-zero vectors are defined to be identical
	    return sum ? ((both + sum) * (1.0 - (both / sum))) : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Magurran (1988)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryMagurran : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Magurran" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    double sum(both + only_1) ;
	    // all-zero vectors are defined to be identical
	    return sum ? ((both + sum) * (1.0 - (both / sum))) : 1.0 ;
	 }
      //virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
   } ;

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Hubalek (1982)
// range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureMcConnagh : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "McConnagh" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT total1(both + v1_only) ;
	    ValT total2(both + v2_only) ;
	    double denom(total1 * total2) ;
	    return denom ? (both * both - v1_only * v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Hubalek (1982)
// range: 0 ... 1
// mcc = (aa - bc) / ((a+b)(a+c))
// aka McConnaughey

template <typename IdxT, typename ValT>
class VectorMeasureBinaryMcConnagh : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary McConnagh" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double total1(both + v1_only) ;
	    double total2(both + v2_only) ;
	    double denom(total1 * total2) ;
	    return denom ? (both * both - v1_only * v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureModGini : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "ModGini" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT N(both + v1_only + v2_only) ;
	    both /= N ;
	    v1_only /= N ;
	    v2_only /= N ;
	    ValT total1(both+v1_only) ;
	    ValT total2(both+v2_only) ;
	    return (both - total1*total2) / (1.0 - std::abs(v1_only-v2_only)/2 - total1*total2) ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureBinaryModGini : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary ModGini" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double N(both + v1_only + v2_only + neither) ;
	    double prod((both+v1_only)/N*(both+v2_only)/N) ;
	    return (both/N - prod) / (1.0 - std::abs((ssize_t)(v1_only-v2_only))/(2.0*N) - prod) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package

template <typename IdxT, typename ValT>
class VectorMeasureMountford : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Mountford" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    if (v1_only + v2_only == 0)
	       return 1.0 ; // vectors with no disagreements are defined to be identical
	    return 2.0 * both / (double)(both*(v1_only + v2_only) + 2.0 * v1_only * v2_only) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package

template <typename IdxT, typename ValT>
class VectorMeasureBinaryMountford : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Mountford" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    if (v1_only + v2_only == 0)
	       return 1.0 ; // vectors with no disagreements are defined to be identical
	    return 2.0 * both / (double)(both*(v1_only + v2_only) + 2.0 * v1_only * v2_only) ;
	 }
   } ;

//============================================================================
// measure first published in
//    Ochiai, A. (1957) "Zoogeographic studies on the soleoid fishes
//    found in Japan and its neighboring regions", Bulletin of the
//    Japanese Society of Scientific Fisheries 22:526-530.  Japanese
//    article with English summary.

template <typename IdxT, typename ValT>
class VectorMeasureOchiai : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Ochiai" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    if (both + v1_only + v2_only == 0)
	       return 1.0 ; // both all-zero vectors are defined to be identical
	    else if (both + v1_only == 0 || both + v2_only == 0)
	       return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
	    return both / std::sqrt((both + v1_only) * (both + v2_only)) ;
	 }
   } ;

//============================================================================
// measure first published in
//    Ochiai, A. (1957) "Zoogeographic studies on the soleoid fishes
//    found in Japan and its neighboring regions", Bulletin of the
//    Japanese Society of Scientific Fisheries 22:526-530.  Japanese
//    article with English summary.

template <typename IdxT, typename ValT>
class VectorMeasureBinaryOchiai : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Ochiai" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    if (both + v1_only + v2_only == 0)
	       return 1.0 ; // both all-zero vectors are defined to be identical
	    else if (both + v1_only == 0 || both + v2_only == 0)
	       return 0.0 ; // single all-zero vector is defined to be maximally DISsimilar
	    return both / std::sqrt((both + v1_only) * (both + v2_only)) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename IdxT, typename ValT>
class VectorMeasureRoutledge1 : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Routledge1" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT sum(both + v1_only + v2_only) ;
	    sum *= sum ;
	    double denom(sum - 2.0 * v1_only * v2_only) ;
	    return denom ? (sum / denom - 1.0) : 1.0 ;// all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryRoutledge1 : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Routledge1" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double sum(both + v1_only + v2_only) ;
	    sum *= sum ;
	    double denom(sum - 2.0 * v1_only * v2_only) ;
	    return denom ? (sum / denom - 1.0) : 1.0 ;// all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename IdxT, typename ValT>
class VectorMeasureRoutledge2 : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Routledge2" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(2.0 * both + v1_only + v2_only) ;
	    if (!denom)
	       return 0.0 ;			// two all-zero vectors are identical
	    ValT total1(both+v1_only) ;
	    ValT total2(both+v2_only) ;
	    double num1(2 * both * log(2)) ;
	    double num2(p_log_p(total1) + p_log_p(total2)) ;
	    return log(denom) - (num1 + num2) / denom ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryRoutledge2 : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Routledge2" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double denom(2.0 * both + v1_only + v2_only) ;
	    if (!denom)
	       return 0.0 ;			// two all-zero vectors are identical
	    size_t total1(both+v1_only) ;
	    size_t total2(both+v2_only) ;
	    double num1(2 * both * log(2)) ;
	    double num2(p_log_p(total1) + p_log_p(total2)) ;
	    return log(denom) - (num1 + num2) / denom ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
//  a / min(a+b,a+c) == a / (a + min(b,c))

template <typename IdxT, typename ValT>
class VectorMeasureSimpson : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Simpson" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(both + std::min(v1_only,v2_only)) ;
	    return denom ? both/denom : 1.0 ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureBinarySimpson : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Simpson" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double denom(both + std::min(v1_only,v2_only)) ;
	    return denom ? both/denom : 1.0 ;
	 }
   } ;

//============================================================================
// https://en.wikipedia.org/wiki/Qualitative_variation says
//  2(a+d) / (2(a+d) + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureSokalSneath : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Sokal-Sneath" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(2*both + v1_only + v2_only) ;
	    return denom ? 2*both / denom : 1.0 ; 	// all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
// https://en.wikipedia.org/wiki/Qualitative_variation says
//  2(a+d) / (2(a+d) + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinarySokalSneath : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Sokal-Sneath" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    both += neither ;
	    both *= 2 ;
	    double denom(both + only_1) ;
	    return denom ? both/denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Sorgenfrei (1959)
// range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureSorgenfrei : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Sorgenfrei" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT total1(both + v1_only) ;
	    ValT total2(both + v2_only) ;
	    double denom(total1 * total2) ;
	    return denom ? (both * both) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Sorgenfrei (1959)
// range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureBinarySorgenfrei : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Sorgenfrei" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double total1(both + v1_only) ;
	    double total2(both + v2_only) ;
	    double denom(total1 * total2) ;
	    return denom ? (both * both) / denom : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureTripartite : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Tripartite" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double min1 = std::min(v1_only,v2_only) ;
	    double max1 = std::max(v1_only,v2_only) ;
	    double U = (both+max1) ? std::log2(1.0 + (both+min1) / (both+max1)) : 1.0 ;
	    double S = 1.0 / std::sqrt(std::log2(2.0 + min1/(both+1.0))) ;
	    double R_1 = (both+v1_only) ? std::log2(1.0 + both / (both+v1_only)) : 1.0 ;
	    double R_2 = (both+v2_only) ? std::log2(1.0 + both / (both+v2_only)) : 1.0 ;
	    double R = R_1 * R_2 ;
	    return std::sqrt(U * S * R) ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryTripartite : public Fr::SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Tripartite" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t neither) const
	 {
	    double min1 = std::min(v1_only,v2_only) ;
	    double max1 = std::max(v1_only,v2_only) ;
	    double U = (both+max1) ? std::log2(1.0 + (both+min1) / (both+max1)) : 1.0 ;
	    double S = 1.0 / std::sqrt(std::log2(2.0 + min1/(both+1.0))) ;
	    double R_1 = (both+v1_only) ? std::log2(1.0 + both / (both+v1_only)) : 1.0 ;
	    double R_2 = (both+v2_only) ? std::log2(1.0 + both / (both+v2_only)) : 1.0 ;
	    double R = R_1 * R_2 ;
	    return std::sqrt(U * S * R) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Whittaker (1960)
//  range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureWhittaker : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Whittaker" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double sum(both + v1_only + v2_only) ;
	    return sum ? (2.0 * sum / (both + sum)) - 1.0 : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Whittaker (1960)
//  range: 0 ... 1
// 2(a + b + c) / (2a + b + c) - 1

template <typename IdxT, typename ValT>
class VectorMeasureBinaryWhittaker : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Whittaker" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    size_t sum(both + only_1) ;
	    return sum ? (2.0 * sum / (both + sum)) - 1.0 : 1.0 ;
	 }
      //virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename IdxT, typename ValT>
class VectorMeasureWilliams : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Williams" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(both + v1_only + v2_only) ;
	    return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename IdxT, typename ValT>
class VectorMeasureBinaryWilliams : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Williams" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double denom(both + v1_only + v2_only) ;
	    return denom ? std::min(v1_only,v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename IdxT, typename ValT>
class VectorMeasureWilliams2 : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Williams2" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT sum(both + v1_only + v2_only) ;
	    double denom(sum*sum - sum) ;
	    return denom ? (2.0 * v1_only * v2_only + 1.0) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename IdxT, typename ValT>
class VectorMeasureBinaryWilliams2 : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Williams2" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double sum(both + v1_only + v2_only) ;
	    double denom(sum*sum - sum) ;
	    return denom ? (2.0 * v1_only * v2_only + 1.0) / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Wilson & Shmida (1984)

template <typename IdxT, typename ValT>
class VectorMeasureWilsonShmida : public Fr::DistanceMeasureCT<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Wilson-Shmida" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(2.0 * both + v1_only + v2_only) ;
	    return denom ? (v1_only + v2_only) / denom : 1.0 ; // all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Wilson & Shmida (1984)
// (b+c)/(2a+b+c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryWilsonShmida : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Binary Wilson-Shmida" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t both, only_1, neither ;
	    binaryAgreement(v1,v2,both,only_1,neither) ;
	    double denom(2.0 * both + only_1) ;
	    return denom ? only_1 / denom : 1.0 ; // all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//   Measures NOT based on a 2x2 contingency table			    //
//============================================================================

//============================================================================
//  sum of absolute value of element-wise difference over sum of absolute values of elements

template <typename IdxT, typename ValT>
class VectorMeasureCanberra : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Canberra" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT absdiff(0) ;
	    ValT sumabs(0) ;
	    for( ; pos1 < elts1 && pos2 < elts2 ; )
	       {
	       auto elt1(v1->elementIndex(pos1)) ;
	       auto elt2(v2->elementIndex(pos2)) ;
	       if (elt1 < elt2)
		  {
		  ValT val1(std::abs(v1->elementValue(pos1++))) ;
		  absdiff += val1 ;
		  sumabs += val1 ;
		  }
	       else if (elt1 > elt2)
		  {
		  ValT val2(std::abs(v2->elementValue(pos2++))) ;
		  absdiff += val2 ;
		  sumabs += val2 ;
		  }
	       else // if (elt1 == elt2)
		  {
		  ValT val1(v1->elementValue(pos1++)) ;
		  ValT val2(v2->elementValue(pos2++)) ;
		  absdiff += std::abs(val1 - val2) ;
		  sumabs += std::abs(val1) + std::abs(val2) ;
		  }
	       }
	    return sumabs > 0 ? absdiff / (double)sumabs : 0.0 ;
	 }
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================
// Soergel: sum of element-wise difference over element-wise maximum
//     sum(abs(P_i-Q_i)) / sum(max(P_i,Q_i))
// from: Monev V., Introduction to Similarity Searching in Chemistry,
//   MATCH Commun. Math. Comput. Chem. 51 pp. 7-38 , 2004

template <typename IdxT, typename ValT>
class VectorMeasureSoergel : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Soergel" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double total_diff(0) ;
	    double total_max(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    for ( ; pos1 < elts1 && pos2 < elts2 ; )
	       {
	       auto elt1(v1->elementIndex(pos1)) ;
	       auto elt2(v2->elementIndex(pos2)) ;
	       if (elt1 < elt2)
		  {
		  auto val1 = v1->elementValue(pos1++) / wt1 ;
		  total_diff += std::abs(val1) ;
		  total_max += val1 ;
		  }
	       else if (elt1 > elt2)
		  {
		  auto val2 = v2->elementValue(pos2++) / wt2 ;
		  total_diff += std::abs(val2) ;
		  total_max += val2 ;
		  }
	       else // if (elt1 == elt2)
		  {
		  auto val1 = v1->elementValue(pos1++) / wt1 ;
		  auto val2 = v2->elementValue(pos2++) / wt2 ;
		  total_diff += std::abs(val1 - val2) ;
		  total_max += std::max(val1 + val2, this->m_opt.smoothing) ;
		  }
	       }   
	    return total_max ? total_diff / total_max : -1.0 ;
	 }
   } ;

//============================================================================
// sum of logarithmic differences
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename IdxT, typename ValT>
class VectorMeasureLorentzian : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Lorentzian" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    for ( ; pos1 < elts1 && pos2 < elts2 ; )
	       {
	       auto elt1(v1->elementIndex(pos1)) ;
	       auto elt2(v2->elementIndex(pos2)) ;
	       if (elt1 < elt2)
		  {
		  sum += std::log(1.0 + std::abs(v1->elementValue(pos1++)/wt1)) ;
		  }
	       else if (elt1 > elt2)
		  {
		  sum += std::log(1.0 + std::abs(v2->elementValue(pos2++)/wt2)) ;
		  }
	       else // if (elt1 == elt2)
		  {
		  auto val1 = v1->elementValue(pos1++) / wt1 ;
		  auto val2 = v2->elementValue(pos2++) / wt2 ;
		  sum += std::log(1.0 + std::abs(val1 - val2)) ;
		  }
	       }
	    return sum ;
	 }
   } ;

//============================================================================
// Fidelity, aka Bhattacharyya coefficient or Hellinger affinity
//     sum(sqrt(P*Q))

template <typename IdxT, typename ValT>
class VectorMeasureFidelity : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Fidelity" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
   } ;

//============================================================================
// Bhattacharyya distance
//    -ln(fidelity)

template <typename IdxT, typename ValT>
class VectorMeasureBhattacharyya : public VectorMeasureFidelity<IdxT,ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Bhattacharyya" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    double fidelity = VectorMeasureFidelity<IdxT,ValT>::sparseDistance(v1,v2) ;
	    return fidelity > 0 ? -std::log(fidelity) : HUGE_VAL ;
	 }
   } ;
      
//============================================================================
// Hellinger distance
//    2*sqrt(1-fidelity)

template <typename IdxT, typename ValT>
class VectorMeasureHellinger : public VectorMeasureFidelity<IdxT,ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Hellinger" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    double fidelity = VectorMeasureFidelity<IdxT,ValT>::sparseDistance(v1,v2) ;
	    return 2 * std::sqrt(1 - fidelity) ;
	 }
   } ;
      
//============================================================================
// Matusita distance
//    sqrt(2 - 2*fidelity)

template <typename IdxT, typename ValT>
class VectorMeasureMatusita : public VectorMeasureFidelity<IdxT,ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Matusita" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    double fidelity = VectorMeasureFidelity<IdxT,ValT>::sparseDistance(v1,v2) ;
	    return std::sqrt(2 - 2 * fidelity) ;
	 }
   } ;

//============================================================================
// squared-chord distance
//	sum((sqrt(P_i) - sqrt(Q_i))^2)
// note: required non-negative elements!
// from: Gavin D.G., Oswald W.W., Wahl, E.R., and Williams J.W., A statistical
//   approach to evaluating distance metrics and analog assignments for pollen
//   records, Quaternary Research 60, pp 356–367, 2003

template <typename IdxT, typename ValT>
class VectorMeasureSquaredChord : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Squared-Chord" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
   } ;

//============================================================================
// Clark
//     sqrt(sum((absdiff/sum)**2))
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename IdxT, typename ValT>
class VectorMeasureClark : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Clark" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
   } ;

//============================================================================
// Wave Hedges
//     sum(1 - (min/max)) == sum(absdiff / max)

template <typename IdxT, typename ValT>
class VectorMeasureWaveHedges : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Wave-Hedges" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    for ( ; pos1 < elts1 && pos2 < elts2 ; )
	       {
	       auto elt1(v1->elementIndex(pos1)) ;
	       auto elt2(v2->elementIndex(pos2)) ;
	       double val1(0) ;
	       double val2(0) ;
	       if (elt1 < elt2)
		  {
		  val1 = v1->elementValue(pos1++) / wt1 ;
		  }
	       else if (elt1 > elt2)
		  {
		  val2 = v2->elementValue(pos2++) / wt2 ;
		  }
	       else // if (elt1 == elt2)
		  {
		  val1 = v1->elementValue(pos1++) / wt1 ;
		  val2 = v2->elementValue(pos2++) / wt2 ;
		  }
	       double diff = std::abs(val1 - val2) ;
	       double maxval = std::max(val1,val2,this->m_opt.smoothing) ;
	       sum += safe_div(diff,maxval) ;
	       }
	    return sum ;
	 }
   } ;

//============================================================================
// probabilistic symmetric chi-squared / Sangvi chi-squared between populations
//     2*sum((diff**2)/(sum))
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename IdxT, typename ValT>
class VectorMeasureSangvi : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Sangvi" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
		  double elt_sum = std::max(val1 + val2, this->m_opt.smoothing) ;
		  sum += safe_div(diff * diff, elt_sum) ;
		  }
	       }
	    return std::sqrt(sum) ;
	 }
   } ;

//============================================================================
// Taneja: hybrid of arithmetic and geometric mean divergence

template <typename IdxT, typename ValT>
class VectorMeasureTaneja : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Taneja" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    for ( ; pos1 < elts1 && pos2 < elts2 ; )
	       {
	       auto elt1(v1->elementIndex(pos1)) ;
	       auto elt2(v2->elementIndex(pos2)) ;
	       double val1(0) ;
	       double val2(0) ;
	       if (elt1 < elt2)
		  {
		  val1 = v1->elementValue(pos1++)/ wt1 ;
		  }
	       else if (elt1 > elt2)
		  {
		  val2 = v2->elementValue(pos2++)/ wt2 ;
		  }
	       else // if (elt1 == elt2)
		  {
		  val1 = v1->elementValue(pos1++)/ wt1 ;
		  val2 = v2->elementValue(pos2++)/ wt2 ;
		  }
	       double mean = (val1 + val2) / 2 ;
	       double denom = std::max(2*std::sqrt(val1*val2), this->m_opt.smoothing) ;
	       double geom = safe_div(val1 + val2, denom) ;
	       sum += mean * std::log(geom) ;
	       }
	    return sum ;
	 }
   } ;

//============================================================================
// Kumar-Johnson: hybrid of symmetric chi-squared, arithmetic and geometric mean divergence

template <typename IdxT, typename ValT>
class VectorMeasureKumarJohnson : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Kumar-Johnson" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    for ( ; pos1 < elts1 && pos2 < elts2 ; )
	       {
	       auto elt1(v1->elementIndex(pos1)) ;
	       auto elt2(v2->elementIndex(pos2)) ;
	       double val1(0) ;
	       double val2(0) ;
	       if (elt1 < elt2)
		  {
		  val1 = v1->elementValue(pos1++)/ wt1 ;
		  }
	       else if (elt1 > elt2)
		  {
		  val2 = v2->elementValue(pos2++)/ wt2 ;
		  }
	       else // if (elt1 == elt2)
		  {
		  val1 = v1->elementValue(pos1++)/ wt1 ;
		  val2 = v2->elementValue(pos2++)/ wt2 ;
		  }
	       double num = (val1 * val1 - val2 * val2) ;
	       double denom = std::max(2 * std::pow(val1 * val2, 1.5), this->m_opt.smoothing) ;
	       sum += safe_div(num,denom) ;
	       }
	    return sum ;
	 }
   } ;

//============================================================================
// normalized sum of element-wise minimums

template <typename IdxT, typename ValT>
class VectorMeasureCircleProduct : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "CircleProduct" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
	    double sum(0.0) ;
	    if (this->m_opt.normalize)
	       {
	       ValT wt1, wt2 ;
	       normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureJaro : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Jaro" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureJaroWinkler : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Jaro-Winkler" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    // for strings s1 and s2:
	    // d = jaro_distance(s1,s2) + (prefixlen*scale_factor*(1 - jaro_distance(s1,s2))) ;
	    // where prefixlen=min(|common_prefix(s1,s2)|,4) ;
	    // scale_factor <= 0.25; standard value 0.1
	    // see: http://web.archive.org/web/20100227020019/http://www.census.gov/geo/msb/stand/strcmp.c
	    return 0.0 ; //FIXME
	 }
   } ;

//============================================================================
// Jensen difference: sum of average of entropies less entropy of average

template <typename IdxT, typename ValT>
class VectorMeasureJensen : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Jensen" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    ValT totalwt1, totalwt2 ;
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
		  val1 = v1->elementValue(pos1++) / totalwt1 ;
		  }
	       else if (elt1 > elt2)
		  {
		  val2 = v2->elementValue(pos2++) / totalwt2 ;
		  }
	       else // if (elt1 == elt2)
		  {
		  val1 = v1->elementValue(pos1++) / totalwt1 ;
		  val2 = v2->elementValue(pos2++) / totalwt2 ;
		  }
	       double ent1 = p_log_p(val1) ;
	       double ent2 = p_log_p(val2) ;
	       double ent_avg = p_log_p((val1+val2)/2) ;
	       sum += (ent1+ent2)/2 - ent_avg ;
	       }
	    return sum ;
	 }
   } ;

//============================================================================
//      JS(a,b) == (KL(a|avg(a,b)) + KL(b|avg(a,b))) / 2
//      KL(a|b) == sum_y( a(y)(log a(y) - log b(y)) )
// JS == 1/2 * Topsoe distance

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

template <typename IdxT, typename ValT>
class VectorMeasureJensenShannon : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Jensen-Shannon" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    ValT totalwt1, totalwt2 ;
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
		  sum += JS_term(val1,static_cast<ValT>(0)) ;
		  }
	       else if (elt1 > elt2)
		  {
		  double val2(v2->elementValue(pos2++) / totalwt2) ;
		  sum += JS_term(static_cast<ValT>(0),val2) ;
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureKullbackLeibler : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Kullback-Leibler" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT totalwt1, totalwt2 ;
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureL0Norm : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      VectorMeasureL0Norm(const VectorSimilarityOptions &opt) : Fr::DistanceMeasure<IdxT,ValT>(opt) {}
      virtual const char* canonicalName() const { return "L0-Norm" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================
// L-norm for L(infinity) = maximum elementwise difference
// aka Chebyshev

template <typename IdxT, typename ValT>
class VectorMeasureLinfNorm : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Linf-Norm" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT maxdiff(0) ;
	    for ( ; pos1 < elts1 && pos2 < elts2 ; )
	       {
	       auto elt1(v1->elementIndex(pos1)) ;
	       auto elt2(v2->elementIndex(pos2)) ;
	       if (elt1 < elt2)
		  {
		  ValT val1(std::abs(v1->elementValue(pos1++))) ;
		  if (val1 > maxdiff) maxdiff = val1 ;
		  }
	       else if (elt1 > elt2)
		  {
		  ValT val2(std::abs(v2->elementValue(pos2++))) ;
		  if (val2 > maxdiff) maxdiff = val2 ;
		  }
	       else // if (elt1 == elt2)
		  {
		  ValT dif(std::abs(v1->elementValue(pos1++) - v2->elementValue(pos2++))) ;
		  if (dif > maxdiff) maxdiff = dif ;
		  }
	       }
	    return maxdiff ;
	 }
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureMahalanobis : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Mahalanobis" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureManhattan : public Fr::DistanceMeasureReciprocal<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Manhattan" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT sum(0) ;
	    if (this->m_opt.normalize)
	       {
	       ValT wt1, wt2 ;
	       normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    double sum(0.0) ;
	    size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
	    if (this->m_opt.normalize)
	       {
	       ValT wt1, wt2 ;
	       normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
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
   } ;

//============================================================================
// this is the measure from the original FramepaC, and differs from that
// described in
//    Robinson, W.S. 1951 A method for chronologically ordering
//    archaeological deposits. American Antiquity, 16: 293-301.

template <typename IdxT, typename ValT>
class VectorMeasureRobinson : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Robinson" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    double sum(0) ;
	    ValT totalwt1, totalwt2 ;
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureSimilarityRatio : public Fr::SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "SimilarityRatio" ; }
      virtual double sparseSimilarity(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    double sumsq(0) ;
	    double prod(0) ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->opt.normalize,wt1,wt2) ;
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
      virtual double denseSimilarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    double sumsq(0) ;
	    double prod(0) ;
	    size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
	    if (this->opt.normalize)
	       {
	       ValT wt1, wt2 ;
	       normalization_weights(v1,v2,this->opt.normalize,wt1,wt2) ;
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureSquaredEuclidean : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "SquaredEuclidean" ; }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
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
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureEuclidean : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "Euclidean" ; }
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    VectorMeasureSquaredEuclidean<IdxT,ValT> vm ;
	    return std::sqrt(vm.distance(v1,v2)) ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureLNorm : public Fr::DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual const char* canonicalName() const { return "L-Norm" ; }
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    double power(this->m_opt.power) ;
	    if (power <= 0.0)
	       {
	       VectorMeasureL0Norm<IdxT,ValT> vm(this->m_opt) ;
	       return vm.distance(v1,v2) ;
	       }
	    else if (power == 1.0)
	       {
	       VectorMeasureManhattan<IdxT,ValT> vm(this->m_opt) ;
	       return vm.distance(v1,v2) ;
	       }
	    else if (power == 2.0)
	       {
	       VectorMeasureEuclidean<IdxT,ValT> vm(this->m_opt) ;
	       return vm.distance(v1,v2) ;
	       }
	    if (!v1 || !v2) return -1;
	    if (v1->isSparseVector() || v2->isSparseVector())
	       return sparseDistance(v1,v2) ;
	    else
	       return denseDistance(v1,v2) ;
	 }
      virtual double sparseDistance(const SparseVector<IdxT,ValT>* v1, const SparseVector<IdxT,ValT>* v2) const
	 {
	    double power(this->m_opt.power) ;
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
      virtual double denseDistance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    double power(this->m_opt.power) ;
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
   } ;

//============================================================================


} // end namespace FramepaC::vecsim
} // end namespace FramepaC

/************************************************************************/
/************************************************************************/

namespace Fr
{



} // end namespace Fr

// end of file vecsim.cc //
