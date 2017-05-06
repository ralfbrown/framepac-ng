/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.01, last edit 2017-05-06					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2017 Carnegie Mellon University			*/
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

#ifndef __Fr_COUNTER_H_INCLUDED
#define __Fr_COUNTER_H_INCLUDED

#include "framepac/atomic.h"

namespace Fr
{

template <unsigned ways, unsigned N = 1>
class DistributedCounter
   {
   public:
      DistributedCounter() {}
      ~DistributedCounter() {}
      void clear(unsigned var)
         {
	    for (unsigned i = 0; i < ways; ++i)
	       m_counters[var].clear(var) ;
	 }
      long get(unsigned var) const
         {
	    long sum = 0 ;
	    for (unsigned i = 0; i < ways; ++i)
	       sum += m_counters[i].get(var) ;
	    return sum ;
	 }
      void incr(unsigned var, unsigned way, long increment = 1) { m_counters[way].incr(var,increment) ; }
      void decr(unsigned var, unsigned way, long decrement = 1) { m_counters[way].decr(var,decrement) ; }

   protected:
      static constexpr unsigned pad = N*sizeof(long) % std::hardware_destructive_interference_size ;
      class CounterArray
         {
	 public:
	    CounterArray() {}
	    ~CounterArray() {}
	    void clear(unsigned var) { m_counters[var].store(0,std::memory_order_release) ; }
	    long get(unsigned var) const { return m_counters[var].load(std::memory_order_consume) ; }
	    void incr(unsigned var, long increment) { m_counters[var] += increment ; }
	    void decr(unsigned var, long decrement) { m_counters[var] -= decrement ; }

	 protected:
	    Atomic<long> m_counters[N] ;
	    char         m_pad[pad] ;
	 } ;
      CounterArray m_counters[ways] ;
   } ;

} // end namespace Fr

#endif /* !__Fr_COUNTER_H_INCLUDED */

// end of file counter.h //
