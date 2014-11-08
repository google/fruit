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

#ifndef FRUIT_NORMALIZED_COMPONENT_STORAGE_H
#define FRUIT_NORMALIZED_COMPONENT_STORAGE_H

#include "../util/type_info.h"
#include "../binding_data.h"
#include "../data_structures/semistatic_map.h"
#include "../data_structures/semistatic_graph.h"
#include "../../fruit_forward_decls.h"

#include <memory>
#include <unordered_map>

namespace fruit {
  
namespace impl {

/**
 * Similar to ComponentStorage, but used a normalized representation to minimize the amount
 * of work needed to turn this into an injector. However, adding bindings to a normalized
 * component is slower than adding them to a simple component.
 */
class NormalizedComponentStorage {
public:  
  using BindingVectors = std::pair<std::vector<std::pair<TypeId, BindingData>>,
                                   std::vector<std::pair<TypeId, MultibindingData>>>;
  using Graph = SemistaticGraph<TypeId, NormalizedBindingData>;
  
private:
  // A graph with types as nodes (each node stores the BindingData for the type) and dependencies as edges.
  // For types that have a constructed object already, the corresponding node is stored as terminal node.
  SemistaticGraph<TypeId, NormalizedBindingData> typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  std::unordered_map<TypeId, NormalizedMultibindingData> typeRegistryForMultibindings;
  
  // The sum of (typeInfo->alignment() + typeInfo->size() - 1) for every binding and multibinding.
  // A new[total_size] allocates enough memory to construct all types registered in this component.
  size_t total_size = 0;
  
  friend class InjectorStorage;
  
public:
  static NormalizedComponentStorage mergeComponentStorages(const NormalizedComponentStorage& normalizedStorage, 
                                                           ComponentStorage&& storage);
  
  NormalizedComponentStorage() = default;
  
  NormalizedComponentStorage(NormalizedComponentStorage&&) = default;
  NormalizedComponentStorage(const NormalizedComponentStorage&) = default;
  
  NormalizedComponentStorage& operator=(NormalizedComponentStorage&&) = default;
  NormalizedComponentStorage& operator=(const NormalizedComponentStorage&) = default;
  
  NormalizedComponentStorage(BindingVectors&& bindingVectors);
  
  ~NormalizedComponentStorage();
};

} // namespace impl
} // namespace fruit

#include "normalized_component_storage.defn.h"

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_H
