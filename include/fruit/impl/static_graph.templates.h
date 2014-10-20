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

#ifndef STATIC_GRAPH_TEMPLATES_H
#define STATIC_GRAPH_TEMPLATES_H

#include "semistatic_graph.h"

#include <unordered_set>
#include <cassert>

namespace fruit {
namespace impl {

template <typename NodeId, typename Node>
StaticGraph<NodeId, Node>::StaticGraph(SemistaticGraph<NodeId, Node>&& other)
  : nodes(std::move(other.nodes)), edgesStorage(std::move(other.edgesStorage)), nodesById(std::move(other.nodeIndexMap)) {
  
  nodesById.transformValues([&](std::uintptr_t i){ return reinterpret_cast<std::uintptr_t>(nodes.data() + i);});
  
  for (std::uintptr_t& edge : edgesStorage) {
    edge = reinterpret_cast<std::uintptr_t>(nodes.data() + edge);
  }
  
  for (NodeData& node : nodes) {
    assert(node.edgesBeginOffset != (SemistaticGraph<NodeId, Node>::invalidEdgesBeginOffset));
    if (node.edgesBeginOffset != 0) {
      node.edgesBeginOffset = reinterpret_cast<std::uintptr_t>(edgesStorage.data() + node.edgesBeginOffset);
    }
  }
}

} // namespace impl
} // namespace fruit

#endif // STATIC_GRAPH_TEMPLATES_H
