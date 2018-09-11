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

#if !IN_FRUIT_CPP_FILE
#error "Fruit .template.h file included in non-cpp file."
#endif

#include <fruit/impl/data_structures/arena_allocator.h>
#include <fruit/impl/data_structures/fixed_size_vector.templates.h>
#include <fruit/impl/data_structures/memory_pool.h>
#include <fruit/impl/data_structures/semistatic_graph.h>
#include <fruit/impl/data_structures/semistatic_map.templates.h>
#include <fruit/impl/util/hash_helpers.h>

#if FRUIT_EXTRA_DEBUG
#include <iostream>
#endif

namespace fruit {
namespace impl {

template <typename Iter, std::size_t index_increment>
struct indexing_iterator {
  Iter iter;
  std::size_t index;

  void operator++() {
    ++iter;
    index += index_increment;
  }

  auto operator*() -> decltype(std::make_pair(*iter, SemistaticGraphInternalNodeId{index})) {
    return std::make_pair(*iter, SemistaticGraphInternalNodeId{index});
  }

  bool operator==(const indexing_iterator<Iter, index_increment>& other) const {
    return iter == other.iter;
  }
};

#if FRUIT_EXTRA_DEBUG
template <typename NodeId, typename Node>
template <typename NodeIter>
void SemistaticGraph<NodeId, Node>::printGraph(NodeIter first, NodeIter last) {
  std::cerr << "Constructed injection graph with nodes:" << std::endl;
  for (NodeIter i = first; i != last; ++i) {
    std::cerr << i->getId() << " -> ";
    if (i->isTerminal()) {
      std::cerr << "(none, terminal)";
    } else {
      std::cerr << "{";
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        if (j != i->getEdgesBegin()) {
          std::cerr << ", ";
        }
        std::cerr << *j;
      }
      std::cerr << "}";
    }
    std::cerr << std::endl;
  }
  std::cerr << std::endl;
}
#endif // FRUIT_EXTRA_DEBUG

template <typename NodeId, typename Node>
template <typename NodeIter>
SemistaticGraph<NodeId, Node>::SemistaticGraph(NodeIter first, NodeIter last, MemoryPool& memory_pool) {
  std::size_t num_edges = 0;
  // Step 1: assign IDs to all nodes, fill node_index_map and set first_unused_index.
  HashSetWithArenaAllocator<NodeId> node_ids = createHashSetWithArenaAllocator<NodeId>(last - first, memory_pool);
  for (NodeIter i = first; i != last; ++i) {
    node_ids.insert(i->getId());
    if (!i->isTerminal()) {
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        node_ids.insert(*j);
        ++num_edges;
      }
    }
  }

  using itr_t = typename HashSetWithArenaAllocator<NodeId>::iterator;
  node_index_map = SemistaticMap<NodeId, InternalNodeId>(
      indexing_iterator<itr_t, sizeof(NodeData)>{node_ids.begin(), 0},
      indexing_iterator<itr_t, sizeof(NodeData)>{node_ids.end(), node_ids.size() * sizeof(NodeData)},
      node_ids.size(),
      memory_pool);

  first_unused_index = node_ids.size();

  // Step 2: fill `nodes' and edges_storage.

  // Note that not all of these will be assigned in the loop below.
  nodes = FixedSizeVector<NodeData>(first_unused_index, NodeData{
#if FRUIT_EXTRA_DEBUG
                                                            NodeId(),
#endif
                                                            1, Node()});

  // edges_storage[0] is unused, that's the reason for the +1
  edges_storage = FixedSizeVector<InternalNodeId>(num_edges + 1);
  edges_storage.push_back(InternalNodeId());

  for (NodeIter i = first; i != last; ++i) {
    NodeData& nodeData = *nodeAtId(node_index_map.at(i->getId()));
    nodeData.node = i->getValue();
    if (i->isTerminal()) {
      nodeData.edges_begin = 0;
    } else {
      nodeData.edges_begin = reinterpret_cast<std::uintptr_t>(edges_storage.data() + edges_storage.size());
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        InternalNodeId other_node_id = node_index_map.at(*j);
        edges_storage.push_back(other_node_id);
      }
    }
  }

#if FRUIT_EXTRA_DEBUG
  printGraph(first, last);
#endif
}

template <typename NodeId, typename Node>
template <typename NodeIter>
SemistaticGraph<NodeId, Node>::SemistaticGraph(const SemistaticGraph& x, NodeIter first, NodeIter last,
                                               MemoryPool& memory_pool)
    : first_unused_index(x.first_unused_index) {

  // TODO: The code below is very similar to the other constructor, extract the common parts in separate functions.

  std::size_t num_new_edges = 0;

  // Step 1: assign IDs to new nodes, fill `node_index_map' and update `first_unused_index'.

  // Step 1a: collect all new node IDs.
  using node_ids_elem_t = std::pair<NodeId, InternalNodeId>;
  using node_ids_t = std::vector<node_ids_elem_t, ArenaAllocator<node_ids_elem_t>>;
  node_ids_t node_ids = node_ids_t(ArenaAllocator<node_ids_elem_t>(memory_pool));
  for (NodeIter i = first; i != last; ++i) {
    if (x.node_index_map.find(i->getId()) == nullptr) {
      node_ids.push_back(std::make_pair(i->getId(), InternalNodeId()));
    }
    if (!i->isTerminal()) {
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        if (x.node_index_map.find(*j) == nullptr) {
          node_ids.push_back(std::make_pair(*j, InternalNodeId()));
        }
        ++num_new_edges;
      }
    }
  }

  // Step 1b: remove duplicates.
  std::sort(node_ids.begin(), node_ids.end());
  node_ids.erase(std::unique(node_ids.begin(), node_ids.end()), node_ids.end());

  // Step 1c: assign new IDs.
  for (auto& p : node_ids) {
    p.second = InternalNodeId{first_unused_index * sizeof(NodeData)};
    ++first_unused_index;
  }

  // Step 1d: actually populate node_index_map.
  node_index_map = SemistaticMap<NodeId, InternalNodeId>(x.node_index_map, std::move(node_ids));

  // Step 2: fill `nodes' and `edges_storage'
  nodes = FixedSizeVector<NodeData>(x.nodes, first_unused_index);
  // Note that the loop below does not necessarily assign all of these.
  for (std::size_t i = x.nodes.size(); i < first_unused_index; ++i) {
    nodes.push_back(NodeData{
#if FRUIT_EXTRA_DEBUG
        NodeId(),
#endif
        1, Node()});
  }

  // edges_storage[0] is unused, that's the reason for the +1
  edges_storage = FixedSizeVector<InternalNodeId>(num_new_edges + 1);
  edges_storage.push_back(InternalNodeId());

  for (NodeIter i = first; i != last; ++i) {
    NodeData& nodeData = *nodeAtId(node_index_map.at(i->getId()));
    nodeData.node = i->getValue();
    if (i->isTerminal()) {
      nodeData.edges_begin = 0;
    } else {
      nodeData.edges_begin = reinterpret_cast<std::uintptr_t>(edges_storage.data() + edges_storage.size());
      for (auto j = i->getEdgesBegin(); j != i->getEdgesEnd(); ++j) {
        InternalNodeId otherNodeId = node_index_map.at(*j);
        edges_storage.push_back(otherNodeId);
      }
    }
  }

#if FRUIT_EXTRA_DEBUG
  printGraph(first, last);
#endif
}

#if FRUIT_EXTRA_DEBUG
template <typename NodeId, typename Node>
void SemistaticGraph<NodeId, Node>::checkFullyConstructed() {
  for (NodeData& data : nodes) {
    if (data.edges_begin == 1) {
      std::cerr << "Fruit bug: the dependency graph was not fully constructed." << std::endl;
      abort();
    }
  }
}
#endif // !FRUIT_EXTRA_DEBUG

// This is here so that we don't have to include fixed_size_vector.templates.h in fruit.h.
template <typename NodeId, typename Node>
SemistaticGraph<NodeId, Node>::~SemistaticGraph() {}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_GRAPH_TEMPLATES_H
