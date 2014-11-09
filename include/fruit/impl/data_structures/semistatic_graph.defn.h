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
  assert(itr->edgesBegin != 1);
  return itr->node;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::node_iterator::isTerminal() {
  assert(itr->edgesBegin != 1);
  return itr->edgesBegin == 0;
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::node_iterator::setTerminal() {
  assert(itr->edgesBegin != 1);
  itr->edgesBegin = 0;
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
  assert(itr->edgesBegin != 1);
  return itr->node;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::const_node_iterator::isTerminal() {
  assert(itr->edgesBegin != 1);
  return itr->edgesBeginOffset == 0;
}

template <typename NodeId, typename Node>
inline void SemistaticGraph<NodeId, Node>::const_node_iterator::setTerminal() {
  assert(itr->edgesBegin != 1);
  itr->edgesBeginOffset = 0;
}

template <typename NodeId, typename Node>
inline bool SemistaticGraph<NodeId, Node>::const_node_iterator::operator==(const const_node_iterator& other) const {
  return itr == other.itr;
}


template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::edge_iterator SemistaticGraph<NodeId, Node>::node_iterator::neighborsBegin() {
  assert(itr->edgesBegin != 0);
  assert(itr->edgesBegin != 1);
  return edge_iterator{reinterpret_cast<InternalNodeId*>(itr->edgesBegin)};
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
  return node_iterator{nodes.begin() + nodeIndexMap.at(nodeId).id};
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::const_node_iterator SemistaticGraph<NodeId, Node>::find(NodeId nodeId) const {
  const InternalNodeId* internalNodeIdPtr = nodeIndexMap.find(nodeId);
  if (internalNodeIdPtr == nullptr) {
    return const_node_iterator{nodes.end()};
  } else {
    auto itr = nodes.begin() + internalNodeIdPtr->id;
    if (itr->edgesBegin == 1) {
      return const_node_iterator{nodes.end()};
    }
    return const_node_iterator{itr};
  }
}

template <typename NodeId, typename Node>
inline typename SemistaticGraph<NodeId, Node>::node_iterator SemistaticGraph<NodeId, Node>::find(NodeId nodeId) {
  const InternalNodeId* internalNodeIdPtr = nodeIndexMap.find(nodeId);
  if (internalNodeIdPtr == nullptr) {
    return node_iterator{nodes.end()};
  } else {
    auto itr = nodes.begin() + internalNodeIdPtr->id;
    if (itr->edgesBegin == 1) {
      return node_iterator{nodes.end()};
    }
    return node_iterator{itr};
  }
}

template <typename NodeId, typename Node>
inline SemistaticGraphInternalNodeId SemistaticGraph<NodeId, Node>::getOrAllocateInternalNodeId(NodeId nodeId) {
  // TODO: Optimize this, do a single lookup.
  InternalNodeId* internalNodeIdPtr = nodeIndexMap.find(nodeId);
  if (internalNodeIdPtr != nullptr) {
    return *internalNodeIdPtr;
  } else {
    // Allocate a new node ID.
    std::size_t nodeIndex = nodes.size();
    nodeIndexMap.insert(nodeId, InternalNodeId{nodeIndex}, [](InternalNodeId x, InternalNodeId){ return x;});
    nodes.push_back(NodeData{
#ifdef FRUIT_EXTRA_DEBUG
    nodeId,
#endif
      Node(), 1});
    return InternalNodeId{nodeIndex};
  }
}

template <typename NodeId, typename Node>
void SemistaticGraph<NodeId, Node>::changeNodeToTerminal(NodeId nodeId) {
  assert(nodeIndexMap.find(nodeId) != nullptr);
  InternalNodeId internalNodeId = nodeIndexMap.at(nodeId);
  NodeData& nodeData = nodes[internalNodeId.id];
  assert(nodeData.edgesBegin != 1);
  nodeData.edgesBegin = 0;
}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_GRAPH_INLINES_H
