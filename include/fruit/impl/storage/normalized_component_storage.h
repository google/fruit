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

#include <fruit/impl/util/type_info.h>
#include <fruit/impl/binding_data.h>
#include <fruit/impl/data_structures/semistatic_map.h>
#include <fruit/impl/data_structures/semistatic_graph.h>
#include <fruit/impl/fruit_internal_forward_decls.h>
#include <fruit/impl/storage/injector_storage.h>

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
  using Graph = InjectorStorage::Graph;
  
private:
  // A graph with types as nodes (each node stores the BindingData for the type) and dependencies as edges.
  // For types that have a constructed object already, the corresponding node is stored as terminal node.
  SemistaticGraph<TypeId, NormalizedBindingData> bindings;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  std::unordered_map<TypeId, NormalizedMultibindingData> multibindings;
  
  // Contains data on the set of types that can be allocated using this component.
  FixedSizeAllocator::FixedSizeAllocatorData fixed_size_allocator_data;
  
  // Stores information on binding compression that was performed in bindings of this object.
  // See also the documentation for BindingCompressionInfoMap.
  InjectorStorage::BindingCompressionInfoMap bindingCompressionInfoMap;
  
  friend class InjectorStorage;
  
public:
  NormalizedComponentStorage() = delete;
  
  NormalizedComponentStorage(ComponentStorage&& component, const std::vector<TypeId>& exposed_types);

  NormalizedComponentStorage(NormalizedComponentStorage&&) = delete;
  NormalizedComponentStorage(const NormalizedComponentStorage&) = delete;
  
  NormalizedComponentStorage& operator=(NormalizedComponentStorage&&) = default;
  NormalizedComponentStorage& operator=(const NormalizedComponentStorage&) = default;
  
  ~NormalizedComponentStorage() = default;
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_H
