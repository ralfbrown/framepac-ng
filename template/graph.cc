/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-08-06					*/
/*	by Ralf Brown <ralf@cs.cmu.edu>					*/
/*									*/
/* (c) Copyright 2019 Carnegie Mellon University			*/
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

#include <numeric>
#include "framepac/graph.h"

namespace Fr
{

/************************************************************************/
/*	Methods for template class EdgeListT				*/
/************************************************************************/

template <typename Vidx>
bool EdgeListT<Vidx>::addEdge(Vidx edgenum)
{
   (void)edgenum ; //TODO
   return true ;
}

/************************************************************************/
/*	Methods for template class GraphT				*/
/************************************************************************/

template <typename V, typename Vidx, typename L>
bool GraphT<V,Vidx,L>::reserve(Vidx extra_cap)
{
   if (!extra_cap)
      return true ;
   //TODO
   return false ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
Vidx GraphT<V,Vidx,L>::addVertex(V vertex)
{
   (void)vertex ; //TODO
   return 0 ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
void GraphT<V,Vidx,L>::addEdge(Vidx from, Vidx to, L length)
{

   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
void GraphT<V,Vidx,L>::shrink_to_fit()
{

   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
bool GraphT<V,Vidx,L>::haveEdge(Vidx from, Vidx to) const
{
   return m_edge_matrix ? m_edge_matrix[numVertices()*from + to] : false ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
void GraphT<V,Vidx,L>::optimize()
{
   
   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
void GraphT<V,Vidx,L>::computeEdgeMatrix()
{

   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
Owned<typename GraphT<V,Vidx,L>::Subgraph> GraphT<V,Vidx,L>::makeSubgraph(const Vidx* vertices) const
{
   return new Subgraph(this,vertices) ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
typename GraphT<V,Vidx,L>::Subgraph** GraphT<V,Vidx,L>::split(size_t num_segments) const
{
   (void)num_segments ; //TODO
   return nullptr ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
NewPtr<Vidx> GraphT<V,Vidx,L>::minSpanningTreePrims() const
{
   //TODO
   return nullptr ;
}

/************************************************************************/
/*	Methods for template class SubgraphT				*/
/************************************************************************/

template <typename V, typename Vidx, typename L>
SubgraphT<V,Vidx,L>::SubgraphT()
   : m_parent(nullptr), m_vertices(nullptr), m_outbound(nullptr), m_inbound(nullptr), m_leaving_edges(nullptr),
     m_num_vertices(0)
{
   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
SubgraphT<V,Vidx,L>::SubgraphT(GraphT<V,Vidx,L>* parent, const Vidx* vertices, size_t num_vertices)
   : m_parent(parent), m_num_vertices(num_vertices)
{
   m_vertices = new Vidx[num_vertices] ;
   std::copy_n(vertices,num_vertices,m_vertices) ;
   setEdges() ;
   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
SubgraphT<V,Vidx,L>::SubgraphT(GraphT<V,Vidx,L>* parent, Vidx first, Vidx past_last)
   : m_parent(parent), m_num_vertices(past_last-first)
{
   m_vertices = new Vidx[m_num_vertices] ;
   std::iota(m_vertices,m_vertices+m_num_vertices,first) ;
   setEdges() ;
   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
void SubgraphT<V,Vidx,L>::setEdges()
{
   m_outbound = new EdgeList[m_num_vertices] ;
   m_inbound = new EdgeList[m_num_vertices] ;
   m_leaving_edges = new EdgeList[m_num_vertices] ;
   //TODO

   return ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
NewPtr<Vidx> SubgraphT<V,Vidx,L>::minSpanningTreePrims() const
{
   NewPtr<Vidx> shortest_arcs(m_num_vertices) ;
   //TODO
   return nullptr ;
}


} // end namespace Fr

// end of file graph.cc //
