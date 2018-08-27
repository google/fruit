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

#define IN_FRUIT_CPP_FILE 1

#include <algorithm>
#include <cstdlib>
#include <fruit/impl/util/type_info.h>
#include <iostream>
#include <memory>
#include <vector>

#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>

#include <fruit/impl/component_storage/component_storage.h>
#include <fruit/impl/data_structures/semistatic_graph.templates.h>
#include <fruit/impl/data_structures/semistatic_map.templates.h>
#include <fruit/impl/injector/injector_storage.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.h>

using std::cout;
using std::endl;

using namespace fruit;
using namespace fruit::impl;

namespace fruit {
namespace impl {

NormalizedComponentStorage::NormalizedComponentStorage(ComponentStorage&& component,
                                                       const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
                                                       MemoryPool& memory_pool, WithPermanentCompression)
    : normalized_component_memory_pool(),
      binding_compression_info_map(createHashMapWithArenaAllocator<TypeId, CompressedBindingUndoInfo>(
          0 /* capacity */, normalized_component_memory_pool)),
      fully_expanded_components_with_no_args(
          createLazyComponentWithNoArgsSet(0 /* capacity */, normalized_component_memory_pool)),
      fully_expanded_components_with_args(
          createLazyComponentWithArgsSet(0 /* capacity */, normalized_component_memory_pool)),
      component_with_no_args_replacements(
          createLazyComponentWithNoArgsReplacementMap(0 /* capacity */, normalized_component_memory_pool)),
      component_with_args_replacements(
          createLazyComponentWithArgsReplacementMap(0 /* capacity */, normalized_component_memory_pool)) {

  using bindings_vector_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;
  bindings_vector_t bindings_vector = bindings_vector_t(ArenaAllocator<ComponentStorageEntry>(memory_pool));
  BindingNormalization::normalizeBindingsWithPermanentBindingCompression(std::move(component).release(),
                                                                         fixed_size_allocator_data, memory_pool,
                                                                         exposed_types, bindings_vector, multibindings);

  bindings = SemistaticGraph<TypeId, NormalizedBinding>(InjectorStorage::BindingDataNodeIter{bindings_vector.begin()},
                                                        InjectorStorage::BindingDataNodeIter{bindings_vector.end()},
                                                        memory_pool);
}

NormalizedComponentStorage::NormalizedComponentStorage(ComponentStorage&& component,
                                                       const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
                                                       MemoryPool& memory_pool, WithUndoableCompression)
    : normalized_component_memory_pool(),
      binding_compression_info_map(createHashMapWithArenaAllocator<TypeId, CompressedBindingUndoInfo>(
          20 /* capacity */, normalized_component_memory_pool)),
      fully_expanded_components_with_no_args(
          createLazyComponentWithNoArgsSet(20 /* capacity */, normalized_component_memory_pool)),
      fully_expanded_components_with_args(
          createLazyComponentWithArgsSet(20 /* capacity */, normalized_component_memory_pool)),
      component_with_no_args_replacements(
          createLazyComponentWithNoArgsReplacementMap(20 /* capacity */, normalized_component_memory_pool)),
      component_with_args_replacements(
          createLazyComponentWithArgsReplacementMap(20 /* capacity */, normalized_component_memory_pool)) {

  using bindings_vector_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;
  bindings_vector_t bindings_vector = bindings_vector_t(ArenaAllocator<ComponentStorageEntry>(memory_pool));
  BindingNormalization::normalizeBindingsWithUndoableBindingCompression(
      std::move(component).release(), fixed_size_allocator_data, memory_pool, normalized_component_memory_pool,
      normalized_component_memory_pool, exposed_types, bindings_vector, multibindings, binding_compression_info_map,
      fully_expanded_components_with_no_args, fully_expanded_components_with_args, component_with_no_args_replacements,
      component_with_args_replacements);

  bindings = SemistaticGraph<TypeId, NormalizedBinding>(InjectorStorage::BindingDataNodeIter{bindings_vector.begin()},
                                                        InjectorStorage::BindingDataNodeIter{bindings_vector.end()},
                                                        memory_pool);
}

NormalizedComponentStorage::~NormalizedComponentStorage() {
  for (auto& x : fully_expanded_components_with_args) {
    x.destroy();
  }

  for (const auto& pair : component_with_args_replacements) {
    const LazyComponentWithArgs& replaced_component = pair.first;
    const ComponentStorageEntry& replacement_component = pair.second;
    replaced_component.destroy();
    replacement_component.destroy();
  }

  for (const auto& pair : component_with_no_args_replacements) {
    const ComponentStorageEntry& replacement_component = pair.second;
    replacement_component.destroy();
  }

  // We must free all the memory in these before the normalized_component_memory_pool is destroyed.
  binding_compression_info_map = createHashMapWithArenaAllocator<TypeId, CompressedBindingUndoInfo>(
      0 /* capacity */, normalized_component_memory_pool);
  fully_expanded_components_with_no_args =
      createLazyComponentWithNoArgsSet(0 /* capacity */, normalized_component_memory_pool);
  fully_expanded_components_with_args =
      createLazyComponentWithArgsSet(0 /* capacity */, normalized_component_memory_pool);
  component_with_no_args_replacements =
      createLazyComponentWithNoArgsReplacementMap(0 /* capacity */, normalized_component_memory_pool);
  component_with_args_replacements =
      createLazyComponentWithArgsReplacementMap(0 /* capacity */, normalized_component_memory_pool);
}

} // namespace impl
// We need a LCOV_EXCL_BR_LINE below because for some reason gcov/lcov think there's a branch there.
} // namespace fruit LCOV_EXCL_BR_LINE
