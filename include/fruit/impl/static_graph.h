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

#ifndef STATIC_GRAPH_H
#define STATIC_GRAPH_H

#include "semistatic_map.h"
#include "semistatic_graph.h"

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
class StaticGraph {
private:
  // Same type, but edgesBeginOffset now is a casted edgesBegin (and if equal to edgesStorage.begin(), the current node is
  // terminal).
  using NodeData = typename SemistaticGraph<NodeId, Node>::NodeData;
  
  std::vector<NodeData> nodes;
  
  // Semantically a NodeData*, but casted to uintptr_t so that we can move the vectors from the SemistaticGraph and save
  // allocations/copies.
  using NodeItr = std::uintptr_t;
  
  // Stores vectors of dependencies as contiguous chunks of elements.
  // The NormalizedBindingData elements in typeRegistry contain indexes into this vector.
  // The first element is unused.
  std::vector<NodeItr> edgesStorage;
  
  // Semantically a NodeItr*, but casted to uintptr_t so that we can move the vectors from the SemistaticGraph and save
  // allocations/copies.
  using EdgeItr = std::uintptr_t;

  
  // Maps each NodeId to an iterator into `nodes' pointing at the NodeData for that node.
  SemistaticMap<NodeId, NodeItr> nodesById;
    
public:
  
  class edge_iterator;
  
  class node_iterator {
  private:
    NodeItr itr;
    
    friend class StaticGraph<NodeId, Node>;
    
    node_iterator(NodeItr itr);
    
  public:
    Node& getNode();
    
    bool isTerminal();
    
    // Turns the node into a terminal node, also removing all the deps.
    void setTerminal();
  
    // Assumes !isTerminal().
    // neighborsEnd() is NOT provided/stored for efficiency, the client code is expected to know the number of neighbors.
    edge_iterator neighborsBegin();
    
    bool operator==(const node_iterator&) const;
  };
  
  class edge_iterator {
  private:
    // Iterator on edgesStorage.
    EdgeItr itr;
    
    friend class StaticGraph<NodeId, Node>;
    friend class StaticGraph<NodeId, Node>::node_iterator;
    
    edge_iterator(EdgeItr itr);

  public:
    node_iterator getNodeIterator();
    
    void operator++();
    
    // Equivalent to i times operator++ followed by getNodeIterator(graph).
    node_iterator getNodeIterator(std::size_t i);
  };
  
  StaticGraph(SemistaticGraph<NodeId, Node>&& graph);
  
  StaticGraph(const StaticGraph&) = delete;
  StaticGraph(StaticGraph&&) = delete;
  
  StaticGraph& operator=(const StaticGraph&) = delete;
  StaticGraph& operator=(StaticGraph&&) = delete;
  
  node_iterator end();
  
  // Precondition: `nodeId' must exist in the graph.
  // Unlike std::map::at(), this yields undefined behavior if the precondition isn't satisfied (instead of throwing).
  node_iterator at(NodeId nodeId);
  
  // Prefer using at() when possible, this is slightly slower.
  // Returns end() if the node ID was not found.
  node_iterator find(NodeId nodeId);
};

} // namespace impl
} // namespace fruit

#include "static_graph.inlines.h"

// static_graph.templates.h is not included here to limit the transitive includes. Include it explicitly (in .cpp files).

#endif // STATIC_GRAPH_H
