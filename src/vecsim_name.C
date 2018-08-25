/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.09, last edit 2018-08-25					*/
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

#include "framepac/utility.h"
#include "framepac/vecsim.h"

namespace Fr
{

/************************************************************************/
/*	Types for this module						*/
/************************************************************************/

struct SimMeasure
   {
      const char* name ;
      VectorSimilarityMeasure measure ;
   } ;


/************************************************************************/
/*	Global data for this module					*/
/************************************************************************/

static SimMeasure measure_names[] = {
   { "Cosine", VectorSimilarityMeasure::cosine },
   { "Anti-Dice", VectorSimilarityMeasure::anti_dice },
   { "AntiDice", VectorSimilarityMeasure::anti_dice },
   { "Benini", VectorSimilarityMeasure::benini },
   { "Bhattacharyya", VectorSimilarityMeasure::bhattacharyya },
   { "Binary Anti-Dice", VectorSimilarityMeasure::binary_anti_dice },
   { "Binary AntiDice", VectorSimilarityMeasure::binary_anti_dice },
   { "BinaryAntiDice", VectorSimilarityMeasure::binary_anti_dice },
   { "Binary Benini", VectorSimilarityMeasure::binary_benini },
   { "BinaryBenini", VectorSimilarityMeasure::binary_benini },
   { "Binary Braun-Blanquet", VectorSimilarityMeasure::binary_braun_blanquet },
   { "Binary BraunBlanquet", VectorSimilarityMeasure::binary_braun_blanquet },
   { "BinaryBraunBlanquet", VectorSimilarityMeasure::binary_braun_blanquet },
   { "Binary Bray-Curtis", VectorSimilarityMeasure::binary_bray_curtis },
   { "Binary BrayCurtis", VectorSimilarityMeasure::binary_bray_curtis },
   { "BinaryBrayCurtis", VectorSimilarityMeasure::binary_bray_curtis },
   { "Binary Cocogaston", VectorSimilarityMeasure::binary_cocogaston },
   { "BinaryCocogaston", VectorSimilarityMeasure::binary_cocogaston },
   { "Binary Cody", VectorSimilarityMeasure::binary_cody },
   { "BinaryCody", VectorSimilarityMeasure::binary_cody },
   { "Binary Dice", VectorSimilarityMeasure::binary_dice },
   { "BinaryDice", VectorSimilarityMeasure::binary_dice },
   { "Binary Fager-McGowan", VectorSimilarityMeasure::binary_fager_mcgowan },
   { "Binary FagerMcGowan", VectorSimilarityMeasure::binary_fager_mcgowan },
   { "BinaryFagerMcGowan", VectorSimilarityMeasure::binary_fager_mcgowan },
   { "Binary Gamma", VectorSimilarityMeasure::binary_gamma },
   { "BinaryGamma", VectorSimilarityMeasure::binary_gamma },
   { "Binary Gilbert", VectorSimilarityMeasure::binary_gilbert },
   { "BinaryGilbert", VectorSimilarityMeasure::binary_gilbert },
   { "Binary Gini", VectorSimilarityMeasure::binary_gini },
   { "BinaryGini", VectorSimilarityMeasure::binary_gini },
   { "Binary Harrison", VectorSimilarityMeasure::binary_harrison },
   { "BinaryHarrison", VectorSimilarityMeasure::binary_harrison },
   { "Binary Jaccard", VectorSimilarityMeasure::binary_jaccard },
   { "BinaryJaccard", VectorSimilarityMeasure::binary_jaccard },
   { "Binary Kulczynski1", VectorSimilarityMeasure::binary_kulczynski1 },
   { "BinaryKulczynski1", VectorSimilarityMeasure::binary_kulczynski1 },
   { "Binary Kulczynski2", VectorSimilarityMeasure::binary_kulczynski2 },
   { "BinaryKulczynski2", VectorSimilarityMeasure::binary_kulczynski2 },
   { "Binary Lance-Williams", VectorSimilarityMeasure::binary_lance_williams },
   { "Binary LanceWilliams", VectorSimilarityMeasure::binary_lance_williams },
   { "BinaryLanceWilliams", VectorSimilarityMeasure::binary_lance_williams },
   { "Binary Lande", VectorSimilarityMeasure::binary_lande },
   { "BinaryLande", VectorSimilarityMeasure::binary_lande },
   { "Binary Lennon", VectorSimilarityMeasure::binary_lennon },
   { "BinaryLennon", VectorSimilarityMeasure::binary_lennon },
   { "Binary Lennon2", VectorSimilarityMeasure::binary_lennon2 },
   { "BinaryLennon2", VectorSimilarityMeasure::binary_lennon2 },
   { "Binary Legendre", VectorSimilarityMeasure::binary_legendre2 },
   { "BinaryLegendre", VectorSimilarityMeasure::binary_legendre2 },
   { "Binary Maarel", VectorSimilarityMeasure::binary_maarel },
   { "BinaryMaarel", VectorSimilarityMeasure::binary_maarel },
   { "Binary Magurran", VectorSimilarityMeasure::binary_magurran },
   { "BinaryMagurran", VectorSimilarityMeasure::binary_magurran },
   { "Binary McConnagh", VectorSimilarityMeasure::binary_mcconnagh },
   { "BinaryMcConnagh", VectorSimilarityMeasure::binary_mcconnagh },
   { "Binary Modified Gini", VectorSimilarityMeasure::binary_modgini },
   { "Binary Mod-Gini", VectorSimilarityMeasure::binary_modgini },
   { "Binary ModGini", VectorSimilarityMeasure::binary_modgini },
   { "BinaryModGini", VectorSimilarityMeasure::binary_modgini },
   { "Binary Mountford", VectorSimilarityMeasure::binary_mountford },
   { "BinaryMountford", VectorSimilarityMeasure::binary_mountford },
   { "Binary Ochiai", VectorSimilarityMeasure::binary_ochiai },
   { "BinaryOchiai", VectorSimilarityMeasure::binary_ochiai },
   { "Binary Routledge1", VectorSimilarityMeasure::binary_routledge1 },
   { "BinaryRoutledge1", VectorSimilarityMeasure::binary_routledge1 },
   { "Binary Routledge2", VectorSimilarityMeasure::binary_routledge2 },
   { "BinaryRoutledge2", VectorSimilarityMeasure::binary_routledge2 },
   { "Binary Simpson", VectorSimilarityMeasure::binary_simpson },
   { "BinarySimpson", VectorSimilarityMeasure::binary_simpson },
   { "Binary Sokal-Sneath", VectorSimilarityMeasure::binary_sokal_sneath },
   { "Binary SokalSneath", VectorSimilarityMeasure::binary_sokal_sneath },
   { "BinarySokalSneath", VectorSimilarityMeasure::binary_sokal_sneath },
   { "Binary Sorgenfrei", VectorSimilarityMeasure::binary_sorgenfrei },
   { "BinarySorgenfrei", VectorSimilarityMeasure::binary_sorgenfrei },
   { "Binary Tripartite", VectorSimilarityMeasure::binary_tripartite },
   { "BinaryTripartite", VectorSimilarityMeasure::binary_tripartite },
   { "Binary Whittaker", VectorSimilarityMeasure::binary_whittaker },
   { "BinaryWhittaker", VectorSimilarityMeasure::binary_whittaker },
   { "Binary Williams", VectorSimilarityMeasure::binary_williams },
   { "BinaryWilliams", VectorSimilarityMeasure::binary_williams },
   { "Binary Williams2", VectorSimilarityMeasure::binary_williams2 },
   { "BinaryWilliams2", VectorSimilarityMeasure::binary_williams2 },
   { "Binary Wilson-Shmida", VectorSimilarityMeasure::binary_wilsonshmida },
   { "Binary WilsonShmida", VectorSimilarityMeasure::binary_wilsonshmida },
   { "BinaryWilsonShmida", VectorSimilarityMeasure::binary_wilsonshmida },
   { "Braun-Blanquet", VectorSimilarityMeasure::braun_blanquet },
   { "BraunBlanquet", VectorSimilarityMeasure::braun_blanquet },
   { "BB", VectorSimilarityMeasure::braun_blanquet },
   { "Bray-Curtis", VectorSimilarityMeasure::bray_curtis },
   { "BrayCurtis", VectorSimilarityMeasure::bray_curtis },
   { "BC", VectorSimilarityMeasure::bray_curtis },
   { "Canberra", VectorSimilarityMeasure::canberra },
   { "Circle Product", VectorSimilarityMeasure::circle_product },
   { "Circle-Product", VectorSimilarityMeasure::circle_product },
   { "CircleProduct", VectorSimilarityMeasure::circle_product },
   { "Clark", VectorSimilarityMeasure::clark },
   { "Cocogaston", VectorSimilarityMeasure::cocogaston },
   { "Cody", VectorSimilarityMeasure::cody },
   { "Cosine", VectorSimilarityMeasure::cosine },
   { "Dice", VectorSimilarityMeasure::dice },
   { "Euclidean", VectorSimilarityMeasure::euclidean },
   { "Fager-McGowan", VectorSimilarityMeasure::fager_mcgowan },
   { "FagerMcGowan", VectorSimilarityMeasure::fager_mcgowan },
   { "FM", VectorSimilarityMeasure::fager_mcgowan },
   { "Fidelity", VectorSimilarityMeasure::fidelity },
   { "Gamma", VectorSimilarityMeasure::gamma },
   { "Gilbert", VectorSimilarityMeasure::gilbert },
   { "Gini", VectorSimilarityMeasure::gini },
   { "Harrison", VectorSimilarityMeasure::harrison },
   { "Hellinger", VectorSimilarityMeasure::hellinger },
   { "Jaccard", VectorSimilarityMeasure::jaccard },
   { "Jensen", VectorSimilarityMeasure::jensen },
   { "Jensen-Shannon", VectorSimilarityMeasure::jensen_shannon },
   { "JensenShannon", VectorSimilarityMeasure::jensen_shannon },
   { "JS", VectorSimilarityMeasure::jensen_shannon },
   { "Kulczynski1", VectorSimilarityMeasure::kulczynski1 },
   { "Kulczynski2", VectorSimilarityMeasure::kulczynski2 },
   { "Kullback-Leibler", VectorSimilarityMeasure::kullback_leibler },
   { "KL", VectorSimilarityMeasure::kullback_leibler },
   { "Kumar-Johnson", VectorSimilarityMeasure::kumar_johnson },
   { "KumarJohnson", VectorSimilarityMeasure::kumar_johnson },
   { "KJ", VectorSimilarityMeasure::kumar_johnson },
   { "L0", VectorSimilarityMeasure::l0 },
   { "Lance-Williams", VectorSimilarityMeasure::lance_williams },
   { "LanceWilliams", VectorSimilarityMeasure::lance_williams },
   { "LW", VectorSimilarityMeasure::lance_williams },
   { "Lande", VectorSimilarityMeasure::lande },
   { "Lennon", VectorSimilarityMeasure::lennon },
   { "Lennon2", VectorSimilarityMeasure::lennon2 },
   { "Legendre", VectorSimilarityMeasure::legendre2 },
   { "Linf", VectorSimilarityMeasure::linf },
   { "L-infinity", VectorSimilarityMeasure::linf },
   { "Lorentzian", VectorSimilarityMeasure::lorentzian },
   { "L-norm", VectorSimilarityMeasure::l_norm },
   { "Lnorm", VectorSimilarityMeasure::l_norm },
   { "Maarel", VectorSimilarityMeasure::maarel },
   { "Magurran", VectorSimilarityMeasure::magurran },
   { "Mahalanobis", VectorSimilarityMeasure::mahalanobis },
   { "Manhattan", VectorSimilarityMeasure::manhattan },
   { "L1", VectorSimilarityMeasure::manhattan },
   { "Matusita", VectorSimilarityMeasure::matusita },
   { "McConnagh", VectorSimilarityMeasure::mcconnagh },
   { "Modified Gini", VectorSimilarityMeasure::modified_gini },
   { "Mod-Gini", VectorSimilarityMeasure::modified_gini },
   { "ModGini", VectorSimilarityMeasure::modified_gini },
   { "Mountford", VectorSimilarityMeasure::mountford },
   { "Ochiai", VectorSimilarityMeasure::ochiai },
   { "Robinson", VectorSimilarityMeasure::robinson },
   { "Routledge1", VectorSimilarityMeasure::routledge1 },
   { "Routledge2", VectorSimilarityMeasure::routledge2 },
   { "Sangvi", VectorSimilarityMeasure::sangvi },
   { "Similarity Ratio", VectorSimilarityMeasure::similarity_ratio },
   { "Similarity-Ratio", VectorSimilarityMeasure::similarity_ratio },
   { "SimilarityRatio", VectorSimilarityMeasure::similarity_ratio },
   { "Sim-Ratio", VectorSimilarityMeasure::similarity_ratio },
   { "SimRatio", VectorSimilarityMeasure::similarity_ratio },
   { "Simpson", VectorSimilarityMeasure::simpson },
   { "Soergel", VectorSimilarityMeasure::soergel },
   { "Sokal-Sneath", VectorSimilarityMeasure::sokal_sneath },
   { "SokalSneath", VectorSimilarityMeasure::sokal_sneath },
   { "SS", VectorSimilarityMeasure::sokal_sneath },
   { "Sorgenfrei", VectorSimilarityMeasure::sorgenfrei },
   { "Squared Chord", VectorSimilarityMeasure::squared_chord },
   { "Squared-Chord", VectorSimilarityMeasure::squared_chord },
   { "SquaredChord", VectorSimilarityMeasure::squared_chord },
   { "Squared Euclidean", VectorSimilarityMeasure::squared_euclidean },
   { "Squared-Euclidean", VectorSimilarityMeasure::squared_euclidean },
   { "SquaredEuclidean", VectorSimilarityMeasure::squared_euclidean },
   { "Taneja", VectorSimilarityMeasure::taneja },
   { "Tripartite", VectorSimilarityMeasure::tripartite },
   { "3P", VectorSimilarityMeasure::tripartite },
   { "Tversky", VectorSimilarityMeasure::tversky },
   { "Wave Hedges", VectorSimilarityMeasure::wave_hedges },
   { "Wave-Hedges", VectorSimilarityMeasure::wave_hedges },
   { "WaveHedges", VectorSimilarityMeasure::wave_hedges },
   { "WH", VectorSimilarityMeasure::wave_hedges },
   { "Whittaker", VectorSimilarityMeasure::whittaker },
   { "Williams", VectorSimilarityMeasure::williams },
   { "Williams2", VectorSimilarityMeasure::williams2 },
   { "Wilson-Shmida", VectorSimilarityMeasure::wilson_shmida },
   { "WilsonShmida", VectorSimilarityMeasure::wilson_shmida },
   { "WS", VectorSimilarityMeasure::wilson_shmida },
//   { "", VectorSimilarityMeasure:: },
   { "User", VectorSimilarityMeasure::user },
   // the end-of-array sentinel
   { nullptr, VectorSimilarityMeasure::none }
   } ;

/************************************************************************/
/************************************************************************/

static const void* next_name(const void* ptr)
{
   const SimMeasure* name = reinterpret_cast<const SimMeasure*>(ptr) ;
   ++name ;
   return  (name->name == nullptr) ? nullptr : name ;
}

//----------------------------------------------------------------------------

static const char* get_key(const void* ptr)
{
   const SimMeasure* name = reinterpret_cast<const SimMeasure*>(ptr) ;
   return name->name ;
}

//----------------------------------------------------------------------------

ListPtr enumerate_vector_measure_names(const char* prefix)
{
   PrefixMatcher matcher(get_key,next_name) ;
   if (prefix && *prefix)
      return matcher.enumerateMatches(prefix,measure_names) ;
   else
      return matcher.enumerateKeys(measure_names) ;
}

//----------------------------------------------------------------------------

VectorSimilarityMeasure parse_vector_measure_name(const char* name)
{
   if (!name || !*name)
      return VectorSimilarityMeasure::none ;
   PrefixMatcher matcher(get_key,next_name) ;
   const void* found = matcher.match(name,measure_names) ;
   return found ? reinterpret_cast<const SimMeasure*>(found)->measure : VectorSimilarityMeasure::none ;
}


} // end namespace Fr

// end of file vecsim_name.C //
