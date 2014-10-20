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

#ifndef STATIC_GRAPH_INLINES_H
#define STATIC_GRAPH_INLINES_H

#include "semistatic_graph.h"

#include <cassert>

namespace fruit {
namespace impl {

template <typename NodeId, typename Node>
inline StaticGraph<NodeId, Node>::node_iterator::node_iterator(NodeItr itr) 
  : itr(itr) {
}

template <typename NodeId, typename Node>
inline Node& StaticGraph<NodeId, Node>::node_iterator::getNode() {
  return reinterpret_cast<NodeData*>(itr)->node;
}

template <typename NodeId, typename Node>
inline bool StaticGraph<NodeId, Node>::node_iterator::isTerminal() {
  return reinterpret_cast<NodeData*>(itr)->edgesBeginOffset == 0;
}

template <typename NodeId, typename Node>
inline void StaticGraph<NodeId, Node>::node_iterator::setTerminal() {
   reinterpret_cast<NodeData*>(itr)->edgesBeginOffset = 0;
}

template <typename NodeId, typename Node>
inline bool StaticGraph<NodeId, Node>::node_iterator::operator==(const node_iterator& other) const {
  return itr == other.itr;
}

template <typename NodeId, typename Node>
inline typename StaticGraph<NodeId, Node>::edge_iterator StaticGraph<NodeId, Node>::node_iterator::neighborsBegin() {
  return edge_iterator{reinterpret_cast<NodeData*>(itr)->edgesBeginOffset};
}

template <typename NodeId, typename Node>
inline StaticGraph<NodeId, Node>::edge_iterator::edge_iterator(EdgeItr itr) 
  : itr(itr) {
}

template <typename NodeId, typename Node>
inline typename StaticGraph<NodeId, Node>::node_iterator StaticGraph<NodeId, Node>::edge_iterator::getNodeIterator() {
  return node_iterator{*reinterpret_cast<NodeItr*>(itr)};
}

template <typename NodeId, typename Node>
inline void StaticGraph<NodeId, Node>::edge_iterator::operator++() {
  itr = reinterpret_cast<std::uintptr_t>(reinterpret_cast<NodeItr*>(itr) + 1);
}

template <typename NodeId, typename Node>
inline typename StaticGraph<NodeId, Node>::node_iterator StaticGraph<NodeId, Node>::edge_iterator::getNodeIterator(
    std::size_t i) {
  itr = reinterpret_cast<std::uintptr_t>(reinterpret_cast<NodeItr*>(itr) + i);
  return getNodeIterator();
}

template <typename NodeId, typename Node>
inline typename StaticGraph<NodeId, Node>::node_iterator StaticGraph<NodeId, Node>::end() {
  return node_iterator{reinterpret_cast<std::uintptr_t>(nodes.data() + nodes.size())};
}

template <typename NodeId, typename Node>
inline typename StaticGraph<NodeId, Node>::node_iterator StaticGraph<NodeId, Node>::at(NodeId nodeId) {
  return node_iterator{nodesById.at(nodeId)};
}

template <typename NodeId, typename Node>
inline typename StaticGraph<NodeId, Node>::node_iterator StaticGraph<NodeId, Node>::find(NodeId nodeId) {
  NodeItr* nodeItrPtr = nodesById.find(nodeId);
  if (nodeItrPtr == nullptr) {
    return end();
  } else {
    return node_iterator{*nodeItrPtr};
  }
}

} // namespace impl
} // namespace fruit

#endif // STATIC_GRAPH_INLINES_H
