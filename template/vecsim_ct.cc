/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.03, last edit 2018-03-28					*/
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

//----------------------------------------------------------------------------
// a vector measure which has a similarity metric based on a 2x2
// contingency table and computes the distance as 1-sim

template <typename IdxT, typename ValT>
class SimilarityMeasureCT : public VectorMeasure<IdxT, ValT>
   {
   public:
      virtual double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    ValT both, v1_only, v2_only ;
	    this->contingencyTable(v1,v2,both,v1_only,v2_only) ;
	    return this->scoreContingencyTable(both,v1_only,v2_only) ;
	 }
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }

   protected:
      SimilarityMeasureCT() : VectorMeasure<IdxT,ValT>() {}
      SimilarityMeasureCT(const VectorSimilarityOptions& opt) : VectorMeasure<IdxT,ValT>(opt) {}
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a similarity metric based on a binary 2x2
// contingency table and computes the distance as 1-sim

template <typename IdxT, typename ValT>
class SimilarityMeasureBCT : public VectorMeasure<IdxT, ValT>
   {
   public:
      virtual double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    size_t both, v1_only, v2_only, neither ;
	    this->contingencyTable(v1,v2,both,v1_only,v2_only,neither) ;
	    return this->scoreContingencyTable(both,v1_only,v2_only,neither) ;
	 }
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }

   protected:
      SimilarityMeasureBCT() : VectorMeasure<IdxT,ValT>() {}
      SimilarityMeasureBCT(const VectorSimilarityOptions& opt) : VectorMeasure<IdxT,ValT>(opt) {}
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a similarity metric based on binary (dis)agreement
// and computes the distance as 1-sim

template <typename IdxT, typename ValT>
class SimilarityMeasureBA : public VectorMeasure<IdxT, ValT>
   {
   public:
      virtual double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    size_t both, disagree, neither ;
	    this->binaryAgreement(v1,v2,both,disagree,neither) ;
	    return this->scoreBinaryAgreement(both,disagree,neither) ;
	 }
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }

   protected:
      SimilarityMeasureBA() : VectorMeasure<IdxT,ValT>() {}
      SimilarityMeasureBA(const VectorSimilarityOptions& opt) : VectorMeasure<IdxT,ValT>(opt) {}
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a distance metric based on a 2x2
// contingency table and computes the similarity as 1-dist

template <typename IdxT, typename ValT>
class DistanceMeasureCT : public VectorMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    ValT both, v1_only, v2_only ;
	    this->contingencyTable(v1,v2,both,v1_only,v2_only) ;
	    return this->scoreContingencyTable(both,v1_only,v2_only) ;
	 }
      virtual double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }

   protected:
      DistanceMeasureCT() : VectorMeasure<IdxT,ValT>() {}
      DistanceMeasureCT(const VectorSimilarityOptions& opt) : VectorMeasure<IdxT,ValT>(opt) {}
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a distance metric based on a binary 2x2
// contingency table and computes the similarity as 1-dist

template <typename IdxT, typename ValT>
class DistanceMeasureBCT : public VectorMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    size_t both, v1_only, v2_only, neither ;
	    this->contingencyTable(v1,v2,both,v1_only,v2_only,neither) ;
	    return this->scoreContingencyTable(both,v1_only,v2_only,neither) ;
	 }
      virtual double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }

   protected:
      DistanceMeasureBCT() : VectorMeasure<IdxT,ValT>() {}
      DistanceMeasureBCT(const VectorSimilarityOptions& opt) : VectorMeasure<IdxT,ValT>(opt) {}
   } ;

//----------------------------------------------------------------------------
// a vector measure which has a distance metric based on binary (dis)agreement
// contingency table and computes the similarity as 1-dist

template <typename IdxT, typename ValT>
class DistanceMeasureBA : public VectorMeasure<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    size_t both, disagree, neither ;
	    this->binaryAgreement(v1,v2,both,disagree,neither) ;
	    return this->scoreBinaryAgreement(both,disagree,neither) ;
	 }
      virtual double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 - similarity(v1,v2) ;
	 }

   protected:
      DistanceMeasureBA() : VectorMeasure<IdxT,ValT>() {}
      DistanceMeasureBA(const VectorSimilarityOptions& opt) : VectorMeasure<IdxT,ValT>(opt) {}
   } ;

//============================================================================
//   Measures based on a 2x2 contingency table			            //
//============================================================================

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureAntiDice : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Anti-Dice" ; }
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
class VectorMeasureBinaryAntiDice : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Anti-Dice" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    double denom(both + 2.0 * only_1) ;
	    return (denom > 0.0) ? both / denom : 1.0 ; // (all-zero vectors are defined to be identical)
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
// measure first published in
//      Benini, R. (1901) "Principii di demografie".  No. 29 of
//      manuali Barbera di science giuridiche sociali e poltiche.
//      Firenzi: G. Barbera

template <typename IdxT, typename ValT>
class VectorMeasureBenini : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Benini" ; }
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
class VectorMeasureBinaryBenini : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Benini" ; }
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
class VectorMeasureBraunBlanquet : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Braun-Blanquet" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT larger(std::max(both+v1_only,both+v2_only)) ;
	    return larger > 0 ? both / larger : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryBraunBlanquet : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Braun-Blanquet" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
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
class VectorMeasureBrayCurtis : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Bray-Curtis" ; }
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
class VectorMeasureBinaryBrayCurtis : public DistanceMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Bray-Curtis" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    size_t sum(2.0 * both + only_1) ;
	    return sum ? only_1 / sum : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Colwell&Coddington (1948) and Gaston (2001)
// (b+c)/(2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureCocogaston : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Cocogaston" ; }
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
class VectorMeasureBinaryCocogaston : public DistanceMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Cocogaston" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    if (both + only_1 == 0)
	       return 1.0 ; // all-zero vectors are defined to be identical
	    return only_1 / (2.0 * both + only_1) ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Cody (1993)

template <typename IdxT, typename ValT>
class VectorMeasureCody : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Cody" ; }
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
class VectorMeasureBinaryCody : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Cody" ; }
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
class VectorMeasureDice : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Dice" ; }
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
class VectorMeasureBinaryDice : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Dice" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    both *= 2 ;
	    double denom(both + only_1) ;
	    return denom ? both / denom : 1.0 ;// all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  a / (sqrt(a+b)(a+c)) - 0.5*max(b,c)

template <typename IdxT, typename ValT>
class VectorMeasureFagerMcGowan : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Fager-McGowan" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT prod((both+v1_only)*(both+v2_only)) ;
	    double denom(std::sqrt(prod) - std::max(v1_only,v2_only)/2) ;
	    return denom ? both / denom : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryFagerMcGowan : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Fager-McGowan" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double prod((both+v1_only)*(both+v2_only)) ;
	    double denom(std::sqrt(prod) - std::max(v1_only,v2_only)/2) ;
	    return denom ? both / denom : 1.0 ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureGamma : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Gamma" ; }
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
class VectorMeasureBinaryGamma : public SimilarityMeasure<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Gamma" ; }
      virtual double similarity(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    size_t elts1(v1->numElements()) ;
	    size_t elts2(v2->numElements()) ;
	    if (!elts1 && !elts2)
	       return 1.0 ;			// two zero-length vectors are identical
	    else if (!elts1 || !elts2)
	       return -1.0 ;			// maximal difference if only one vector zero-length
	    size_t both, v1_only, v2_only, neither ;
	    this->contingencyTable(v1,v2,both,v1_only,v2_only,neither) ;
	    double N(both + v1_only + v2_only + neither) ;
	    double concordance(both / N * neither / N) ;
	    double discordance(v1_only / N * v2_only / N) ;
	    return (concordance + discordance > 0) ? (concordance - discordance) / (concordance + discordance) : 1.0 ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureGilbert : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Gilbert" ; }
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
class VectorMeasureBinaryGilbert : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Gilbert" ; }
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
class VectorMeasureGini : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Gini" ; }
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
class VectorMeasureBinaryGini : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Gini" ; }
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
class VectorMeasureHarrison : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Harrison" ; }
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
class VectorMeasureBinaryHarrison : public DistanceMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Harrison" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
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
class VectorMeasureJaccard : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Jaccard" ; }
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
class VectorMeasureBinaryJaccard : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Jaccard" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    double either(both + only_1) ;
	    return either ? both / either : 1.0 ; // all-zero vectors are defined to be identical
	 }
   } ;

//============================================================================
//  a/(b+c)
// "simba" package calls (a+d)/(b+c) sokal4 from Sokal&Sneath(1963)
// same as Tanimoto from old FramepaC

template <typename IdxT, typename ValT>
class VectorMeasureKulczynski1 : public SimilarityMeasureCT<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 / (1.0 + this->similarity(v1,v2)) ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Kulczynski1" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double only_1(v1_only+v2_only) ;
	    return only_1 ? both / only_1 : HUGE_VAL ;
	 }
   } ;

//============================================================================

template <typename IdxT, typename ValT>
class VectorMeasureBinaryKulczynski1 : public SimilarityMeasureBA<IdxT, ValT>
   {
   public:
      virtual double distance(const Vector<ValT>* v1, const Vector<ValT>* v2) const
	 {
	    return 1.0 / (1.0 + this->similarity(v1,v2)) ;
	 }

   protected:
      virtual const char* myCanonicalName() const { return "Binary Kulczynski1" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    // both += neither ;   // "simba"s sokal4
	    return only_1 ? both / only_1 : HUGE_VAL ;
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
class VectorMeasureKulczynski2 : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Kulczynski2" ; }
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
class VectorMeasureBinaryKulczynski2 : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Kulczynski2" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
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
class VectorMeasureLanceWilliams : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Lance-Williams" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(2.0*both + v1_only + v2_only) ;
	    return denom ? (v1_only + v2_only) / denom : 1.0 ;
	 }
   } ;

//============================================================================
// (b+c) / (2a + b + c)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryLanceWilliams : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Lance-Williams" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    double denom(2.0*both + only_1) ;
	    return denom ? only_1 / denom : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Lande (1996)
// (b+c)/2

template <typename IdxT, typename ValT>
class VectorMeasureLande : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Lande" ; }
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
class VectorMeasureBinaryLande : public DistanceMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Lande" ; }
      virtual double scoreBinaryAgreement(size_t /*both*/, size_t only_1, size_t /*neither*/) const
	 {
	    return only_1 / 2.0 ;
	 }
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
class VectorMeasureLennon : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Lennon" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT total1(both + v1_only) ;
	    ValT total2(both + v2_only) ;
	    double denom(total1 + total2) ;
	    // all-zero vectors are defined to be identical
	    return denom ? (2.0 * abs_value(v1_only - v2_only) / denom) : 0.0 ;
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
class VectorMeasureBinaryLennon : public DistanceMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Lennon" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
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
class VectorMeasureLennon2 : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Lennon2" ; }
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
class VectorMeasureBinaryLennon2 : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Lennon2" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    double sum(both + only_1) ;
	    return sum ? std::log2((both+sum)/sum) : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Legendre & Legendre (1998)
//  range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureLegendre : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Legendre" ; }
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
class VectorMeasureBinaryLegendre : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Legendre" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
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
class VectorMeasureMaarel : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Maarel" ; }
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
class VectorMeasureBinaryMaarel : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Maarel" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
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
class VectorMeasureMagurran : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Magurran" ; }
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
class VectorMeasureBinaryMagurran : public DistanceMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Magurran" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    double sum(both + only_1) ;
	    // all-zero vectors are defined to be identical
	    return sum ? ((both + sum) * (1.0 - (both / sum))) : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
// measure first published in
//     Hubalek (1982)
// range: 0 ... 1

template <typename IdxT, typename ValT>
class VectorMeasureMcConnagh : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "McConnagh" ; }
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
class VectorMeasureBinaryMcConnagh : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary McConnagh" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
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
class VectorMeasureModGini : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "ModGini" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    ValT N(both + v1_only + v2_only) ;
	    both /= N ;
	    v1_only /= N ;
	    v2_only /= N ;
	    ValT total1(both+v1_only) ;
	    ValT total2(both+v2_only) ;
	    return (both - total1*total2) / (1.0 - abs_value(v1_only-v2_only)/2 - total1*total2) ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureBinaryModGini : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary ModGini" ; }
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
class VectorMeasureMountford : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Mountford" ; }
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
class VectorMeasureBinaryMountford : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Mountford" ; }
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
class VectorMeasureOchiai : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Ochiai" ; }
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
class VectorMeasureBinaryOchiai : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Ochiai" ; }
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
class VectorMeasureRoutledge1 : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Routledge1" ; }
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
class VectorMeasureBinaryRoutledge1 : public DistanceMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Routledge1" ; }
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
class VectorMeasureRoutledge2 : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Routledge2" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(2.0 * both + v1_only + v2_only) ;
	    if (!denom)
	       return 0.0 ;			// two all-zero vectors are identical
	    ValT total1(both+v1_only) ;
	    ValT total2(both+v2_only) ;
	    double num1(2 * both * log(2)) ;
	    double num2(this->p_log_p(total1) + this->p_log_p(total2)) ;
	    return log(denom) - (num1 + num2) / denom ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Routledge (1977)

template <typename IdxT, typename ValT>
class VectorMeasureBinaryRoutledge2 : public DistanceMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Routledge2" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
	 {
	    double denom(2.0 * both + v1_only + v2_only) ;
	    if (!denom)
	       return 0.0 ;			// two all-zero vectors are identical
	    size_t total1(both+v1_only) ;
	    size_t total2(both+v2_only) ;
	    double num1(2 * both * log(2)) ;
	    double num2(this->p_log_p(total1) + this->p_log_p(total2)) ;
	    return log(denom) - (num1 + num2) / denom ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation
//  a / min(a+b,a+c) == a / (a + min(b,c))

template <typename IdxT, typename ValT>
class VectorMeasureSimpson : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Simpson" ; }
      virtual double scoreContingencyTable(ValT both, ValT v1_only, ValT v2_only) const
	 {
	    double denom(both + std::min(v1_only,v2_only)) ;
	    return denom ? both/denom : 1.0 ;
	 }
   } ;

//============================================================================
// see https://en.wikipedia.org/wiki/Qualitative_variation

template <typename IdxT, typename ValT>
class VectorMeasureBinarySimpson : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Simpson" ; }
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
class VectorMeasureSokalSneath : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Sokal-Sneath" ; }
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
class VectorMeasureBinarySokalSneath : public SimilarityMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Sokal-Sneath" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t neither) const
	 {
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
class VectorMeasureSorgenfrei : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Sorgenfrei" ; }
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
class VectorMeasureBinarySorgenfrei : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Sorgenfrei" ; }
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
class VectorMeasureTripartite : public SimilarityMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Tripartite" ; }
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
class VectorMeasureBinaryTripartite : public SimilarityMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Tripartite" ; }
      virtual double scoreContingencyTable(size_t both, size_t v1_only, size_t v2_only, size_t /*neither*/) const
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
class VectorMeasureWhittaker : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Whittaker" ; }
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
class VectorMeasureBinaryWhittaker : public DistanceMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Whittaker" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    size_t sum(both + only_1) ;
	    return sum ? (2.0 * sum / (both + sum)) - 1.0 : 1.0 ;
	 }
   } ;

//============================================================================
//  see documentation for R-project "simba" package
//  published in Williams (1996), Koleff et al (20030

template <typename IdxT, typename ValT>
class VectorMeasureWilliams : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Williams" ; }
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
class VectorMeasureBinaryWilliams : public DistanceMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Williams" ; }
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
class VectorMeasureWilliams2 : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Williams2" ; }
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
class VectorMeasureBinaryWilliams2 : public DistanceMeasureBCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Williams2" ; }
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
class VectorMeasureWilsonShmida : public DistanceMeasureCT<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Wilson-Shmida" ; }
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
class VectorMeasureBinaryWilsonShmida : public DistanceMeasureBA<IdxT, ValT>
   {
   protected:
      virtual const char* myCanonicalName() const { return "Binary Wilson-Shmida" ; }
      virtual double scoreBinaryAgreement(size_t both, size_t only_1, size_t /*neither*/) const
	 {
	    double denom(2.0 * both + only_1) ;
	    return denom ? only_1 / denom : 1.0 ; // all-zero vectors are defined to be identical
	 }
   } ;



} // end namespace Fr

// end of file vecsim_ct.cc //
