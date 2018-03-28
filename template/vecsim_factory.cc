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
#include "template/vecsim.cc"
#include "template/vecsim_ct.cc"

/************************************************************************/
/************************************************************************/

namespace Fr
{

template <typename IdxT, typename ValT>
VectorMeasure<IdxT,ValT>* VectorMeasure<IdxT,ValT>::create(const char* simtype, const char* options)
{
   (void)simtype; (void)options;
   return nullptr ;
}

//----------------------------------------------------------------------------

template <typename IdxT, typename ValT>
VectorMeasure<IdxT,ValT>* VectorMeasure<IdxT,ValT>::create(VectorSimilarityMeasure simtype, const char* options)
{
   VectorMeasure* meas = nullptr ;
   switch (simtype)
      {
      case VectorSimilarityMeasure::benini:
	 meas = new VectorMeasureBenini<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::bhattacharyya:
	 meas = new VectorMeasureBhattacharyya<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_anti_dice:
	 meas = new VectorMeasureBinaryAntiDice<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_benini:
	 meas = new VectorMeasureBinaryBenini<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_braun_blanquet:
	 meas = new VectorMeasureBinaryBraunBlanquet<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_bray_curtis:
	 meas = new VectorMeasureBinaryBrayCurtis<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_cocogaston:
	 meas = new VectorMeasureBinaryCocogaston<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_cody:
	 meas = new VectorMeasureBinaryCody<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_dice:
	 meas = new VectorMeasureBinaryDice<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_fager_mcgowan:
	 meas = new VectorMeasureBinaryFagerMcGowan<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_gamma:
	 meas = new VectorMeasureBinaryGamma<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_gilbert:
	 meas = new VectorMeasureBinaryGilbert<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_gini:
	 meas = new VectorMeasureBinaryGini<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_harrison:
	 meas = new VectorMeasureBinaryHarrison<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_jaccard:
	 meas = new VectorMeasureBinaryJaccard<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_kulczynski1:
	 meas = new VectorMeasureBinaryKulczynski1<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_kulczynski2:
	 meas = new VectorMeasureBinaryKulczynski2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_lance_williams:
	 meas = new VectorMeasureBinaryLanceWilliams<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_lande:
	 meas = new VectorMeasureBinaryLande<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_lennon:
	 meas = new VectorMeasureBinaryLennon<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_lennon2:
	 meas = new VectorMeasureBinaryLennon2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_legendre2:
	 meas = new VectorMeasureBinaryLegendre<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_maarel:
	 meas = new VectorMeasureBinaryMaarel<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_magurran:
	 meas = new VectorMeasureBinaryMagurran<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_mcconnagh:
	 meas = new VectorMeasureBinaryMcConnagh<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_modgini:
	 meas = new VectorMeasureBinaryModGini<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_mountford:
	 meas = new VectorMeasureBinaryMountford<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_ochiai:
	 meas = new VectorMeasureBinaryOchiai<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_routledge1:
	 meas = new VectorMeasureBinaryRoutledge1<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_routledge2:
	 meas = new VectorMeasureBinaryRoutledge2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_simpson:
	 meas = new VectorMeasureBinarySimpson<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_sokal_sneath:
	 meas = new VectorMeasureBinarySokalSneath<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_sorgenfrei:
	 meas = new VectorMeasureBinarySorgenfrei<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_tripartite:
	 meas = new VectorMeasureBinaryTripartite<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_whittaker:
	 meas = new VectorMeasureBinaryWhittaker<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_williams:
	 meas = new VectorMeasureBinaryWilliams<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_williams2:
	 meas = new VectorMeasureBinaryWilliams2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::binary_wilsonshmida:
	 meas = new VectorMeasureBinaryWilsonShmida<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::braun_blanquet:
	 meas = new VectorMeasureBraunBlanquet<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::bray_curtis:
	 meas = new VectorMeasureBrayCurtis<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::canberra:
	 meas = new VectorMeasureCanberra<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::circle_product:
	 meas = new VectorMeasureCircleProduct<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::clark:
	 meas = new VectorMeasureClark<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::cocogaston:
	 meas = new VectorMeasureCocogaston<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::cody:
	 meas = new VectorMeasureCody<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::cosine:
	 meas = new VectorMeasureCosine<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::dice:
	 meas = new VectorMeasureDice<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::euclidean:
	 meas = new VectorMeasureEuclidean<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::fager_mcgowan:
	 meas = new VectorMeasureFagerMcGowan<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::gamma:
	 meas = new VectorMeasureGamma<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::gilbert:
	 meas = new VectorMeasureGilbert<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::gini:
	 meas = new VectorMeasureGini<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::harrison:
	 meas = new VectorMeasureHarrison<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::hellinger:
	 meas = new VectorMeasureHellinger<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::jaccard:
	 meas = new VectorMeasureJaccard<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::jensen:
	 meas = new VectorMeasureJensen<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::jensen_shannon:
	 meas = new VectorMeasureJensenShannon<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::kulczynski1:
	 meas = new VectorMeasureKulczynski1<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::kulczynski2:
	 meas = new VectorMeasureKulczynski2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::kullback_leibler:
	 meas = new VectorMeasureKullbackLeibler<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::kumar_johnson:
	 meas = new VectorMeasureKumarJohnson<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::l0:
	 meas = new VectorMeasureL0Norm<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::lance_williams:
	 meas = new VectorMeasureLanceWilliams<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::lande:
	 meas = new VectorMeasureLande<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::lennon:
	 meas = new VectorMeasureLennon<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::lennon2:
	 meas = new VectorMeasureLennon2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::legendre2:
	 meas = new VectorMeasureLegendre<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::linf:
	 meas = new VectorMeasureLinfNorm<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::lorentzian:
	 meas = new VectorMeasureLorentzian<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::l_norm:
	 meas = new VectorMeasureLNorm<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::maarel:
	 meas = new VectorMeasureMaarel<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::magurran:
	 meas = new VectorMeasureMagurran<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::mahalanobis:
	 meas = new VectorMeasureMahalanobis<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::manhattan:
	 meas = new VectorMeasureManhattan<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::matusita:
	 meas = new VectorMeasureMatusita<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::mcconnagh:
	 meas = new VectorMeasureMcConnagh<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::modified_gini:
	 meas = new VectorMeasureModGini<IdxT,ValT> ;
	 break ;
//      case VectorSimilarityMeasure::morisita:
//	 meas = new VectorMeasureMorisita<IdxT,ValT> ;
//	 break ;
      case VectorSimilarityMeasure::mountford:
	 meas = new VectorMeasureMountford<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::ochiai:
	 meas = new VectorMeasureOchiai<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::robinson:
	 meas = new VectorMeasureRobinson<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::routledge1:
	 meas = new VectorMeasureRoutledge1<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::routledge2:
	 meas = new VectorMeasureRoutledge2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::sangvi:
	 meas = new VectorMeasureSangvi<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::similarity_ratio:
	 meas = new VectorMeasureSimilarityRatio<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::simpson:
	 meas = new VectorMeasureSimpson<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::soergel:
	 meas = new VectorMeasureSoergel<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::sokal_sneath:
	 meas = new VectorMeasureSokalSneath<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::sorgenfrei:
	 meas = new VectorMeasureSorgenfrei<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::squared_chord:
	 meas = new VectorMeasureSquaredChord<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::squared_euclidean:
	 meas = new VectorMeasureSquaredEuclidean<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::taneja:
	 meas = new VectorMeasureTaneja<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::tripartite:
	 meas = new VectorMeasureTripartite<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::wave_hedges:
	 meas = new VectorMeasureWaveHedges<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::whittaker:
	 meas = new VectorMeasureWhittaker<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::williams:
	 meas = new VectorMeasureWilliams<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::williams2:
	 meas = new VectorMeasureWilliams2<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::wilson_shmida:
	 meas = new VectorMeasure<IdxT,ValT> ;
	 break ;
      case VectorSimilarityMeasure::none:
      default:
	 return nullptr ;
      }
   if (meas)
      {
      // parse the options string
      (void)options;
      }
   return meas ;
}



} // end of namespace Fr

// end of vecsim_factory.cc //
