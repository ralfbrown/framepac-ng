/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.05, last edit 2018-04-17					*/
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

#include <csignal>  // for sig_atomic_t and __sighandler_t
#include "framepac/cluster.h"
#include "framepac/message.h"
#include "framepac/signal.h"
using namespace Fr ;

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
class ClusteringAlgoTight : public ClusteringAlgo<IdxT,ValT>
   {
   public:
      virtual ~ClusteringAlgoTight() { delete this ; }
      virtual const char*algorithmName() const { return "Tight" ; }

      virtual ClusterInfo* cluster(const Array* vectors) const ;

   protected: // methods
      static void sigint_handler(int)
	 {
	 if (!abort_requested)
	    {
	    abort_requested = 1 ;
	    SystemMessage::status("*** User interrupt: clustering will be terminated after the current iteration ***");
	    }
	 }

   protected: // data members
      static SignalHandler* s_sigint ;
      static std::sig_atomic_t abort_requested ;
   } ;

template <typename IdxT, typename ValT>
SignalHandler* ClusteringAlgoTight<IdxT,ValT>::s_sigint = nullptr ;
template <typename IdxT, typename ValT>
std::sig_atomic_t ClusteringAlgoTight<IdxT,ValT>::abort_requested = 0 ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoTight<IdxT,ValT>::cluster(const Array* vectors) const
{
   // trap signals to allow graceful early termination
   s_sigint = new SignalHandler(SIGINT,sigint_handler) ;
   
   (void)vectors;
   // cleanup: untrap signals
   delete s_sigint ;
   s_sigint = nullptr ;
   return nullptr ; //TODO
}

} // end of namespace Fr

// end of file cluster_tight.C //
