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
  assert(itr->edgesBeginOffset != invalidEdgesBeginOffset);
  return itr->node;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::node_iterator::isTerminal() {
  assert(itr->edgesBeginOffset != invalidEdgesBeginOffset);
  return itr->edgesBeginOffset == 0;
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::node_iterator::setTerminal() {
  assert(itr->edgesBeginOffset != invalidEdgesBeginOffset);
  itr->edgesBeginOffset = 0;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::node_iterator::operator==(const node_iterator& other) const {
  return itr == other.itr;
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::edge_iterator SemistaticGraph<NodeId, Node>::node_iterator::neighborsBegin(
    SemistaticGraph<NodeId, Node>& graph) {
  return edge_iterator{graph.edgesStorage.begin() + itr->edgesBeginOffset};
}

template <typename NodeId, typename Node>
inline SemistaticGraph<NodeId, Node>::edge_iterator::edge_iterator(typename std::vector<std::size_t>::iterator itr) 
  : itr(itr) {
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::edge_iterator::getNodeIterator(
    SemistaticGraph<NodeId, Node>& graph) {
  return node_iterator{graph.nodes.begin() + *itr};
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
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::at(NodeId nodeId) {
  return node_iterator{nodes.begin() + nodeIndexMap.at(nodeId)};
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::find(NodeId nodeId) {
  std::size_t* nodeIndexPtr = nodeIndexMap.find(nodeId);
  if (nodeIndexPtr == nullptr) {
    return node_iterator{nodes.end()};
  } else {
    return node_iterator{nodes.begin() + *nodeIndexPtr};
  }
}

template <typename NodeId, typename Node>
inline std::size_t SemistaticGraph<NodeId, Node>::getOrAllocateNodeIndex(NodeId nodeId) {
  std::size_t* nodeIndexPtr = nodeIndexMap.find(nodeId);
  if (nodeIndexPtr != nullptr) {
    return *nodeIndexPtr;
  } else {
    // Allocate a new node ID.
    std::size_t nodeIndex = nodes.size();
    nodeIndexMap.insert(nodeId, nodeIndex, [](std::size_t x, std::size_t){ return x;});
    nodes.push_back(NodeData{
#ifndef NDEBUG
    nodeId,
#endif
      Node(), ~std::size_t(0)});
    return nodeIndex;
  }
}

template <typename NodeId, typename Node>
template <typename EdgeIter, typename Combine>
void SemistaticGraph<NodeId, Node>::setNode(NodeId nodeId, Node node, EdgeIter edgesBegin, EdgeIter edgesEnd, Combine combine) {
  std::size_t nodeIndex = getOrAllocateNodeIndex(nodeId);
  NodeData& nodeData = nodes[nodeIndex];
#ifndef NDEBUG
  nodeData.key = nodeId;
#endif
  if (nodeData.edgesBeginOffset == ~std::size_t(0)) {
    // The node did not exist.
    nodeData.node = node;
  } else {
    nodeData.node = combine(nodeData.node, node);
  }
  nodeData.edgesBeginOffset = edgesStorage.size();
  for (EdgeIter i = edgesBegin; i != edgesEnd; ++i) {
    edgesStorage.push_back(getOrAllocateNodeIndex(*i));
  }
}

template <typename NodeId, typename Node>
template <typename Combine>
void SemistaticGraph<NodeId, Node>::setTerminalNode(NodeId nodeId, Node node, Combine combine) {
  std::size_t nodeIndex = getOrAllocateNodeIndex(nodeId);
  NodeData& nodeData = nodes[nodeIndex];
#ifndef NDEBUG
  nodeData.key = nodeId;
#endif
  if (nodeData.edgesBeginOffset == ~std::size_t(0)) {
    // The node did not exist.
    nodeData.node = node;
  } else {
    nodeData.node = combine(nodeData.node, node);
  }
  nodeData.edgesBeginOffset = 0;
}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_GRAPH_INLINES_H
