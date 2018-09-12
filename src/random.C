/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.11, last edit 2018-09-11					*/
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

#include "framepac/critsect.h"
#include "framepac/message.h"
#include "framepac/random.h"

using namespace std ;

namespace Fr {

/************************************************************************/
/************************************************************************/

static random_device global_randomizer ;
static mt19937_64 global_random_engine ;
static CriticalSection rand_critsect ;

/************************************************************************/
/*	Seeding the random number engine				*/
/************************************************************************/

void Randomize()
{
   global_random_engine.seed(global_randomizer()) ;
   return ;
}

//----------------------------------------------------------------------------

void Randomize(size_t seed)
{
   global_random_engine.seed(seed) ;
   return ;
}

/************************************************************************/
/*	Methods for class RandomInteger					*/
/************************************************************************/

RandomInteger::RandomInteger(size_t range)
   : m_dist(0,range-1)
{
   return  ;
}

//----------------------------------------------------------------------------

RandomInteger::RandomInteger(size_t min, size_t max)
   : m_dist(min,max)
{
   return  ;
}

//----------------------------------------------------------------------------

size_t RandomInteger::get()
{
   ScopeCriticalSection guard(rand_critsect) ;
   return m_dist(global_random_engine) ;
}

//----------------------------------------------------------------------------

void RandomInteger::seed()
{
   global_random_engine.seed() ;
   return ;
}

//----------------------------------------------------------------------------

void RandomInteger::seed(size_t s)
{
   global_random_engine.seed(s) ;
   return ;
}

//----------------------------------------------------------------------------

void RandomInteger::randomize()
{
   global_random_engine.seed(global_randomizer()) ;
   return ;
}

/************************************************************************/
/*	Methods for class RandomFloat					*/
/************************************************************************/

RandomFloat::RandomFloat()
   : m_dist()
{
   return  ;
}

//----------------------------------------------------------------------------

RandomFloat::RandomFloat(double range)
   : m_dist(0,range)
{
   return  ;
}

//----------------------------------------------------------------------------

RandomFloat::RandomFloat(double min, double max)
   : m_dist(min,max)
{
   return  ;
}

//----------------------------------------------------------------------------

double RandomFloat::get()
{
   ScopeCriticalSection guard(rand_critsect) ;
   return m_dist(global_random_engine) ;
}

//----------------------------------------------------------------------------

void RandomFloat::seed()
{
   global_random_engine.seed() ;
   return ;
}

//----------------------------------------------------------------------------

void RandomFloat::seed(size_t s)
{
   global_random_engine.seed(s) ;
   return ;
}

//----------------------------------------------------------------------------

void RandomFloat::randomize()
{
   global_random_engine.seed(global_randomizer()) ;
   return ;
}

/************************************************************************/
/*	Random sampling							*/
/************************************************************************/

NewPtr<bool> RandomSample(size_t total, size_t sample)
{
   if (sample > total)
      return nullptr ;
   bool* selected = new bool[total+1] ;
   if (!selected)
      {
      // out of memory
      SystemMessage::no_memory("generating random sample") ;
      return nullptr ;
      }
   bool unsampled = false ;
   bool sampled = true ;
   // if we're asked to sample more than half of the total space, it's
   //   faster (due to fewer collisions) to select everything and then
   //   UNsample
   if (sample > total / 2)
      {
      unsampled = true ;
      sampled = false ;
      sample = total - sample ;
      }
   std::fill(selected,selected+total,unsampled) ;
   RandomInteger rand(total) ;
   for (size_t i = 1 ; i <= sample ; i++)
      {
      for ( ; ; )
	 {
	 size_t r = rand() ;
	 if (selected[r] == unsampled)
	    {
	    selected[r] = sampled ;
	    break ;
	    }
	 }
      }
   return selected ;
}


/************************************************************************/
/************************************************************************/

} // end namespace Fr

// end of file random.C //
