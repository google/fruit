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

#ifndef SEMISTATIC_GRAPH_TEMPLATES_H
#define SEMISTATIC_GRAPH_TEMPLATES_H

#include "semistatic_graph.h"

#include <unordered_set>

namespace fruit {
namespace impl {

template <typename Iter>
struct indexing_iterator {
  Iter iter;
  std::uintptr_t index;
  
  void operator++() {
    ++iter;
    ++index;
  }
  
  auto operator*() -> decltype(std::make_pair(*iter, index)) {
    return std::make_pair(*iter, index);
  }
};

template <typename NodeId, typename Node>
template <typename NodeIter>
SemistaticGraph<NodeId, Node>::SemistaticGraph(NodeIter first, NodeIter last) {
  
  std::uintptr_t num_edges = 0;
  std::uintptr_t firstUnusedIndex;
  
  // Step 1: assign IDs to all nodes, fill `nodeIndexMap' and set `firstUnusedIndex'.
  std::unordered_set<NodeId> nodeIds;
  for (NodeIter i = first; i != last; ++i) {
    nodeIds.insert(i->getId());
    if (!i->isTerminal()) {
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        nodeIds.insert(*j);
        ++num_edges;
      }
    }
  }
  
  nodeIndexMap = SemistaticMap<NodeId, std::uintptr_t>(
      indexing_iterator<typename std::unordered_set<NodeId>::iterator>{nodeIds.begin(), 0},
      nodeIds.size());
  
  firstUnusedIndex = nodeIds.size();
  
  // Step 2: fill `nodes' and `edgesStorage'
  
  // Note that not all of these will be assigned in the loop below.
  nodes.resize(firstUnusedIndex, NodeData{
#ifndef NDEBUG
    getTypeId<float*>(),
#endif
    Node(), ~std::uintptr_t(0)});
  
#ifndef NDEBUG
  {
#ifdef FRUIT_EXTRA_DEBUG
    std::cerr << "SemistaticGraph constructed with the following known types:" << std::endl;
#endif
    std::uintptr_t i = 0;
    for (typename std::unordered_set<NodeId>::iterator itr = nodeIds.begin(); itr != nodeIds.end(); ++i, ++itr) {
      nodes[i].key = *itr;
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << i << ": " << (*itr)->name() << std::endl;
#endif
    }
  }
#endif
  
  // edgesStorage[0] is unused, that's the reason for the +1
  edgesStorage.reserve(num_edges + 1);
  edgesStorage.resize(1);
  
#ifdef FRUIT_EXTRA_DEBUG
    std::cerr << "Nodes:" << std::endl;
#endif
  for (NodeIter i = first; i != last; ++i) {
    std::uintptr_t nodeId = nodeIndexMap.at(i->getId());
#ifdef FRUIT_EXTRA_DEBUG
    std::cerr << nodeId << ": " << i->getId()->name() << " depends on";
#endif
    nodes[nodeId].node = i->getValue();
    if (i->isTerminal()) {
      nodes[nodeId].edgesBeginOffset = 0;
#ifdef FRUIT_EXTRA_DEBUG
        std::cerr << " (none, terminal)";
#endif
    } else {
      nodes[nodeId].edgesBeginOffset = edgesStorage.size();
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
#ifdef FRUIT_EXTRA_DEBUG
        std::cerr << " " << (*j)->name();
#endif
        std::uintptr_t otherNodeId = nodeIndexMap.at(*j);
        edgesStorage.push_back(otherNodeId);
      }
    }
#ifdef FRUIT_EXTRA_DEBUG
    std::cerr << std::endl;
    std::cerr << "nodes[" << nodeId << "].edgesBeginOffset == " << nodes[nodeId].edgesBeginOffset << std::endl;
#endif
  }  
}

// TODO: This requires NodeId==const TypeInfo*, it breaks the abstraction. Needs refactoring.
#ifndef NDEBUG

#include <iostream>

template <typename NodeId, typename Node>
void SemistaticGraph<NodeId, Node>::checkFullyConstructed() {
  for (std::uintptr_t i = 0; i < nodes.size(); ++i) {
    NodeData& data = nodes[i];
    if (data.edgesBeginOffset == invalidEdgesBeginOffset) {
      std::cerr << "Fruit bug: the node for the following type was not fully constructed in the dependency graph: " << data.key->name() << std::endl;
      abort();
    }
  }
}
#endif // !NDEBUG

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_GRAPH_TEMPLATES_H
