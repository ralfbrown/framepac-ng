/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.11, last edit 2018-09-06					*/
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

// forward declarations

template <typename IdxT, typename ValT> class VectorMeasureLNorm ;

//----------------------------------------------------------------------------

double safe_div(double num, double denom)
{
   return denom ? num / denom : (num ? HUGE_VAL : 0) ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ValT sum_of_weights(const Vector<IdxT,ValT>* v)
{
   ValT sum(0) ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      sum += abs_value(v->elementValue(i)) ;
   // since we use the returned value as a divisor, force it to be nonzero
   // a zero sum means we'll be dividing only zeros by the return
   //   value, so we can pick anything without changing the results; a
   //   value of one is convenient
   return sum ? sum : 1 ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
ValT max_weight(const Vector<IdxT,ValT>* v)
{
   ValT max(0) ;
   for (size_t i = 0 ; i < v->numElements() ; ++i)
      {
      ValT val = abs_value(v->elementValue(i)) ;
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

template <typename IdxT, typename ValT>
void normalization_weights(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2, int normalization,
			   ValT &wt1, ValT &wt2)
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
class VectorMeasureCosine : public SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual double similarity(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    double prod_lengths((v1->length() * v2->length())) ;
	    if (!prod_lengths)
	       return 0.0 ;
	    ValT dotprod(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       const SparseVector<IdxT,ValT>* sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       const SparseVector<IdxT,ValT>* sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       size_t pos1(0) ;
	       size_t pos2(0) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       }
	    else
	       {
	       // with two dense vectors we can be more efficient
	       size_t len(std::min(elts1,elts2)) ;
	       for (size_t i = 0 ; i < len ; ++i)
		  {
		  dotprod += (v1->elementValue(i) * v2->elementValue(i)) ;
		  }
	       }
	    return dotprod / prod_lengths ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Cosine" ; }
   } ;
      
//============================================================================
//   Measures NOT based on a 2x2 contingency table			    //
//============================================================================

//============================================================================
//  sum of absolute value of element-wise difference over sum of absolute values of elements

template <typename IdxT, typename ValT>
class VectorMeasureCanberra : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    ValT absdiff(0) ;
	    ValT sumabs(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     ValT val1(abs_value(v1->elementValue(pos1++))) ;
		     absdiff += val1 ;
		     sumabs += val1 ;
		     }
		  else if (elt1 > elt2)
		     {
		     ValT val2(abs_value(v2->elementValue(pos2++))) ;
		     absdiff += val2 ;
		     sumabs += val2 ;
		     }
		  else // if (elt1 == elt2)
		     {
		     ValT val1(v1->elementValue(pos1++)) ;
		     ValT val2(v2->elementValue(pos2++)) ;
		     absdiff += abs_value(val1 - val2) ;
		     sumabs += abs_value(val1) + abs_value(val2) ;
		     }
		  }
	       }
	    else
	       {
	       // with two dense vectors we can be more efficient
	       size_t minlen(std::min(elts1,elts2)) ;
	       for (; pos1 < minlen ; ++pos1)
		  {
		  ValT val1(v1->elementValue(pos1)) ;
		  ValT val2(v2->elementValue(pos1)) ;
		  absdiff += abs_value(val1 - val2) ;
		  sumabs += abs_value(val1) + abs_value(val2) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any leftovers from the first vector
	    while (pos1 < elts1)
	       {
	       absdiff += abs_value(v1->elementValue(pos1++)) ;
	       }
	    // handle any leftovers from the second vector
	    while (pos2<  elts2)
	       {
	       absdiff += abs_value(v2->elementValue(pos2++)) ;
	       }
	    return sumabs > 0 ? absdiff / (double)sumabs : 0.0 ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Canberra" ; }
   } ;

//============================================================================
// Soergel: sum of element-wise difference over element-wise maximum
//     sum(abs(P_i-Q_i)) / sum(max(P_i,Q_i))
// from: Monev V., Introduction to Similarity Searching in Chemistry,
//   MATCH Commun. Math. Comput. Chem. 51 pp. 7-38 , 2004

template <typename IdxT, typename ValT>
class VectorMeasureSoergel : public DistanceMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Soergel" ; }
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double total_diff(0) ;
	    double total_max(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     auto val1 = v1->elementValue(pos1++) / wt1 ;
		     total_diff += abs_value(val1) ;
		     total_max += val1 ;
		     }
		  else if (elt1 > elt2)
		     {
		     auto val2 = v2->elementValue(pos2++) / wt2 ;
		     total_diff += abs_value(val2) ;
		     total_max += val2 ;
		     }
		  else // if (elt1 == elt2)
		     {
		     auto val1 = v1->elementValue(pos1++) / wt1 ;
		     auto val2 = v2->elementValue(pos2++) / wt2 ;
		     total_diff += abs_value(val1 - val2) ;
		     total_max += std::max((double)(val1 + val2), this->m_opt.smoothing) ;
		     }
		  }   
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  auto val1 = v1->elementValue(pos1) / wt1 ;
		  auto val2 = v2->elementValue(pos1) / wt2 ;
		  total_diff += abs_value(val1 - val2) ;
		  total_max += std::max((double)(val1 + val2), this->m_opt.smoothing) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any left-over elements of the first vector
	    while (pos1 < elts1)
	       {
	       auto val1 = v1->elementValue(pos1++) / wt1 ;
	       total_diff += abs_value(val1) ;
	       total_max += val1 ;
	       }
	    // handle any left-over elements of the second vector
	    while (pos2 < elts2)
	       {
	       auto val2 = v2->elementValue(pos2++) / wt2 ;
	       total_diff += abs_value(val2) ;
	       total_max += val2 ;
	       }
	    return total_max ? total_diff / total_max : -1.0 ;
	 }
   } ;

//============================================================================
// sum of logarithmic differences
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename IdxT, typename ValT>
class VectorMeasureLorentzian : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     sum += std::log(1.0 + abs_value(v1->elementValue(pos1++)/wt1)) ;
		     }
		  else if (elt1 > elt2)
		     {
		     sum += std::log(1.0 + abs_value(v2->elementValue(pos2++)/wt2)) ;
		     }
		  else // if (elt1 == elt2)
		     {
		     auto val1 = v1->elementValue(pos1++) / wt1 ;
		     auto val2 = v2->elementValue(pos2++) / wt2 ;
		     sum += std::log(1.0 + abs_value(val1 - val2)) ;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  auto val1 = v1->elementValue(pos1) / wt1 ;
		  auto val2 = v2->elementValue(pos1) / wt2 ;
		  sum += std::log(1.0 + abs_value(val1 - val2)) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle left-over elements of the first vector
	    while (pos1 < elts1)
	       {
	       sum += std::log(1.0 + abs_value(v1->elementValue(pos1++)/wt1)) ;
	       }
	    // handle left-over elements from the second vector
	    while (pos2 < elts2)
	       {
	       sum += std::log(1.0 + abs_value(v2->elementValue(pos2++)/wt2)) ;
	       }
	    return sum ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Lorentzian" ; }
   } ;

//============================================================================
// Fidelity, aka Bhattacharyya coefficient or Hellinger affinity
//     sum(sqrt(P*Q))

template <typename IdxT, typename ValT>
class VectorMeasureFidelity : public SimilarityMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Fidelity" ; }
      virtual double similarity(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       }
	    else
	       {
	       // two dense vectors allows us to be more efficient
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  auto val1 = v1->elementValue(pos1++) / wt1 ;
		  auto val2 = v2->elementValue(pos1++) / wt2 ;
		  sum += std::sqrt(val1 * val2) ;
		  }
	       }
	    // no need to handle left-over elements, because unmatched elements do not contribute to the score
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
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    double fidelity = VectorMeasureFidelity<IdxT,ValT>::distance(v1,v2) ;
	    return fidelity > 0 ? -std::log(fidelity) : HUGE_VAL ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Bhattacharyya" ; }
   } ;
      
//============================================================================
// Hellinger distance
//    2*sqrt(1-fidelity)

template <typename IdxT, typename ValT>
class VectorMeasureHellinger : public VectorMeasureFidelity<IdxT,ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    double fidelity = VectorMeasureFidelity<IdxT,ValT>::distance(v1,v2) ;
	    return 2 * std::sqrt(1 - fidelity) ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Hellinger" ; }
   } ;
      
//============================================================================
// Matusita distance
//    sqrt(2 - 2*fidelity)

template <typename IdxT, typename ValT>
class VectorMeasureMatusita : public VectorMeasureFidelity<IdxT,ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    double fidelity = VectorMeasureFidelity<IdxT,ValT>::distance(v1,v2) ;
	    return std::sqrt(2 - 2 * fidelity) ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Matusita" ; }
   } ;

//============================================================================
// squared-chord distance
//	sum((sqrt(P_i) - sqrt(Q_i))^2)
// note: required non-negative elements!
// from: Gavin D.G., Oswald W.W., Wahl, E.R., and Williams J.W., A statistical
//   approach to evaluating distance metrics and analog assignments for pollen
//   records, Quaternary Research 60, pp 356–367, 2003

template <typename IdxT, typename ValT>
class VectorMeasureSquaredChord : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     sum += v1->elementValue(pos1++) / wt1 ;
		     }
		  else if (elt1 > elt2)
		     {
		     sum += v2->elementValue(pos2++) / wt2 ;
		     }
		  else // if (elt1 == elt2)
		     {
		     auto val1 = v1->elementValue(pos1++) / wt1 ;
		     auto val2 = v2->elementValue(pos2++) / wt2 ;
		     double value = std::sqrt(val1) - std::sqrt(val2) ;
		     sum += (value * value) ;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  auto val1 = v1->elementValue(pos1++) / wt1 ;
		  auto val2 = v2->elementValue(pos2++) / wt2 ;
		  double value = std::sqrt(val1) - std::sqrt(val2) ;
		  sum += (value * value) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any left-over elements from the first vector
	    while (pos1 < elts1)
	       {
	       sum += v1->elementValue(pos1++) / wt1 ;
	       }
	    // handle any left-over elements from the second vector
	    while (pos2 < elts2)
	       {
	       sum += v2->elementValue(pos2++) / wt2 ;
	       }
	    return sum ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Squared-Chord" ; }
   } ;

//============================================================================
// Clark
//     sqrt(sum((absdiff/sum)**2))
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename IdxT, typename ValT>
class VectorMeasureClark : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
		     double value = abs_value(val1-val2) / (val1 + val2) ;
		     sum += (value * value) ;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  auto val1 = v1->elementValue(pos1) / wt1 ;
		  auto val2 = v2->elementValue(pos1) / wt2 ;
		  double value = abs_value(val1-val2) / (val1 + val2) ;
		  sum += (value * value) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle remaining elements of first vector
	    // since these add exactly 1.0 each, just add the number of remaining elements
	    sum += (elts1 - pos1) ;
	    // same for the second vector
	    sum += (elts2 - pos2) ;
	    return std::sqrt(sum) ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Clark" ; }
   } ;

//============================================================================
// Wave Hedges
//     sum(1 - (min/max)) == sum(absdiff / max)

template <typename IdxT, typename ValT>
class VectorMeasureWaveHedges : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
		  double diff = abs_value(val1 - val2) ;
		  double bigger = std::max(val1,val2) ;
		  double maxval = std::max(bigger,this->m_opt.smoothing) ;
		  sum += safe_div(diff,maxval) ;
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  double val1 = v1->elementValue(pos1) / wt1 ;
		  double val2 = v2->elementValue(pos1) / wt2 ;
		  double diff = abs_value(val1 - val2) ;
		  double bigger = std::max(val1,val2) ;
		  double maxval = std::max(bigger,this->m_opt.smoothing) ;
		  sum += safe_div(diff,maxval) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle left-over elements of the first vector
	    while (pos1 < elts1)
	       {
	       double val1 = v1->elementValue(pos1++) / wt1 ;
	       double diff = abs_value(val1) ;
	       double maxval = std::max(val1,this->m_opt.smoothing) ;
	       sum += safe_div(diff,maxval) ;
	       }
	    // handle left-over elements of the second vector
	    while (pos2 < elts2)
	       {
	       double val2 = v2->elementValue(pos2++) / wt2 ;
	       double diff = abs_value(val2) ;
	       double maxval = std::max(val2,this->m_opt.smoothing) ;
	       sum += safe_div(diff,maxval) ;
	       }
	    return sum ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Wave-Hedges" ; }
   } ;

//============================================================================
// probabilistic symmetric chi-squared / Sangvi chi-squared between populations
//     2*sum((diff**2)/(sum))
// from: Deza E. and Deza M.M., Dictionary of Distances, Elsevier, 2006

template <typename IdxT, typename ValT>
class VectorMeasureSangvi : public DistanceMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Sangvi" ; }
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     sum += abs_value(v1->elementValue(pos1++)) / wt1 ;
		     }
		  else if (elt1 > elt2)
		     {
		     sum += abs_value(v2->elementValue(pos2++)) / wt2 ;
		     }
		  else // if (elt1 == elt2)
		     {
		     auto val1 = v1->elementValue(pos1++) / wt1 ;
		     auto val2 = v2->elementValue(pos2++) / wt2 ;
		     double diff = val1 - val2 ;
		     double elt_sum = std::max((double)(val1 + val2), this->m_opt.smoothing) ;
		     sum += safe_div(diff * diff, elt_sum) ;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  auto val1 = v1->elementValue(pos1++) / wt1 ;
		  auto val2 = v2->elementValue(pos2++) / wt2 ;
		  double diff = val1 - val2 ;
		  double elt_sum = std::max((double)(val1 + val2), this->m_opt.smoothing) ;
		  sum += safe_div(diff * diff, elt_sum) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any left-over elements of the first vector
	    while (pos1 < elts1)
	       {
	       sum += abs_value(v1->elementValue(pos1++)) / wt1 ;
	       }
	    // handle any left-over elements of the second vector
	    while (pos2 < elts2)
	       {
	       sum += abs_value(v2->elementValue(pos2++)) / wt2 ;
	       }
	    return std::sqrt(sum) ;
	 }
   } ;

//============================================================================
// Taneja: hybrid of arithmetic and geometric mean divergence

template <typename IdxT, typename ValT>
class VectorMeasureTaneja : public DistanceMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Taneja" ; }
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  double val1 = v1->elementValue(pos1++)/ wt1 ;
		  double val2 = v2->elementValue(pos2++)/ wt2 ;
		  double mean = (val1 + val2) / 2 ;
		  double denom = std::max(2*std::sqrt(val1*val2), this->m_opt.smoothing) ;
		  double geom = safe_div(val1 + val2, denom) ;
		  sum += mean * std::log(geom) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any left-over elements from the first vector
	    while (pos1 < elts1)
	       {
	       double val1 = v1->elementValue(pos1++)/ wt1 ;
	       double mean = (val1) / 2 ;
	       double denom = std::max(0.0, this->m_opt.smoothing) ;
	       double geom = safe_div(val1, denom) ;
	       sum += mean * std::log(geom) ;
	       }
	    // handle any left-over elements from the second vector
	    while (pos2 < elts2)
	       {
	       double val2 = v2->elementValue(pos2++)/ wt2 ;
	       double mean = (val2) / 2 ;
	       double denom = std::max(0.0, this->m_opt.smoothing) ;
	       double geom = safe_div(val2, denom) ;
	       sum += mean * std::log(geom) ;
	       }
	    return sum ;
	 }
   } ;

//============================================================================
// Kumar-Johnson: hybrid of symmetric chi-squared, arithmetic and geometric mean divergence

template <typename IdxT, typename ValT>
class VectorMeasureKumarJohnson : public DistanceMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Kumar-Johnson" ; }
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    ValT wt1, wt2 ;
	    normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  double val1 = v1->elementValue(pos1)/ wt1 ;
		  double val2 = v2->elementValue(pos1)/ wt2 ;
		  double num = (val1 * val1 - val2 * val2) ;
		  double denom = std::max(2 * std::pow(val1 * val2, 1.5), this->m_opt.smoothing) ;
		  sum += safe_div(num,denom) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any left-over elements of the first vector
	    while (pos1 < elts1)
	       {
	       double val1 = v1->elementValue(pos1++)/ wt1 ;
	       double num = (val1 * val1) ;
	       double denom = std::max(0.0, this->m_opt.smoothing) ;
	       sum += safe_div(num,denom) ;
	       }
	    // handle any left-over elements of the second vector
	    while (pos2 < elts2)
	       {
	       double val2 = v2->elementValue(pos2++)/ wt2 ;
	       double num = ( - val2 * val2) ;
	       double denom = std::max(0.0, this->m_opt.smoothing) ;
	       sum += safe_div(num,denom) ;
	       }
	    return sum ;
	 }
   } ;

//============================================================================
// normalized sum of element-wise minimums

template <typename IdxT, typename ValT>
class VectorMeasureCircleProduct : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       ValT wt1, wt2 ;
	       normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       }
	    else
	       {
	       // with two dense vectors we can be more efficient
	       size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
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
	       }
	    // no need to handle leftovers; since we assume elements are always non-negative,
	    //   the minimums for the "missing" element comparisons will always be zero
	    size_t total(elts1 + elts2) ;
	    return total ? sum / total : 0.0 ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "CircleProduct" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureJaro : public SimilarityMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Jaro" ; }
      virtual double similarity(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
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
class VectorMeasureJaroWinkler : public SimilarityMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Jaro-Winkler" ; }
      virtual double similarity(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
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
class VectorMeasureJensen : public DistanceMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Jensen" ; }
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    ValT totalwt1, totalwt2 ;
	    normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0.0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
		  double ent1 = this->p_log_p(val1) ;
		  double ent2 = this->p_log_p(val2) ;
		  double ent_avg = this->p_log_p((val1+val2)/2) ;
		  sum += (ent1+ent2)/2 - ent_avg ;
		  }
	       }
	    else
	       {
	       // with two dense vectors we can be more efficient
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1 )
		  {
		  double val1 = v1->elementValue(pos1) / totalwt1 ;
		  double val2 = v2->elementValue(pos1) / totalwt2 ;
		  double ent1 = this->p_log_p(val1) ;
		  double ent2 = this->p_log_p(val2) ;
		  double ent_avg = this->p_log_p((val1+val2)/2) ;
		  sum += (ent1+ent2)/2 - ent_avg ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle leftover elements in the first vector (if second is shorter)
	    while (pos1 < elts1)
	       {
	       double val1(v1->elementValue(pos1++)/totalwt1) ;
	       double ent1 = this->p_log_p(val1) ;
	       double ent_avg = this->p_log_p(val1/2) ;
	       sum += (ent1)/2 - ent_avg ;
	       }
	    // handle leftover elements in the second vector (if first is shorter)
	    while (pos2 < elts2)
	       {
	       double val2(v2->elementValue(pos2++)/totalwt2) ;
	       double ent2 = this->p_log_p(val2) ;
	       double ent_avg = this->p_log_p(val2/2) ;
	       sum += (ent2)/2 - ent_avg ;
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
class VectorMeasureJensenShannon : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    ValT totalwt1, totalwt2 ;
	    normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0.0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     double val1(v1->elementValue(pos1++) / totalwt1) ;
		     sum += JS_term(val1,0.0) ;
		     }
		  else if (elt1 > elt2)
		     {
		     double val2(v2->elementValue(pos2++) / totalwt2) ;
		     sum += JS_term(0.0,val2) ;
		     }
		  else // if (elt1 == elt2)
		     {
		     double val1(v1->elementValue(pos1++) / totalwt1) ;
		     double val2(v2->elementValue(pos2++) / totalwt2) ;
		     sum += JS_term(val1,val2) ;
		     }
		  }
	       }
	    else
	       {
	       // with two dense vectors we can be more efficient
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1 )
		  {
		  ValT val1(v1->elementValue(pos1)/totalwt1) ;
		  ValT val2(v2->elementValue(pos1)/totalwt2) ;
		  sum += JS_term(val1,val2) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle leftover elements in the first vector (if second is shorter)
	    while (pos1 < elts1)
	       {
	       ValT val1(v1->elementValue(pos1++)/totalwt1) ;
	       sum += JS_term(val1,(ValT)0) ;
	       }
	    // handle leftover elements in the second vector (if first is shorter)
	    while (pos2 < elts2)
	       {
	       ValT val2(v2->elementValue(pos2++)/totalwt2) ;
	       sum += JS_term((ValT)0,val2) ;
	       }
	    return sum / 2.0 ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Jensen-Shannon" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureKullbackLeibler : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT totalwt1, totalwt2 ;
	    normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
	    double sum(0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     auto val1(v1->elementValue(pos1++)/totalwt1) ;
		     sum += KL_term(val1,(ValT)0) ;
		     }
		  else if (elt1 > elt2)
		     {
		     auto val2(v2->elementValue(pos2++)/totalwt2) ;
		     sum += KL_term((ValT)0,val2) ;
		     }
		  else // if (elt1 == elt2)
		     {
		     auto val1(v1->elementValue(pos1++)/totalwt1) ;
		     auto val2(v2->elementValue(pos2++)/totalwt2) ;
		     sum += KL_term(val1,val2) ;
		     }
		  }
	       }
	    else
	       {
	       // with two dense vectors we can be more efficient
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  ValT val1(v1->elementValue(pos1)/totalwt1) ;
		  ValT val2(v2->elementValue(pos1)/totalwt2) ;
		  sum += KL_term(val1,val2) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle leftover elements in the first vector (if second is shorter)
	    while (pos1 < elts1)
	       {
	       ValT val1(v1->elementValue(pos1++)/totalwt1) ;
	       sum += KL_term(val1,(ValT)0) ;
	       }
	    // handle leftover elements in the second vector (if first is shorter)
	    while (pos2 < elts2)
	       {
	       ValT val2(v2->elementValue(pos2++)/totalwt2) ;
	       sum += KL_term((ValT)0,val2) ;
	       }
	    return sum ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Kullback-Leibler" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureL0Norm : public DistanceMeasure<IdxT, ValT>
   {
   public:
      VectorMeasureL0Norm() : DistanceMeasure<IdxT,ValT>() {}
      VectorMeasureL0Norm(const VectorSimilarityOptions &opt) : Fr::DistanceMeasure<IdxT,ValT>(opt) {}
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    // count the non-zero elements of the difference between the two vectors
	    size_t differences(0) ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       }
	    else
	       {
	       size_t minlen = std::min(elts1,elts2) ;
	       for (; pos1 < minlen ; ++pos1)
		  {
		  if (v1->elementValue(pos1) != v2->elementValue(pos1)) ++differences ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle left-over elements of the first vector
	    while (pos1 < elts1)
	       {
	       if (v1->elementValue(pos1++))
		  ++differences ;
	       }
	    // handle left-over elements of the second vector
	    while (pos2 < elts2)
	       {
	       if (v2->elementValue(pos2++))
		  ++differences ;
	       }
	    size_t total(v1->numElements() + v2->numElements()) ;
	    return total ? differences/total : 0.0 ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "L0-Norm" ; }
   } ;

//============================================================================
// L-norm for L(infinity) = maximum elementwise difference
// aka Chebyshev

template <typename IdxT, typename ValT>
class VectorMeasureLinfNorm : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT maxdiff(0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     ValT val1(abs_value(v1->elementValue(pos1++))) ;
		     if (val1 > maxdiff) maxdiff = val1 ;
		     }
		  else if (elt1 > elt2)
		     {
		     ValT val2(abs_value(v2->elementValue(pos2++))) ;
		     if (val2 > maxdiff) maxdiff = val2 ;
		     }
		  else // if (elt1 == elt2)
		     {
		     ValT dif(abs_value(v1->elementValue(pos1++) - v2->elementValue(pos2++))) ;
		     if (dif > maxdiff) maxdiff = dif ;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  ValT diff = abs_value(v1->elementValue(pos1) - v2->elementValue(pos1)) ;
		  if (diff > maxdiff) maxdiff = diff ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any leftovers from the first vector
	    while (pos1 < elts1)
	       {
	       ValT diff = abs_value(v1->elementValue(pos1++)) ;
	       if (diff > maxdiff) maxdiff = diff ;
	       }
	    // handle any leftovers from the second vector
	    while (pos2 < elts2)
	       {
	       ValT diff = abs_value(v2->elementValue(pos2++)) ;
	       if (diff > maxdiff) maxdiff = diff ;
	       }
	    return maxdiff ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Linf-Norm" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureMahalanobis : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     //FIXME
		     ++pos1 ;
		     }
		  else if (elt1 > elt2)
		     {
		     //FIXME
		     ++pos2 ;
		     }
		  else // if (elt1 == elt2)
		     {
		     //FIXME
		     ++pos1; ++pos2;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  //FIXME
		  }
	       pos2 = pos1 ;
	       }
	    // handle any leftovers from the first vector
	    while (pos1 < elts1)
	       {
	       //FIXME
	       ++pos1 ;
	       }
	    // handle any leftovers from the second vector
	    while (pos2 < elts2)
	       {
	       ++pos2 ;
	       }
	    return 0.0 ; //FIXME
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Mahalanobis" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureManhattan : public DistanceMeasureReciprocal<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
//TODO: refactor, normalization check first, then sparse/dense
//!!!
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    ValT sum(0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       if (this->m_opt.normalize)
		  {
		  ValT wt1, wt2 ;
		  normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
		  for ( ; pos1 < elts1 && pos2 < elts2 ; )
		     {
		     auto elt1(sv1->elementIndex(pos1)) ;
		     auto elt2(sv2->elementIndex(pos2)) ;
		     if (elt1 < elt2)
			{
			sum += abs_value(v1->elementValue(pos1++)/wt1) ;
			}
		     else if (elt1 > elt2)
			{
			sum += abs_value(v2->elementValue(pos2++)/wt2) ;
			}
		     else // if (elt1 == elt2)
			{
			sum += abs_value(v1->elementValue(pos1++)/wt1 - v2->elementValue(pos2++)/wt2) ;
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
			sum += abs_value(v1->elementValue(pos1++)) ;
			}
		     else if (elt1 > elt2)
			{
			sum += abs_value(v2->elementValue(pos2++)) ;
			}
		     else // if (elt1 == elt2)
			{
			sum += abs_value(v1->elementValue(pos1++) - v2->elementValue(pos2++)) ;
			}
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
	       if (this->m_opt.normalize)
		  {
		  ValT wt1, wt2 ;
		  normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
		  for (size_t i = 0 ; i < minlen ; ++i)
		     {
		     sum += abs_value(v1->elementValue(i)/wt1 - v2->elementValue(i)/wt2) ;
		     }
		  // handle any leftovers from the first vector
		  for (size_t i = minlen ; i < v1->numElements() ; ++i)
		     {
		     sum += abs_value(v1->elementValue(i)/wt1) ;
		     }
		  // handle any leftovers from the second vector
		  for (size_t i = minlen ; i < v2->numElements() ; ++i)
		     {
		     sum += abs_value(v2->elementValue(i)/wt2) ;
		     }
		  }
	       else
		  {
		  // no normalization
		  for (size_t i = 0 ; i < minlen ; ++i)
		     {
		     sum += abs_value(v1->elementValue(i) - v2->elementValue(i)) ;
		     }
		  // handle any leftovers from the first vector
		  for (size_t i = minlen ; i < v1->numElements() ; ++i)
		     {
		     sum += abs_value(v1->elementValue(i)) ;
		     }
		  // handle any leftovers from the second vector
		  for (size_t i = minlen ; i < v2->numElements() ; ++i)
		     {
		     sum += abs_value(v2->elementValue(i)) ;
		     }
		  }
	       }
	    return (double)sum ;
	 }

   protected:
      friend class VectorMeasure<IdxT,ValT> ;
      friend class VectorMeasureLNorm<IdxT,ValT> ;
      VectorMeasureManhattan() : DistanceMeasureReciprocal<IdxT,ValT>() {}
      VectorMeasureManhattan(const VectorSimilarityOptions& opt) : DistanceMeasureReciprocal<IdxT,ValT>(opt) {}
      virtual const char* myCanonicalName() const { return "Manhattan" ; }
   } ;

//============================================================================
// this is the measure from the original FramepaC, and differs from that
// described in
//    Robinson, W.S. 1951 A method for chronologically ordering
//    archaeological deposits. American Antiquity, 16: 293-301.

template <typename IdxT, typename ValT>
class VectorMeasureRobinson : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    double sum(0) ;
	    ValT totalwt1, totalwt2 ;
	    normalization_weights(v1,v2,1,totalwt1,totalwt2) ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     double val1(v1->elementValue(pos1++) / totalwt1) ;
		     sum += abs_value(val1) ;
		     }
		  else if (elt1 > elt2)
		     {
		     double val2(v2->elementValue(pos2++) / totalwt2) ;
		     sum += abs_value(val2) ;
		     }
		  else // if (elt1 == elt2)
		     {
		     double val1(v1->elementValue(pos1++) / totalwt1) ;
		     double val2(v2->elementValue(pos2++) / totalwt2) ;
		     sum += abs_value(val1 - val2) ;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  ValT val1(v1->elementValue(pos1) / totalwt1) ;
		  ValT val2(v2->elementValue(pos1) / totalwt2) ;
		  sum += abs_value(val1 - val2) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any leftovers from the first vector
	    while (pos1 < elts1)
	       {
	       ValT val1(v1->elementValue(pos1++) / totalwt1) ;
	       sum += abs_value(val1) ;
	       }
	    // handle any leftovers from the second vector
	    while (pos2 < elts2)
	       {
	       ValT val2(v2->elementValue(pos2++) / totalwt2) ;
	       sum += abs_value(val2) ;
	       }
	    return sum / 2.0 ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Robinson" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureSimilarityRatio : public SimilarityMeasure<IdxT, ValT>
   {
   public:
      virtual double similarity(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    double sumsq(0) ;
	    double prod(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       size_t pos1(0) ;
	       size_t pos2(0) ;
	       ValT wt1, wt2 ;
	       normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       while (pos1 < elts1)
		  {
		  double val1(v1->elementValue(pos1++) / wt1) ;
		  sumsq += (val1*val1) ;
		  }
	       while (pos2 < elts2)
		  {
		  double val2(v2->elementValue(pos2++) / wt2) ;
		  sumsq += (val2*val2) ;
		  }
	       }
	    else
	       {
	       // with two dense vectors we can be more efficient
	       size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
	       if (this->m_opt.normalize)
		  {
		  ValT wt1, wt2 ;
		  normalization_weights(v1,v2,this->m_opt.normalize,wt1,wt2) ;
		  for (size_t i = 0 ; i < minlen ; ++i)
		     {
		     double val1(v1->elementValue(i) / wt1) ;
		     double val2(v2->elementValue(i) / wt2) ;
		     sumsq += (val1 * val1) + (val2 * val2) ;
		     prod += (val1 * val2) ;
		     }
		  for (size_t i = minlen ; i < elts1 ; ++i)
		     {
		     double val1(v1->elementValue(i) / wt1) ;
		     sumsq += (val1 * val1) ;
		     }
		  for (size_t i = minlen ; i < elts2 ; ++i)
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
	       }
	    return (sumsq == prod) ? DBL_MAX : prod / (sumsq - prod) ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "SimilarityRatio" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureSquaredEuclidean : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    double sum(0) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
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
	       }
	    else
	       {
	       size_t minlen(std::min(elts1,elts2)) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  ValT diff = v1->elementValue(pos1) - v2->elementValue(pos1) ;
		  sum += (diff * diff) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any leftovers from the first vector
	    while (pos1 < elts1)
	       {
	       ValT diff = v1->elementValue(pos1++) ;
	       sum += (diff * diff) ;
	       }
	    // handle any leftovers from the second vector
	    while (pos2 < elts2)
	       {
	       ValT diff = v2->elementValue(pos2++) ;
	       sum += (diff * diff) ;
	       }
	    return sum ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "SquaredEuclidean" ; }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureEuclidean : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
	 {
	    VectorMeasureSquaredEuclidean<IdxT,ValT> vm ;
	    return std::sqrt(vm.distance(v1,v2)) ;
	 }

   protected:
      friend class VectorMeasure<IdxT,ValT> ;
      friend class VectorMeasureLNorm<IdxT,ValT> ;
      VectorMeasureEuclidean() : DistanceMeasure<IdxT,ValT>() {}
      VectorMeasureEuclidean(const VectorSimilarityOptions& opt) : DistanceMeasure<IdxT,ValT>(opt) {}
      virtual const char* myCanonicalName() const { return "Euclidean" ; }
} ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureLNorm : public DistanceMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2) const
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
	    double sum(0.0) ;
	    size_t pos1(0) ;
	    size_t pos2(0) ;
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (v1->isSparseVector()) // assume both vectors are sparse or both are dense
	       {
	       auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
	       auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
	       for ( ; pos1 < elts1 && pos2 < elts2 ; )
		  {
		  auto elt1(sv1->elementIndex(pos1)) ;
		  auto elt2(sv2->elementIndex(pos2)) ;
		  if (elt1 < elt2)
		     {
		     double diff(abs_value(v1->elementValue(pos1++))) ;
		     sum += std::pow(diff,power) ;
		     }
		  else if (elt1 > elt2)
		     {
		     double diff(abs_value(v2->elementValue(pos2++))) ;
		     sum += std::pow(diff,power) ;
		     }
		  else // if (elt1 == elt2)
		     {
		     double diff(abs_value(v1->elementValue(pos1++) - v2->elementValue(pos2++))) ;
		     sum += std::pow(diff,power) ;
		     }
		  }
	       }
	    else
	       {
	       size_t minlen(std::min(v1->numElements(),v2->numElements())) ;
	       for ( ; pos1 < minlen ; ++pos1)
		  {
		  ValT diff = abs_value(v1->elementValue(pos1) - v2->elementValue(pos1)) ;
		  sum += std::pow(diff,power) ;
		  }
	       pos2 = pos1 ;
	       }
	    // handle any leftovers from the first vector
	    while (pos1 < elts1)
	       {
	       ValT diff = abs_value(v1->elementValue(pos1++)) ;
	       sum += std::pow(diff,power) ;
	       }
	    // handle any leftovers from the second vector
	    while (pos2 < elts2)
	       {
	       ValT diff = abs_value(v2->elementValue(pos2++)) ;
	       sum += std::pow(diff,power) ;
	       }
	    return std::pow(sum,1.0/power) ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "L-Norm" ; }
   } ;

//============================================================================

/************************************************************************/
/*	methods for base class VectorMeasure				*/
/************************************************************************/

template <typename IdxT, typename ValT>
void VectorMeasure<IdxT,ValT>::contingencyTable(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2,
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
   if (v1->isSparseVector()) // assume both vectors are sparse or both dense
      {
      auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
      auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
      for ( ; pos1 < elts1 && pos2 < elts2 ; )
	 {
	 auto elt1 = sv1->elementIndex(pos1) ;
	 auto elt2 = sv2->elementIndex(pos2) ;
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
   else
      {
      // figure out how many elements to process in both vectors
      size_t minlen = std::min(elts1,elts2) ;
      // if either vector is all zeros, we don't need to treat it in
      //   common with the other one
      if (wt1 == 0.0 || wt2 == 0.0) minlen = 0 ;
      for ( ; pos1 < minlen ; ++pos1)
	 {
	 ValT val1(v1->elementValue(pos1)) ;
	 ValT val2(v2->elementValue(pos1)) ;
	 ValT com(std::min(val1/wt1,val2/wt2)) ;
	 a += com ;
	 b += (val1 - wt1*com) ;
	 c += (val2 - wt2*com) ;
	 }
      pos2 = pos1 ;
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
void VectorMeasure<IdxT,ValT>::contingencyTable(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2,
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
   if (v1->isSparseVector()) // assume vectors are both sparse or both dense
      {
      auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
      auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
      for ( ; pos1 < elts1 && pos2 < elts2 ; )
	 {
	 auto elt1 = sv1->elementIndex(pos1) ;
	 auto elt2 = sv2->elementIndex(pos2) ;
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
   else
      {
      // figure out how many elements to process in both vectors
      size_t minlen = std::min(elts1,elts2) ;
      for ( ; pos1 < minlen ; ++pos1)
	 {
	 ValT val1 = v1->elementValue(pos1) ;
	 ValT val2 = v2->elementValue(pos1) ;
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
void VectorMeasure<IdxT,ValT>::binaryAgreement(const Vector<IdxT,ValT>* v1, const Vector<IdxT,ValT>* v2,
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
   if (v1->isSparseVector()) // assume vectors are both sparse or both dense
      {
      auto sv1 = static_cast<const SparseVector<IdxT,ValT>*>(v1) ;
      auto sv2 = static_cast<const SparseVector<IdxT,ValT>*>(v2) ;
      for ( ; pos1 < elts1 && pos2 < elts2 ; )
	 {
	 auto elt1 = sv1->elementIndex(pos1) ;
	 auto elt2 = sv2->elementIndex(pos2) ;
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
   else
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


} // end namespace Fr

// end of file vecsim.cc //
