/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-08-05					*/
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

#ifndef _Fr_GRAPH_H_INCLUDED
#define _Fr_GRAPH_H_INCLUDED

#include "framepac/itempool.h"
#include "framepac/object.h"
#include "framepac/smartptr.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename Vidx, typename L = float>
class GraphEdgeT
   {
   public:
      GraphEdgeT() {}
      GraphEdgeT(Vidx from_vertex, Vidx to_vertex, L len)
	 : m_from(from_vertex), m_to(to_vertex), m_length(len) {}
      ~GraphEdgeT() = default ;

      Vidx from() const { return m_from ; }
      Vidx to() const { return m_to ; }
      L length() const { return m_length ; }

   protected:
      Vidx m_from ;
      Vidx m_to ;
      L m_length ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename Vidx>
class EdgeListT
   {
   public:
      EdgeListT(Vidx cap = 0) : m_edges(cap), m_size(0), m_capacity(cap) {}
      ~EdgeListT() = default ;

      // accessors
      size_t size() const { return m_size ; }
      size_t capacity() const { return m_capacity ; }
      Vidx edge(size_t N) const { return m_edges[N] ; }

      // manipulators
      bool addEdge(Vidx edgenum) ;

   protected:
      Owned<Vidx> m_edges ;
      Vidx        m_size ;
      Vidx        m_capacity ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename V,typename Vidx, typename L>
class SubgraphT ;

template <typename V, typename Vidx, typename L>
class GraphT
   {
   public:
      typedef GraphEdgeT<Vidx,L> Edge ;
      typedef EdgeListT<Vidx> EdgeList ;
      typedef SubgraphT<V,Vidx,L> Subgraph ;
   public:
      GraphT()
	 {
	 }
      ~GraphT()
	 {
	 }
      // mutators
      bool makeUndirected()
	 {
	    if (numEdges() == 0) { m_directed = false ; return true ; }
	    return false ; 		// error to change after edges have been added
	 }
      bool reserve(Vidx extra_cap) ;
      Vidx addVertex(V vertex) ;
      void addEdge(Vidx from, Vidx to, L length) ;
      void shrink_to_fit() ;
      void optimize() ;
      void computeEdgeMatrix() ;

      // accessors
      size_t size() const { return m_size ; }
      size_t capacity() const { return m_capacity ; }
      size_t numVertices() const { return m_size ; }
      size_t numEdges() const { return m_total_edges ; }
      bool isDirected() const { return m_directed ; }
      V& vertex(Vidx v) const { return m_vertices[v] ; }
      Edge& edge(size_t e) const { return m_edges[e] ; }
      Edge& outboundEdge(size_t v, size_t e) const { return m_edges[m_outbound[v].edge(e)] ; }
      Edge& inboundEdge(size_t v, size_t e) const { return m_edges[m_inbound[v].edge(e)] ; }
      bool haveEdge(Vidx from, Vidx to) const ;

      // iterator support
      const V* begin() const { return m_vertices ; }
      const V* end() const { return m_vertices + size() ; }
      const Edge* beginE() const { return m_edges ; }
      const Edge* endE() const { return m_edges + numEdges() ; }

      // algorithms
      // return a new subgraph containing only the listed vertices
      Owned<Subgraph> makeSubgraph(const Vidx* vertex_list) const ;
      // return an array of subgraphs such that each has roughly the same number of vertices
      //    (used by parallel algorithms)
      Subgraph** split(size_t num_segments) const ;
      // return a list of edges forming a minimum spanning tree over the graph
      NewPtr<Vidx> minSpanningTreePrims() const ;

   protected:
      V*             m_vertices ;		// array of vertices
      ItemPool<Edge> m_edges ;			// array of edges
      EdgeList*      m_outbound ;		// list of outbound edge-indices for each vertex
      EdgeList*      m_inbound ;		// list of inbound edge-indices for each vertex
      bool*          m_edge_matrix { nullptr };	// used by haveEdge()
      size_t         m_total_edges ;
      size_t         m_alloc_edges ;
      Vidx           m_size ; 			// number of vertices
      Vidx           m_capacity ;		// number of vertex slots allocated in m_outbound and m_inbound
      bool           m_directed  { true } ;
   } ;

typedef GraphT<Object*,uint32_t,float> Graph ;

/************************************************************************/
/************************************************************************/

template <typename V,typename Vidx, typename L>
class SubgraphT
   {
   public:
      typedef GraphT<V,Vidx,L> Graph ;
      typedef typename Graph::EdgeList EdgeList ;
      typedef typename Graph::Edge Edge ;
   public:
      SubgraphT() ;
      SubgraphT(Graph* parent, const Vidx* vertices, size_t num_vertices) ;
      SubgraphT(Graph* parent, Vidx first, Vidx past_last) ; // make subgraph from contiguous range of vertices
      ~SubgraphT() ;

      // accessors
      size_t numVertices() const { return m_num_vertices ; }

      // return a list of edges forming a minimum spanning tree over the graph
      NewPtr<Vidx> minSpanningTreePrims() const ;

   private:
      void setEdges() ;

   private:
      GraphT<V,Vidx,L>* m_parent ;
      Vidx*     m_vertices ;		// which vertices of the parent are part of the subgraph?
      EdgeList* m_outbound ;		// outbound edges (within subgraph) for each vertex
      EdgeList* m_inbound ;		// inbound edges (within subgraph) for each vertex
      EdgeList* m_leaving_edges ;	// the edges leading to vertices outside the subgraph, for each vertex
      Vidx      m_num_vertices ;
   } ;

typedef SubgraphT<Object*,uint32_t,float> Subgraph ;
extern template class SubgraphT<Object*,uint32_t,float> ;

/************************************************************************/
/************************************************************************/


}  // end namespace Fr


#endif /* !_Fr_GRAPH_H_INCLUDED */

// end of file graph.h //

