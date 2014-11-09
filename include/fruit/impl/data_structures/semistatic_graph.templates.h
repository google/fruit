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

#ifndef IN_FRUIT_CPP_FILE
#error "Fruit .template.h file included in non-cpp file."
#endif

#include "semistatic_graph.h"
#include "semistatic_map.templates.h"

#include <unordered_set>

#ifdef FRUIT_EXTRA_DEBUG
#include <iostream>
#endif

namespace fruit {
namespace impl {

template <typename Iter>
struct indexing_iterator {
  Iter iter;
  std::size_t index;
  
  void operator++() {
    ++iter;
    ++index;
  }
  
  auto operator*() -> decltype(std::make_pair(*iter, SemistaticGraphInternalNodeId{index})) {
    return std::make_pair(*iter, SemistaticGraphInternalNodeId{index});
  }
};

#ifdef FRUIT_EXTRA_DEBUG
template <typename NodeId, typename Node>
void SemistaticGraph<NodeId, Node>::printEdgesBegin(std::ostream& os, std::uintptr_t edgesBegin) {
  if (edgesBegin == 0) {
    os << "[no edges, terminal node]";
  } else if (edgesBegin == 1) {
    os << "[node not fully constructed]";
  } else {
    SemistaticGraphInternalNodeId* p = reinterpret_cast<SemistaticGraphInternalNodeId*>(edgesBegin);
    if (edgesStorage.data() <= p && p < edgesStorage.data() + edgesStorage.size()) {
      os << "edgesStorage.data() + " << (p - edgesStorage.data());
    } else {
      os << p;
    }
  }
}
#endif

template <typename NodeId, typename Node>
template <typename NodeIter>
SemistaticGraph<NodeId, Node>::SemistaticGraph(NodeIter first, NodeIter last) {
  std::size_t num_edges = 0;
  
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
  
  nodeIndexMap = SemistaticMap<NodeId, InternalNodeId>(
      indexing_iterator<typename std::unordered_set<NodeId>::iterator>{nodeIds.begin(), 0},
      nodeIds.size());
  
  firstUnusedIndex = nodeIds.size();
  
  // Step 2: fill `nodes' and `edgesStorage'
  
  // Note that not all of these will be assigned in the loop below.
  nodes.resize(firstUnusedIndex, NodeData{
#ifdef FRUIT_EXTRA_DEBUG
    NodeId(),
#endif
    Node(), 1});
  
#ifdef FRUIT_EXTRA_DEBUG
  {
    std::cerr << "SemistaticGraph constructed with the following known types:" << std::endl;
    std::size_t i = 0;
    for (typename std::unordered_set<NodeId>::iterator itr = nodeIds.begin(); itr != nodeIds.end(); ++i, ++itr) {
      nodes[i].key = *itr;
      std::cerr << i << ": " << *itr << std::endl;
    }
  }
#endif
  
  // edgesStorage[0] is unused, that's the reason for the +1
  edgesStorage.reserve(num_edges + 1);
  edgesStorage.resize(1);
  
  for (NodeIter i = first; i != last; ++i) {
    std::size_t nodeId = nodeIndexMap.at(i->getId()).id;
    nodes[nodeId].node = i->getValue();
    if (i->isTerminal()) {
      nodes[nodeId].edgesBegin = 0;
    } else {
      nodes[nodeId].edgesBegin = reinterpret_cast<std::uintptr_t>(edgesStorage.data() + edgesStorage.size());
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        std::size_t otherNodeId = nodeIndexMap.at(*j).id;
        edgesStorage.push_back(InternalNodeId{otherNodeId});
      }
    }
  }  
  
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "Nodes:" << std::endl;
  for (NodeIter i = first; i != last; ++i) {
    std::size_t nodeId = nodeIndexMap.at(i->getId()).id;
    std::cerr << nodeId << ": " << i->getId() << " depends on";
    if (i->isTerminal()) {
      std::cerr << " (none, terminal)";
    } else {
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        std::cerr << " " << *j;
      }
    }
    std::cerr << std::endl;
    std::cerr << "nodes[" << nodeId << "].edgesBegin == "; // << nodes[nodeId].edgesBegin << std::endl;
    printEdgesBegin(std::cerr, nodes[nodeId].edgesBegin);
    std::cerr << std::endl;
  }  
#endif
}

template <typename NodeId, typename Node>
template <typename NodeIter>
SemistaticGraph<NodeId, Node>::SemistaticGraph(const SemistaticGraph& x, NodeIter first, NodeIter last)
  // TODO: Do a shallow copy of the index map too.
  : nodeIndexMap(x.nodeIndexMap), firstUnusedIndex(x.firstUnusedIndex), nodes(x.nodes) {
  
  // TODO: The code below is very similar to the other constructor, extract the common parts in separate functions.
  
  std::size_t num_new_edges = 0;
  
  // Step 1: assign IDs to new nodes, fill `nodeIndexMap' and update `firstUnusedIndex'.
  std::unordered_set<NodeId> nodeIds;
  for (NodeIter i = first; i != last; ++i) {
    ++firstUnusedIndex;
    nodeIndexMap.insert(i->getId(), InternalNodeId{firstUnusedIndex - 1}, [this](InternalNodeId x, InternalNodeId) {
      // There was already an index for this TypeId, we don't need to allocate an index after all.
      --firstUnusedIndex;
      return x;
    });
    if (!i->isTerminal()) {
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        ++firstUnusedIndex;
        nodeIndexMap.insert(*j, InternalNodeId{firstUnusedIndex - 1}, [this](InternalNodeId x, InternalNodeId) {
          // There was already an index for this TypeId, we don't need to allocate an index after all.
          --firstUnusedIndex;
          return x;
        });
        ++num_new_edges;
      }
    }
  }
  
  // Step 2: fill `nodes' and `edgesStorage'
  
  // Note that not all of these will be assigned in the loop below.
  nodes.resize(firstUnusedIndex, NodeData{
#ifdef FRUIT_EXTRA_DEBUG
    NodeId(),
#endif
    Node(), 1});
  
#ifdef FRUIT_EXTRA_DEBUG
  {
    std::cerr << "SemistaticGraph constructed with the following known types:" << std::endl;
    std::size_t i = 0;
    for (typename std::unordered_set<NodeId>::iterator itr = nodeIds.begin(); itr != nodeIds.end(); ++i, ++itr) {
      nodes[i].key = *itr;
      std::cerr << i << ": " << *itr << std::endl;
    }
  }
#endif
  
  // edgesStorage[0] is unused, that's the reason for the +1
  edgesStorage.reserve(num_new_edges + 1);
  edgesStorage.resize(1);
  
  for (NodeIter i = first; i != last; ++i) {
    std::size_t nodeId = nodeIndexMap.at(i->getId()).id;
    nodes[nodeId].node = i->getValue();
    if (i->isTerminal()) {
      nodes[nodeId].edgesBegin = 0;
    } else {
      nodes[nodeId].edgesBegin = reinterpret_cast<std::uintptr_t>(edgesStorage.data() + edgesStorage.size());
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        std::size_t otherNodeId = nodeIndexMap.at(*j).id;
        edgesStorage.push_back(InternalNodeId{otherNodeId});
      }
    }
  }  
  
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "Nodes:" << std::endl;
  for (NodeIter i = first; i != last; ++i) {
    std::size_t nodeId = nodeIndexMap.at(i->getId()).id;
    std::cerr << nodeId << ": " << i->getId() << " depends on";
    if (i->isTerminal()) {
      std::cerr << " (none, terminal)";
    } else {
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        std::cerr << " " << *j;
      }
    }
    std::cerr << std::endl;
    std::cerr << "nodes[" << nodeId << "].edgesBegin == "; //<< nodes[nodeId].edgesBegin << std::endl;
    printEdgesBegin(std::cerr, nodes[nodeId].edgesBegin);
    std::cerr << std::endl;
  }  
#endif
}

#ifdef FRUIT_EXTRA_DEBUG
template <typename NodeId, typename Node>
void SemistaticGraph<NodeId, Node>::checkFullyConstructed() {
  for (std::size_t i = 0; i < nodes.size(); ++i) {
    NodeData& data = nodes[i];
    if (data.edgesBegin == 1) {
      std::cerr << "Fruit bug: the dependency graph was not fully constructed." << std::endl;
      abort();
    }
  }
}
#endif // !NDEBUG

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_GRAPH_TEMPLATES_H
