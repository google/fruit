/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SEMISTATIC_GRAPH_H
#define SEMISTATIC_GRAPH_H

#include "semistatic_map.h"

namespace fruit {
namespace impl {

/**
 * A direct graph implementation where most of the graph is fixed at construction time, but a few nodes and edges can be added
 * later.
 * 
 * Also, nodes can either be normal nodes or terminal nodes. Terminal nodes can't have outgoing edges. Note that a node with no
 * outgoing edges may or may not be marked as terminal.
 * 
 * While insertion of elements after construction is supported, inserting or changing the neighbors of more than O(1) nodes
 * after construction will raise the cost of any further operations to more than O(1).
 * 
 * Even though adding nodes/edges after construction is inefficient, it is efficient to turn non-terminal nodes into terminal ones
 * (and therefore removing all the outgoing edges from the node) after construction.
 */
template <typename NodeId, typename Node>
class SemistaticGraph {
private:
  // The node data for nodeId is in nodes[nodeIndexMap.at(nodeId)].
  // To avoid hash table lookups, the edges in edgesStorage are stored as indexes of `nodes' instead of as NodeIds.
  // nodeIndexMap contains all known NodeIds, including ones known only due to an outgoing edge ending there from another node.
  SemistaticMap<NodeId, std::size_t> nodeIndexMap;
  
  static constexpr std::size_t invalidEdgesBeginOffset = ~std::size_t(0);
  
  struct NodeData {
#ifndef NDEBUG
    NodeId key;
#endif
    
    Node node;
    // (edgesStorage.begin() + edgesBeginOffset) is the beginning of the edges range.
    // If edgesBeginOffset==0, this is a terminal node.
    // If edgesBeginOffset==invalidEdgesBeginOffset, this node doesn't exist, it's just referenced by another node.
    std::size_t edgesBeginOffset;
  };
  
  std::vector<NodeData> nodes;
  
  // Stores vectors of dependencies as contiguous chunks of elements.
  // The NormalizedBindingData elements in typeRegistry contain indexes into this vector.
  // The first element is unused.
  std::vector<std::size_t> edgesStorage;
  
  std::size_t getOrAllocateNodeIndex(NodeId nodeId);
  
public:
  
  class edge_iterator;
  
  class node_iterator {
  private:
    typename std::vector<NodeData>::iterator itr;
    
    friend class SemistaticGraph<NodeId, Node>;
    
    node_iterator(typename std::vector<NodeData>::iterator itr);
    
  public:
    Node& getNode();
    
    bool isTerminal();
    
    // Turns the node into a terminal node, also removing all the deps.
    void setTerminal();
  
    // Assumes !isTerminal().
    // neighborsEnd() is NOT provided/stored for efficiency, the client code is expected to know the number of neighbors.
    edge_iterator neighborsBegin(SemistaticGraph<NodeId, Node>& graph);
    
    bool operator==(const node_iterator&) const;
  };
  
  class edge_iterator {
  private:
    // Iterator on edgesStorage.
    typename std::vector<std::size_t>::iterator itr;
    
    friend class SemistaticGraph<NodeId, Node>;
    friend class SemistaticGraph<NodeId, Node>::node_iterator;
    
    edge_iterator(typename std::vector<std::size_t>::iterator itr);

  public:
    node_iterator getNodeIterator(SemistaticGraph<NodeId, Node>& graph);
    
    void operator++();
    
    // Equivalent to i times operator++ followed by getNodeIterator(graph).
    node_iterator getNodeIterator(std::size_t i, SemistaticGraph<NodeId, Node>& graph);
  };
  
  // Constructs an *invalid* graph (as if this graph was just moved from).
  SemistaticGraph() = default;
  
  // A value x obtained dereferencing a NodeIter::value_type must support the following operations:
  // * x.getId(), returning a NodeId
  // * x.getValue(), returning a Node
  // * x.isTerminal(), returning a bool
  // * x.getEdgesBegin() and x.getEdgesEnd(), that if !x.isTerminal() define a range of values of type NodeId (the outgoing edges).
  // 
  // This constructor is *not* defined in semistatic_graph.templates.h, but only in semistatic_graph.cc.
  // All instantiations must have a matching instantiation in semistatic_graph.cc.
  template <typename NodeIter>
  SemistaticGraph(NodeIter first, NodeIter last);
  
  SemistaticGraph(const SemistaticGraph&) = default;
  SemistaticGraph(SemistaticGraph&&) = default;
  
  SemistaticGraph& operator=(const SemistaticGraph&) = default;
  SemistaticGraph& operator=(SemistaticGraph&&) = default;
  
  node_iterator end();
  
  // Precondition: `nodeId' must exist in the graph.
  // Unlike std::map::at(), this yields undefined behavior if the precondition isn't satisfied (instead of throwing).
  node_iterator at(NodeId nodeId);
  
  // Prefer using at() when possible, this is slightly slower.
  // Returns end() if the node ID was not found.
  node_iterator find(NodeId nodeId);
    
  // Sets nodeId as a non-terminal node with outgoing edges [edgesBegin, edgesEnd).
  // If the node already exists, combine(oldNode, newNode) is called and the result (also of type Node) is used as the node value.
  template <typename EdgeIter, typename Combine>
  void setNode(NodeId nodeId, Node node, EdgeIter edgesBegin, EdgeIter edgesEnd, Combine combine);
  
  // Sets nodeId as a terminal node.
  // If the node already exists, combine(oldNode, newNode) is called and the result (also of type Node) is used as the node value.
  // For cases where the node is known to exist, use the setTerminal() method on an iterator, it's faster.
  template <typename Combine> 
  void setTerminalNode(NodeId nodeId, Node node, Combine combine);
  
#ifndef NDEBUG
  // Emits a runtime error if some node was not created but there is an edge pointing to it.
  void checkFullyConstructed();
#endif
};

} // namespace impl
} // namespace fruit

#include "semistatic_graph.defn.h"

// semistatic_graph.templates.h is not included here to limit the transitive includes. Include it explicitly (in .cpp files).

#endif // SEMISTATIC_GRAPH_H
