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

#ifndef SEMISTATIC_GRAPH_DEFN_H
#define SEMISTATIC_GRAPH_DEFN_H

#include <fruit/impl/data_structures/semistatic_graph.h>

namespace fruit {
namespace impl {

inline bool SemistaticGraphInternalNodeId::operator==(const SemistaticGraphInternalNodeId& x) const {
  return id == x.id;
}

inline bool SemistaticGraphInternalNodeId::operator<(const SemistaticGraphInternalNodeId& x) const {
  return id < x.id;
}

template <typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::node_iterator::node_iterator(NodeData* itr) : itr(itr) {}

template <typename NodeId, typename Node>
inline Node& SemistaticGraph<NodeId, Node>::node_iterator::getNode() {
  FruitAssert(itr->edges_begin != 1);
  return itr->node;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::node_iterator::isTerminal() {
  FruitAssert(itr->edges_begin != 1);
  return itr->edges_begin == 0;
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::node_iterator::setTerminal() {
  FruitAssert(itr->edges_begin != 1);
  itr->edges_begin = 0;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::node_iterator::operator==(const node_iterator& other) const {
  return itr == other.itr;
}

template <typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::const_node_iterator::const_node_iterator(const NodeData* itr) : itr(itr) {}

template<typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::const_node_iterator::const_node_iterator(node_iterator itr) : itr(itr.itr) {}

template <typename NodeId, typename Node>
inline const Node& SemistaticGraph<NodeId, Node>::const_node_iterator::getNode() {
  FruitAssert(itr->edges_begin != 1);
  return itr->node;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::const_node_iterator::isTerminal() {
  FruitAssert(itr->edges_begin != 1);
  return itr->edges_begin == 0;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::const_node_iterator::operator==(const const_node_iterator& other) const {
  return itr == other.itr;
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::edge_iterator
SemistaticGraph<NodeId, Node>::node_iterator::neighborsBegin() {
  FruitAssert(itr->edges_begin != 0);
  FruitAssert(itr->edges_begin != 1);
  return edge_iterator{reinterpret_cast<InternalNodeId*>(itr->edges_begin)};
}

template <typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::edge_iterator::edge_iterator(InternalNodeId* itr) : itr(itr) {}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator
SemistaticGraph<NodeId, Node>::edge_iterator::getNodeIterator(node_iterator nodes_begin) {
  return node_iterator{nodeAtId(nodes_begin.itr, *itr)};
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::edge_iterator::operator++() {
  ++itr;
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator
SemistaticGraph<NodeId, Node>::edge_iterator::getNodeIterator(std::size_t i, node_iterator nodes_begin) {
  itr += i;
  return getNodeIterator(nodes_begin);
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::begin() {
  return node_iterator{nodes.begin()};
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::end() {
  return node_iterator{nodes.end()};
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::const_node_iterator SemistaticGraph<NodeId, Node>::end() const {
  return const_node_iterator{nodes.end()};
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::at(NodeId nodeId) {
  InternalNodeId internalNodeId = node_index_map.at(nodeId);
  return node_iterator{nodeAtId(internalNodeId)};
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::const_node_iterator
SemistaticGraph<NodeId, Node>::find(NodeId nodeId) const {
  const InternalNodeId* internalNodeIdPtr = node_index_map.find(nodeId);
  if (internalNodeIdPtr == nullptr) {
    return const_node_iterator{nodes.end()};
  } else {
    const NodeData* p = nodeAtId(*internalNodeIdPtr);
    if (p->edges_begin == 1) {
      return const_node_iterator{nodes.end()};
    }
    return const_node_iterator{p};
  }
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::find(NodeId nodeId) {
  const InternalNodeId* internalNodeIdPtr = node_index_map.find(nodeId);
  if (internalNodeIdPtr == nullptr) {
    return node_iterator{nodes.end()};
  } else {
    NodeData* p = nodeAtId(*internalNodeIdPtr);
    if (p->edges_begin == 1) {
      return node_iterator{nodes.end()};
    }
    return node_iterator{p};
  }
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::NodeData*
SemistaticGraph<NodeId, Node>::nodeAtId(InternalNodeId internalNodeId) {
  return nodeAtId(nodes.data(), internalNodeId);
}

template <typename NodeId, typename Node>
inline const typename SemistaticGraph<NodeId, Node>::NodeData*
SemistaticGraph<NodeId, Node>::nodeAtId(InternalNodeId internalNodeId) const {
  return nodeAtId(nodes.data(), internalNodeId);
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::NodeData*
SemistaticGraph<NodeId, Node>::nodeAtId(NodeData* nodes_begin, InternalNodeId internalNodeId) {
  FruitAssert(internalNodeId.id % sizeof(NodeData) == 0);
  NodeData* p = reinterpret_cast<NodeData*>(reinterpret_cast<char*>(nodes_begin) + internalNodeId.id);
  // The code above is faster (the compiler doesn't have to worry about internalNodeId.id%sizeof(NodeData), that we know
  // to be 0).
  FruitAssert(p == nodes_begin + internalNodeId.id / sizeof(NodeData));
  return p;
}

template <typename NodeId, typename Node>
inline const typename SemistaticGraph<NodeId, Node>::NodeData*
SemistaticGraph<NodeId, Node>::nodeAtId(const NodeData* nodes_begin, InternalNodeId internalNodeId) {
  FruitAssert(internalNodeId.id % sizeof(NodeData) == 0);
  const NodeData* p = reinterpret_cast<const NodeData*>(reinterpret_cast<const char*>(nodes_begin) + internalNodeId.id);
  // The code above is faster (the compiler doesn't have to worry about internalNodeId.id%sizeof(NodeData), that we know
  // to be 0).
  FruitAssert(p == nodes_begin + internalNodeId.id / sizeof(NodeData));
  return p;
}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_GRAPH_INLINES_H
