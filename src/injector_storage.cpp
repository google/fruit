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

#include <fruit/impl/component_storage/component_storage.h>
#include <fruit/impl/data_structures/semistatic_graph.templates.h>
#include <fruit/impl/injector/injector_storage.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.templates.h>

using std::cout;
using std::endl;

using namespace fruit::impl;

namespace fruit {
namespace impl {

void InjectorStorage::fatal(const std::string& error) {
  std::cerr << "Fatal injection error: " << error << std::endl;
  exit(1);
}

// LCOV_EXCL_START
namespace {
template <typename Id, typename Value>
struct DummyNode {
  Id getId() {
    return Id();
  }
  bool isTerminal() {
    return false;
  }
  Id* getEdgesBegin() {
    return nullptr;
  }
  Id* getEdgesEnd() {
    return nullptr;
  }
  Value getValue() {
    return Value();
  }
};
}
// LCOV_EXCL_STOP

InjectorStorage::InjectorStorage(ComponentStorage&& component,
                                 const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
                                 MemoryPool& memory_pool)
    : normalized_component_storage_ptr(new NormalizedComponentStorage(
          std::move(component), exposed_types, memory_pool, NormalizedComponentStorage::WithPermanentCompression())),
      allocator(normalized_component_storage_ptr->fixed_size_allocator_data),
      bindings(normalized_component_storage_ptr->bindings, (DummyNode<TypeId, NormalizedBinding>*)nullptr,
               (DummyNode<TypeId, NormalizedBinding>*)nullptr, memory_pool),
      multibindings(std::move(normalized_component_storage_ptr->multibindings)) {

#if FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

InjectorStorage::InjectorStorage(const NormalizedComponentStorage& normalized_component, ComponentStorage&& component,
                                 MemoryPool& memory_pool) {

  FixedSizeAllocator::FixedSizeAllocatorData fixed_size_allocator_data;
  using new_bindings_vector_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;
  new_bindings_vector_t new_bindings_vector = new_bindings_vector_t(ArenaAllocator<ComponentStorageEntry>(memory_pool));

  BindingNormalization::normalizeBindingsAndAddTo(std::move(component).release(), memory_pool, normalized_component,
                                                  fixed_size_allocator_data, new_bindings_vector, multibindings);

  allocator = FixedSizeAllocator(fixed_size_allocator_data);

  bindings = Graph(normalized_component.bindings, BindingDataNodeIter{new_bindings_vector.begin()},
                   BindingDataNodeIter{new_bindings_vector.end()}, memory_pool);
#if FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

InjectorStorage::~InjectorStorage() {}

void InjectorStorage::ensureConstructedMultibinding(NormalizedMultibindingSet& multibinding_set) {
  for (NormalizedMultibinding& multibinding : multibinding_set.elems) {
    if (!multibinding.is_constructed) {
      multibinding.object = multibinding.create(*this);
      multibinding.is_constructed = true;
    }
  }
}

void* InjectorStorage::getMultibindings(TypeId typeInfo) {
  NormalizedMultibindingSet* multibinding_set = getNormalizedMultibindingSet(typeInfo);
  if (multibinding_set == nullptr) {
    // Not registered.
    return nullptr;
  }
  return multibinding_set->get_multibindings_vector(*this).get();
}

void InjectorStorage::eagerlyInjectMultibindings() {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  for (auto& typeInfoInfoPair : multibindings) {
    typeInfoInfoPair.second.get_multibindings_vector(*this);
  }
}

} // namespace impl
// We need a LCOV_EXCL_BR_LINE below because for some reason gcov/lcov think there's a branch there.
} // namespace fruit LCOV_EXCL_BR_LINE
