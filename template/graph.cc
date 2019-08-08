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

#include <limits>
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
   // split into subgraphs
   //   eight times as many as hardware threads (unless the graph is really small)
   // dispatch the subgraphs into a thread pool to generate MSTs over each subgraph
   // treat the subgraphs as single vertices, and runs Prim's algo to find an MST over the subgraph-vertices
   // merge the list of edges forming the MSTs over individual subgraphs and the edges forming the subgraph-level
   //   MST to create the final MST of the full graph
   return nullptr ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
bool GraphT<V,Vidx,L>::PrimsAlgoSegment(Vidx first, Vidx past_last, Vidx* MST) const
{
   if (past_last <= first)
      return false ;			// no vertices in the specified segment
   Vidx num_vertices = past_last - first ;
   Vidx unknown { std::numeric_limits<Vidx>::max() } ;
   L infinity { std::numeric_limits<L>::has_infinity ? std::numeric_limits<L>::infinity() : std::numeric_limits<L>::max() } ;
   // step 1: for each vertex, initialize shortest edge to NULL and min known edge length to infinity
   NewPtr<bool> picked(num_vertices) ;
   std::fill_n(&picked,num_vertices,false) ;
   NewPtr<Vidx> shortest_arcs(num_vertices) ;
   std::fill_n(&shortest_arcs,num_vertices,unknown) ;
   NewPtr<L> min_lengths(num_vertices) ;
   std::fill_n(&min_lengths,num_vertices,infinity) ;
   // step 2: select an arbitrary vertex among those tied for shortest known edge -- we'll pick the very first
   Vidx curr_v = first ;
   size_t MST_edges { 0 } ;
   while (MST_edges + 1 < num_vertices)
      {
      Vidx min_neighbor = unknown ;
      L min_length = infinity ;
      // step 3: for every edge leaving the selected vertex going to an unpicked vertex,
      //         if the length is lower than the other's min known length, update its min length and set its
      //         min edge to be the one back to the current vertex
      for (auto arc_index : m_outbound[curr_v])
	 {
	 auto e = edge(arc_index) ;
	 Vidx other = (e.from() == curr_v) ? e.to() : e.from() ;
	 if (other < first || other >= past_last)
	    continue ;			// other vertex is not in the current subgraph
	 other -= first ;
	 if (picked[other])
	    continue ;			// other vertex is already part of the MST
	 L len = e.length() ;
	 if (len < min_lengths[other])
	    {
	    min_lengths[other] = len  ;
	    shortest_arcs[other] = arc_index ;
	    }
	 if (len < min_length)
	    {
	    min_length = len ;
	    min_neighbor = other ;
	    }
	 }
      // step 4: pick the neighbor vertex with the lowest min length, and add it and its stored edge to the MST
      //         since we tracked the minimums during the update step, we already have it picked
      MST[MST_edges++] = shortest_arcs[min_neighbor] ;
      // step 5: mark that selected neighbor as picked and make it the new current vertex
      picked[min_neighbor] = true ;
      curr_v = min_neighbor + first ;
      }
   return true ;
}

//----------------------------------------------------------------------

template <typename V, typename Vidx, typename L>
bool GraphT<V,Vidx,L>::PrimsAlgoMerge(Vidx subgraph_size, Vidx num_subgraphs, Vidx* MST) const
{
   if (subgraph_size == 1 || num_subgraphs < 2)
      return false ;			// MST is already complete
   Vidx unknown { std::numeric_limits<Vidx>::max() } ;
   L infinity { std::numeric_limits<L>::has_infinity ? std::numeric_limits<L>::infinity() : std::numeric_limits<L>::max() } ;
   // step 1: for each vertex, initialize shortest edge to NULL and min known edge length to infinity
   NewPtr<bool> picked(num_subgraphs) ;
   std::fill_n(&picked,num_subgraphs,false) ;
   NewPtr<Vidx> shortest_arcs(num_subgraphs) ;
   std::fill_n(&shortest_arcs,num_subgraphs,unknown) ;
   NewPtr<L> min_lengths(num_subgraphs) ;
   std::fill_n(&min_lengths,num_subgraphs,infinity) ;
   // step 2: select an arbitrary vertex among those tied for shortest known edge -- we'll pick the very first
   Vidx curr_v = 0 ;
   size_t MST_edges { 0 } ;
   while (MST_edges + 1 < num_subgraphs)
      {
      Vidx min_neighbor = unknown ;
      L min_length = infinity ;
      // step 3: for every edge leaving the selected subgraph going to an unpicked subgraph,
      //         if the length is lower than the other's min known length, update its min length and set its
      //         min edge to be the one back to the current vertex
      for (Vidx v = curr_v ; v < curr_v + subgraph_size && v < numVertices() ; ++v)
	 {
	 for (auto arc_index : m_outbound[v])
	    {
	    auto e = edge(arc_index) ;
	    Vidx other = (e.from() == v) ? e.to() : e.from() ;
	    if (other >= curr_v && other < curr_v + subgraph_size)
	       continue ;			// other vertex is in the current subgraph
	    other /= subgraph_size ;
	    if (picked[other])
	       continue ;			// other subgraph is already part of the MST
	    L len = e.length() ;
	    if (len < min_lengths[other])
	       {
	       min_lengths[other] = len  ;
	       shortest_arcs[other] = arc_index ;
	       }
	    if (len < min_length)
	       {
	       min_length = len ;
	       min_neighbor = other ;
	       }
	    }
	 }
      // step 4: pick the neighbor subgraph with the lowest min length, and add it and its stored edge to the MST
      //         since we tracked the minimums during the update step, we already have it picked
      MST[MST_edges++] = shortest_arcs[min_neighbor] ;
      // step 5: mark that selected subgraph as picked and make it the new current subgraph
      picked[min_neighbor] = true ;
      curr_v = min_neighbor * subgraph_size ;
      }
   return true ;
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


} // end namespace Fr

// end of file graph.cc //
