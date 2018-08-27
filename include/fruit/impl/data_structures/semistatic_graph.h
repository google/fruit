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

#include "memory_pool.h"
#include <fruit/impl/data_structures/semistatic_map.h>

#if FRUIT_EXTRA_DEBUG
#include <iostream>
#endif

namespace fruit {
namespace impl {

// The alignas ensures that a SemistaticGraphInternalNodeId* always has 0 in the low-order bit.
struct alignas(2) alignas(alignof(std::size_t)) SemistaticGraphInternalNodeId {
  // This stores the index in the vector times sizeof(NodeData).
  std::size_t id;

  bool operator==(const SemistaticGraphInternalNodeId& x) const;
  bool operator<(const SemistaticGraphInternalNodeId& x) const;
};

/**
 * A direct graph implementation where most of the graph is fixed at construction time, but a few nodes and edges can be
 * added
 * later.
 *
 * Also, nodes can either be normal nodes or terminal nodes. Terminal nodes can't have outgoing edges. Note that a node
 * with no
 * outgoing edges may or may not be marked as terminal.
 *
 * While insertion of elements after construction is supported, inserting or changing the neighbors of more than O(1)
 * nodes
 * after construction will raise the cost of any further operations to more than O(1).
 *
 * Even though adding nodes/edges after construction is inefficient, it is efficient to turn non-terminal nodes into
 * terminal ones
 * (and therefore removing all the outgoing edges from the node) after construction.
 *
 * NodeId and Node must be default constructible and trivially copyable.
 */
template <typename NodeId, typename Node>
class SemistaticGraph {
private:
  using InternalNodeId = SemistaticGraphInternalNodeId;

  // The node data for nodeId is in nodes[node_index_map.at(nodeId)/sizeof(NodeData)].
  // To avoid hash table lookups, the edges in edges_storage are stored as indexes of `nodes' instead of as NodeIds.
  // node_index_map contains all known NodeIds, including ones known only due to an outgoing edge ending there from
  // another node.
  SemistaticMap<NodeId, InternalNodeId> node_index_map;

  struct NodeData {
#if FRUIT_EXTRA_DEBUG
    NodeId key;
#endif

  public:
    // If edges_begin==0, this is a terminal node.
    // If edges_begin==1, this node doesn't exist, it's just referenced by another node.
    // Otherwise, reinterpret_cast<InternalNodeId*>(edges_begin) is the beginning of the edges range.
    std::uintptr_t edges_begin;

    // An explicit "public" specifier here prevents the compiler from reordering the fields.
    // We want the edges_begin field to be stored first because that's what we're going to branch on.
  public:
    Node node;
  };

  std::size_t first_unused_index;

  FixedSizeVector<NodeData> nodes;

  // Stores vectors of edges as contiguous chunks of node IDs.
  // The NodeData elements in `nodes' contain indexes into this vector (stored as already multiplied by
  // sizeof(NodeData)).
  // The first element is unused.
  FixedSizeVector<InternalNodeId> edges_storage;

#if FRUIT_EXTRA_DEBUG
  template <typename NodeIter>
  void printGraph(NodeIter first, NodeIter last);
#endif

  NodeData* nodeAtId(InternalNodeId internalNodeId);
  const NodeData* nodeAtId(InternalNodeId internalNodeId) const;

  static NodeData* nodeAtId(NodeData* nodes_begin, InternalNodeId internalNodeId);
  static const NodeData* nodeAtId(const NodeData* nodes_begin, InternalNodeId internalNodeId);

public:
  class edge_iterator;

  class node_iterator {
  private:
    NodeData* itr;

    friend class SemistaticGraph<NodeId, Node>;

    node_iterator(NodeData* itr);

  public:
    Node& getNode();

    bool isTerminal();

    // Turns the node into a terminal node, also removing all the deps.
    void setTerminal();

    // Assumes !isTerminal().
    // neighborsEnd() is NOT provided/stored for efficiency, the client code is expected to know the number of
    // neighbors.
    edge_iterator neighborsBegin();

    bool operator==(const node_iterator&) const;
  };

  class const_node_iterator {
  private:
    const NodeData* itr;

    friend class SemistaticGraph<NodeId, Node>;

    const_node_iterator(const NodeData* itr);

  public:
    const_node_iterator(node_iterator itr);

    const Node& getNode();

    bool isTerminal();

    bool operator==(const const_node_iterator&) const;
  };


  class edge_iterator {
  private:
    // Iterator on edges_storage.
    InternalNodeId* itr;

    friend class SemistaticGraph<NodeId, Node>;
    friend class SemistaticGraph<NodeId, Node>::node_iterator;

    edge_iterator(InternalNodeId* itr);

  public:
    // getNodeIterator(graph.nodes.begin()) returns the first neighbor.
    node_iterator getNodeIterator(node_iterator nodes_begin);

    void operator++();

    // Equivalent to i times operator++ followed by getNodeIterator(nodes_begin).
    node_iterator getNodeIterator(std::size_t i, node_iterator nodes_begin);
  };

  // Constructs an *invalid* graph (as if this graph was just moved from).
  SemistaticGraph() = default;

  /**
   * A value x obtained dereferencing a NodeIter::value_type must support the following operations:
   * - x.getId(), returning a NodeId
   * - x.getValue(), returning a Node
   * - x.isTerminal(), returning a bool
   * - x.getEdgesBegin() and x.getEdgesEnd(), that if !x.isTerminal() define a range of values of type NodeId
   *   (the outgoing edges).
   *
   * This constructor is *not* defined in semistatic_graph.templates.h, but only in semistatic_graph.cc.
   * All instantiations must have a matching instantiation in semistatic_graph.cc.
   *
   * The MemoryPool is only used during construction, the constructed object *can* outlive the memory pool.
   */
  template <typename NodeIter>
  SemistaticGraph(NodeIter first, NodeIter last, MemoryPool& memory_pool);

  SemistaticGraph(SemistaticGraph&&) = default;
  SemistaticGraph(const SemistaticGraph&) = delete;

  /**
   * Creates a copy of x with the additional nodes in [first, last). The requirements on NodeIter as the same as for the
   * 2-arg
   * constructor.
   * The nodes in [first, last) must NOT be already in x, but can be neighbors of nodes in x.
   * The new graph will share data with `x', so must be destroyed before `x' is destroyed.
   * Also, after this is called, `x' must not be modified until this object has been destroyed.
   *
   * The MemoryPool is only used during construction, the constructed object *can* outlive the memory pool.
   */
  template <typename NodeIter>
  SemistaticGraph(const SemistaticGraph& x, NodeIter first, NodeIter last, MemoryPool& memory_pool);

  ~SemistaticGraph();

  SemistaticGraph& operator=(const SemistaticGraph&) = delete;
  SemistaticGraph& operator=(SemistaticGraph&&) = default;

  // The result is unspecified. The only guarantee is that it's the right value to pass to edge_iterator's
  // getNodeIterator() methods.
  node_iterator begin();

  node_iterator end();
  const_node_iterator end() const;

  // Precondition: `nodeId' must exist in the graph.
  // Unlike std::map::at(), this yields undefined behavior if the precondition isn't satisfied (instead of throwing).
  node_iterator at(NodeId nodeId);

  // Prefer using at() when possible, this is slightly slower.
  // Returns end() if the node ID was not found.
  node_iterator find(NodeId nodeId);
  const_node_iterator find(NodeId nodeId) const;

#if FRUIT_EXTRA_DEBUG
  // Emits a runtime error if some node was not created but there is an edge pointing to it.
  void checkFullyConstructed();
#endif
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/data_structures/semistatic_graph.defn.h>

// semistatic_graph.templates.h is not included here to limit the transitive includes. Include it explicitly (in .cpp
// files).

#endif // SEMISTATIC_GRAPH_H
