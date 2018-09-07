/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.11, last edit 2018-09-06					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017,2018 Carnegie Mellon University			*/
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

#ifndef __FrRANDOM_H_INCLUDED
#define __FrRANDOM_H_INCLUDED

#include <random>
#include "framepac/smartptr.h"

using namespace std ;

namespace Fr {

/************************************************************************/
/************************************************************************/

// return a uniformly-distributed random integer from 0 to (range-1), inclusive
class RandomInteger
   {
   public:
      RandomInteger(size_t range) ;
      RandomInteger(size_t min, size_t max) ;
      ~RandomInteger() {}

      size_t get() ;
      size_t operator() () { return get() ; }

      void seed() ;			// use default seed
      void seed(size_t s) ;
      void randomize() ;

   private:
      uniform_int_distribution<size_t> m_dist ;
   } ;

/************************************************************************/
/************************************************************************/

// return a uniformly-distributed random real from 0 (inclusive) to range (exclusive)
class RandomFloat
   {
   public:
      RandomFloat() ;   		// generate numbers in [0..1)
      RandomFloat(double range) ;	// generate numbers in [0..range)
      RandomFloat(double min, double max) ;
      ~RandomFloat() {}

      double get() ;
      double operator() () { return get() ; }

      void seed() ;			// use default seed
      void seed(size_t s) ;
      void randomize() ;

   private:
      uniform_real_distribution<double> m_dist ;
   } ;

/************************************************************************/
/************************************************************************/

// (re)seed the global random number generator used by the functions below
void Randomize() ;
void Randomize(size_t seed) ;

NewPtr<bool> RandomSample(size_t total, size_t sample) ;

//----------------------------------------------------------------------------

} // end namespace Fr

#endif /* !__FrRANDOM_H_INCLUDED */

// end of file random.h //
