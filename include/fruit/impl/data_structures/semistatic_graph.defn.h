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

#include "semistatic_graph.h"

#include <cassert>

namespace fruit {
namespace impl {

template <typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::node_iterator::node_iterator(typename std::vector<NodeData>::iterator itr) 
  : itr(itr) {
}

template <typename NodeId, typename Node>
inline Node& SemistaticGraph<NodeId, Node>::node_iterator::getNode() {
  assert(itr->edges_begin != 1);
  return itr->node;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::node_iterator::isTerminal() {
  assert(itr->edges_begin != 1);
  return itr->edges_begin == 0;
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::node_iterator::setTerminal() {
  assert(itr->edges_begin != 1);
  itr->edges_begin = 0;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::node_iterator::operator==(const node_iterator& other) const {
  return itr == other.itr;
}

template <typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::const_node_iterator::const_node_iterator(
  typename std::vector<NodeData>::const_iterator itr) 
  : itr(itr) {
}

template <typename NodeId, typename Node>
inline const Node& SemistaticGraph<NodeId, Node>::const_node_iterator::getNode() {
  assert(itr->edges_begin != 1);
  return itr->node;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::const_node_iterator::isTerminal() {
  assert(itr->edges_begin != 1);
  return itr->edges_begin == 0;
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::const_node_iterator::setTerminal() {
  assert(itr->edges_begin != 1);
  itr->edges_beginOffset = 0;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::const_node_iterator::operator==(const const_node_iterator& other) const {
  return itr == other.itr;
}


template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::edge_iterator SemistaticGraph<NodeId, Node>::node_iterator::neighborsBegin() {
  assert(itr->edges_begin != 0);
  assert(itr->edges_begin != 1);
  return edge_iterator{reinterpret_cast<InternalNodeId*>(itr->edges_begin)};
}

template <typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::edge_iterator::edge_iterator(InternalNodeId* itr) 
  : itr(itr) {
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::edge_iterator::getNodeIterator(
    SemistaticGraph<NodeId, Node>& graph) {
  return node_iterator{graph.nodes.begin() + itr->id};
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::edge_iterator::operator++() {
  ++itr;
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::edge_iterator::getNodeIterator(
    std::size_t i, SemistaticGraph<NodeId, Node>& graph) {
  itr += i;
  return getNodeIterator(graph);
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
  return node_iterator{nodes.begin() + node_index_map.at(nodeId).id};
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::const_node_iterator SemistaticGraph<NodeId, Node>::find(NodeId nodeId) const {
  const InternalNodeId* internalNodeIdPtr = node_index_map.find(nodeId);
  if (internalNodeIdPtr == nullptr) {
    return const_node_iterator{nodes.end()};
  } else {
    auto itr = nodes.begin() + internalNodeIdPtr->id;
    if (itr->edges_begin == 1) {
      return const_node_iterator{nodes.end()};
    }
    return const_node_iterator{itr};
  }
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::find(NodeId nodeId) {
  const InternalNodeId* internalNodeIdPtr = node_index_map.find(nodeId);
  if (internalNodeIdPtr == nullptr) {
    return node_iterator{nodes.end()};
  } else {
    auto itr = nodes.begin() + internalNodeIdPtr->id;
    if (itr->edges_begin == 1) {
      return node_iterator{nodes.end()};
    }
    return node_iterator{itr};
  }
}

template <typename NodeId, typename Node>
inline std::size_t SemistaticGraph<NodeId, Node>::size() const {
  return nodes.size();
}

template <typename NodeId, typename Node>
void SemistaticGraph<NodeId, Node>::changeNodeToTerminal(NodeId nodeId) {
  assert(node_index_map.find(nodeId) != nullptr);
  InternalNodeId internal_node_id = node_index_map.at(nodeId);
  NodeData& node_data = nodes[internal_node_id.id];
  assert(node_data.edges_begin != 1);
  node_data.edges_begin = 0;
}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_GRAPH_INLINES_H
