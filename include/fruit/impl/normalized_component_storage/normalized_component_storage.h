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

#if !IN_FRUIT_CPP_FILE
// We don't want to include it in public headers to save some compile time.
#error "normalized_component_storage.h included in non-cpp file."
#endif

#include <fruit/impl/component_storage/component_storage_entry.h>
#include <fruit/impl/data_structures/fixed_size_allocator.h>
#include <fruit/impl/data_structures/semistatic_graph.h>
#include <fruit/impl/data_structures/semistatic_map.h>
#include <fruit/impl/fruit_internal_forward_decls.h>
#include <fruit/impl/normalized_component_storage/normalized_bindings.h>
#include <fruit/impl/util/hash_helpers.h>
#include <fruit/impl/util/type_info.h>

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
  struct CompressedBindingUndoInfo {
    TypeId i_type_id;
    ComponentStorageEntry::BindingForObjectToConstruct i_binding;
    ComponentStorageEntry::BindingForObjectToConstruct c_binding;
  };

  // A map from c_type_id to the corresponding CompressedBindingUndoInfo (if binding compression was performed for
  // c_type_id).
  using BindingCompressionInfoMap = HashMapWithArenaAllocator<TypeId, CompressedBindingUndoInfo>;
  using BindingCompressionInfoMapAllocator = BindingCompressionInfoMap::allocator_type;

  using LazyComponentWithNoArgs = ComponentStorageEntry::LazyComponentWithNoArgs;
  using LazyComponentWithArgs = ComponentStorageEntry::LazyComponentWithArgs;

  struct HashLazyComponentWithNoArgs {
    std::size_t operator()(const LazyComponentWithNoArgs& x) const {
      return x.hashCode();
    }
  };

  struct LazyComponentWithArgsEqualTo {
    bool operator()(const LazyComponentWithArgs& x, const LazyComponentWithArgs& y) const {
      return *x.component == *y.component;
    }
  };

  struct HashLazyComponentWithArgs {
    std::size_t operator()(const LazyComponentWithArgs& x) const {
      return x.component->hashCode();
    }
  };

  using LazyComponentWithNoArgsSet = HashSetWithArenaAllocator<LazyComponentWithNoArgs, HashLazyComponentWithNoArgs,
                                                               std::equal_to<LazyComponentWithNoArgs>>;
  using LazyComponentWithArgsSet =
      HashSetWithArenaAllocator<LazyComponentWithArgs, HashLazyComponentWithArgs, LazyComponentWithArgsEqualTo>;

  using LazyComponentWithNoArgsReplacementMap =
      HashMapWithArenaAllocator<LazyComponentWithNoArgs, ComponentStorageEntry,
                                NormalizedComponentStorage::HashLazyComponentWithNoArgs,
                                std::equal_to<LazyComponentWithNoArgs>>;
  using LazyComponentWithArgsReplacementMap =
      HashMapWithArenaAllocator<LazyComponentWithArgs, ComponentStorageEntry,
                                NormalizedComponentStorage::HashLazyComponentWithArgs,
                                NormalizedComponentStorage::LazyComponentWithArgsEqualTo>;

  static LazyComponentWithNoArgsSet createLazyComponentWithNoArgsSet(size_t capacity, MemoryPool& memory_pool);
  static LazyComponentWithArgsSet createLazyComponentWithArgsSet(size_t capacity, MemoryPool& memory_pool);

  static LazyComponentWithNoArgsReplacementMap createLazyComponentWithNoArgsReplacementMap(size_t capacity,
                                                                                           MemoryPool& memory_pool);
  static LazyComponentWithArgsReplacementMap createLazyComponentWithArgsReplacementMap(size_t capacity,
                                                                                       MemoryPool& memory_pool);

private:
  // A graph with types as nodes (each node stores the BindingData for the type) and dependencies as edges.
  // For types that have a constructed object already, the corresponding node is stored as terminal node.
  SemistaticGraph<TypeId, NormalizedBinding> bindings;

  // Maps the type index of a type T to the corresponding NormalizedMultibindingSet.
  std::unordered_map<TypeId, NormalizedMultibindingSet> multibindings;

  // Contains data on the set of types that can be allocated using this component.
  FixedSizeAllocator::FixedSizeAllocatorData fixed_size_allocator_data;

  // The MemoryPool used to allocate bindingCompressionInfoMap, fully_expanded_components_with_no_args and
  // fully_expanded_components_with_args.
  MemoryPool normalized_component_memory_pool;

  // Stores information on binding compression that was performed in bindings of this object.
  // See also the documentation for BindingCompressionInfoMap.
  BindingCompressionInfoMap binding_compression_info_map;

  LazyComponentWithNoArgsSet fully_expanded_components_with_no_args;
  LazyComponentWithArgsSet fully_expanded_components_with_args;

  LazyComponentWithNoArgsReplacementMap component_with_no_args_replacements;
  LazyComponentWithArgsReplacementMap component_with_args_replacements;

  friend class InjectorStorage;
  friend class BindingNormalization;

public:
  using Graph = SemistaticGraph<TypeId, NormalizedBinding>;

  NormalizedComponentStorage() = delete;

  NormalizedComponentStorage(NormalizedComponentStorage&&) = delete;
  NormalizedComponentStorage(const NormalizedComponentStorage&) = delete;

  NormalizedComponentStorage& operator=(NormalizedComponentStorage&&) = delete;
  NormalizedComponentStorage& operator=(const NormalizedComponentStorage&) = delete;

  // These are just used as tags to select the desired constructor.
  struct WithUndoableCompression {};
  struct WithPermanentCompression {};

  /**
   * The MemoryPool is only used during construction, the constructed object *can* outlive the memory pool.
   */
  NormalizedComponentStorage(ComponentStorage&& component,
                             const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types, MemoryPool& memory_pool,
                             WithUndoableCompression);

  /**
   * The MemoryPool is only used during construction, the constructed object *can* outlive the memory pool.
   */
  NormalizedComponentStorage(ComponentStorage&& component,
                             const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types, MemoryPool& memory_pool,
                             WithPermanentCompression);

  // We don't use the default destructor because that will require the inclusion of
  // the Boost's hashmap header. We define this in the cpp file instead.
  ~NormalizedComponentStorage();
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/normalized_component_storage/normalized_component_storage.defn.h>

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_H
