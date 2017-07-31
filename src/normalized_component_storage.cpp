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

#define IN_FRUIT_CPP_FILE

#include <cstdlib>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fruit/impl/util/type_info.h>

#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>

#include <fruit/impl/data_structures/semistatic_map.templates.h>
#include <fruit/impl/data_structures/semistatic_graph.templates.h>
#include <fruit/impl/component_storage/component_storage.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.h>
#include <fruit/impl/injector/injector_storage.h>

using std::cout;
using std::endl;

using namespace fruit;
using namespace fruit::impl;

namespace fruit {
namespace impl {

NormalizedComponentStorage::NormalizedComponentStorage(
    ComponentStorage&& component,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    MemoryPool& memory_pool,
    WithPermanentCompression)
  : bindingCompressionInfoMapMemoryPool(),
    bindingCompressionInfoMap() {

  using bindings_vector_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;
  bindings_vector_t bindings_vector =
      bindings_vector_t(ArenaAllocator<ComponentStorageEntry>(memory_pool));
  BindingNormalization::normalizeBindingsWithPermanentBindingCompression(
      std::move(component).release(),
      fixed_size_allocator_data,
      memory_pool,
      exposed_types,
      bindings_vector,
      multibindings);

  bindings = SemistaticGraph<TypeId, NormalizedBinding>(InjectorStorage::BindingDataNodeIter{bindings_vector.begin()},
                                                        InjectorStorage::BindingDataNodeIter{bindings_vector.end()},
                                                        memory_pool);
}

NormalizedComponentStorage::NormalizedComponentStorage(
    ComponentStorage&& component,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    MemoryPool& memory_pool,
    WithUndoableCompression)
  : bindingCompressionInfoMapMemoryPool(),
    bindingCompressionInfoMap(
      std::unique_ptr<BindingCompressionInfoMap>(
          new BindingCompressionInfoMap(
              createHashMapWithArenaAllocator<TypeId, CompressedBindingUndoInfo>(
                  bindingCompressionInfoMapMemoryPool)))) {

  using bindings_vector_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;
  bindings_vector_t bindings_vector =
      bindings_vector_t(ArenaAllocator<ComponentStorageEntry>(memory_pool));
  BindingNormalization::normalizeBindingsWithUndoableBindingCompression(
      std::move(component).release(),
      fixed_size_allocator_data,
      memory_pool,
      exposed_types,
      bindings_vector,
      multibindings,
      *bindingCompressionInfoMap);

  bindings = SemistaticGraph<TypeId, NormalizedBinding>(InjectorStorage::BindingDataNodeIter{bindings_vector.begin()},
                                                        InjectorStorage::BindingDataNodeIter{bindings_vector.end()},
                                                        memory_pool);
}

NormalizedComponentStorage::~NormalizedComponentStorage() {
}

} // namespace impl
// We need a LCOV_EXCL_BR_LINE below because for some reason gcov/lcov think there's a branch there.
} // namespace fruit LCOV_EXCL_BR_LINE
