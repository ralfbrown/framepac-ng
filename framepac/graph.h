/****************************** -*- C++ -*- *****************************/
/*									*/
/* FramepaC-ng								*/
/* Version 0.15, last edit 2019-04-18					*/
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

#include "framepac/builder.h"
#include "framepac/object.h"

/************************************************************************/
/************************************************************************/

namespace Fr
{

/************************************************************************/
/************************************************************************/

template <typename T>
class GraphEdgeT<T>
   {
   public:
      GraphEdgeT() {}
      GraphEdgeT(T from_vertex, T to_vertex) : m_from(from_vertext), m_to(to_vertex) {}
      ~GraphEdgeT() = default ;

      T from() const { return m_from ; }
      T to() const { return m_to ; }
      
   protected:
      T m_from ;
      T m_to ;
   } ;

typedef GraphEdgeT<GraphNode*> GraphEdge ;
typedef GraphEdgeT<uint16_t> GraphEdge16 ;
typedef GraphEdgeT<uint32_t> GraphEdge32 ;
typedef GraphEdgeT<uint64_t> GraphEdge64 ;

/************************************************************************/
/************************************************************************/

template <typename T, typename L>
class GraphEdgeLengthT<T> : public GraphEdgeT<T>
   {
   public:
      typedef GraphEdgeT<T> super ;
   public:
      GraphEdgeLengthT() : super() {}
      GraphEdgeLengthT(T from_vertex, T to_vertex, L len) : super(from_vertex,to_vertex), m_length(len) {}
      ~GraphEdgeLengthT() = default ;

      L length() const { return m_length ; }
   protected:
      L m_length ;
   } ;

typedef GraphEdgeLengthT<GraphNode*,double> GraphEdgeLength ;
typedef GraphEdgeLengthT<uint32_t,float> GraphEdge32Length ;

/************************************************************************/
/************************************************************************/

template <typename T = Object*>
class GraphVertex<T>
   {
   public:
      GraphVertex() {}
      GraphVertex(const T vert) : m_vertex(vert) {}
      ~GraphVertex() = default ;

      T& vertex() const { return &m_vertex ; }
   protected:
      T m_vertex ;
   } ;

/************************************************************************/
/************************************************************************/

template <typename V, typename Eidx, typename L>
class GraphT<V,Eidx,L>
   {
   public:
      typedef GraphVertex<V> Vertex ;
      typedef GraphEdgeLengthT<Eidx,L> Edge ;
      typedef BufferBuilder<Vertex,1> VertexList ;
      typedef BufferBuilder<Edge,1> EdgeList ;
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
      void addVertex(V vertex) ;
      void addEdge(const Edge& edge) ;
      void addEdge(Eidx from, Eidx to) ;

      // accessors
      bool isDirected() const { return m_directed ; }
      bool isSubgraph() const { return m_is_subgraph ; }
      size_t numVertices() const { return m_vertices.currentLength() ; }
      size_t numEdges() const { return m_edges.currentLength() ; }
      V* vertex(size_t index) const { return m_vertices[index] ; }
      Edge* edge(size_t index) const { return m_edges[index] ; }

      // iterator support
      const Vertex* beginVert() const { return m_vertices.begin() ; }
      const Vertex* endVert() const { return m_vertices.end() ; }
      const Edge* beginEdge() const { return m_edges.begin() ; }
      const Edge* endEdge() const { return m_edges.end() ; }

      // algorithms
      // return an array of subgraphs such that each has roughly the same number of vertices
      //    (used by parallel algorithms)
      GraphT** split(size_t num_segments) const ;
      // return a list of edges forming a minimum spanning tree over the graph
      EdgeList&& minSpanningTreePrims() const ;

   protected:
      VertexList m_vertices ;
      EdgeList m_edges ;
      EdgeList m_leaving_edges ;	// in a subgraph, the edges leading to vertices outside the subgraph
      bool m_directed  { true } ;
      bool m_is_subgraph { false } ;
   } ;

typedef GraphT<typename T,uint32_t,float> Graph ;

/************************************************************************/
/************************************************************************/


}  // end namespace Fr


#endif /* !_Fr_GRAPH_H_INCLUDED */

// end of file graph.h //

