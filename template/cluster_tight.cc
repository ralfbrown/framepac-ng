/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.08, last edit 2018-08-15					*/
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

#include "framepac/cluster.h"
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
   } ;

/************************************************************************/
/************************************************************************/

template <typename IdxT, typename ValT>
ClusterInfo* ClusteringAlgoTight<IdxT,ValT>::cluster(const Array* vectors) const
{
   // trap signals to allow graceful early termination
   this->trapSigInt() ;
   
   (void)vectors;
   // cleanup: untrap signals
   this->untrapSigInt() ;
   return nullptr ; //TODO
}

} // end of namespace Fr

// end of file cluster_tight.C //
