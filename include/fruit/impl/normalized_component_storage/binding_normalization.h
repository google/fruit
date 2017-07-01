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

#ifndef FRUIT_BINDING_NORMALIZATION_H
#define FRUIT_BINDING_NORMALIZATION_H

#ifndef IN_FRUIT_CPP_FILE
// We don't want to include it in public headers to save some compile time.
#error "binding_normalization.h included in non-cpp file."
#endif

#include <fruit/impl/util/hash_helpers.h>
#include <fruit/impl/data_structures/fixed_size_allocator.h>
#include <fruit/impl/component_storage/component_storage_entry.h>
#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>

namespace fruit {
namespace impl {

/**
 * This struct contains helper functions used for binding normalization.
 * They are wrapped in a struct so that Fruit classes can easily declare to be friend
 * of all these.
 */
class BindingNormalization {
public:
  
  // Stores an element of the form (c_type_id, -> undo_info) for each binding compression that was
  // performed.
  // These are used to undo binding compression after applying it (if necessary).
  using BindingCompressionInfoMap = HashMap<TypeId, NormalizedComponentStorage::CompressedBindingUndoInfo>;

  static void split_component_storage_entries(
      std::vector<ComponentStorageEntry>&& all_entries_vector,
      std::vector<ComponentStorageEntry>& bindings_vector,
      std::vector<ComponentStorageEntry>& compressed_bindings_vector,
      std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>& multibindings_vector);

  /**
   * This is the first step of binding normalization: the lazy components in `storage` are expanded.
   * After this call, `storage` no longer contains any entries for lazy components.
   */
  static std::vector<ComponentStorageEntry> expandLazyComponents(
      FixedSizeVector<ComponentStorageEntry>&& storage, TypeId toplevel_component_fun_type_id);

  // bindingCompressionInfoMap is an output parameter. This function will store
  // information on all performed binding compressions
  // in that map, to allow them to be undone later, if necessary.
  static void normalizeBindings(
      std::vector<ComponentStorageEntry>& bindings_vector,
      std::vector<ComponentStorageEntry>&& compressed_bindings_vector,
      const std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>& multibindings_vector,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
      const std::vector<TypeId>& exposed_types,
      BindingCompressionInfoMap& bindingCompressionInfoMap);

  /**
   * Adds the multibindings in multibindings_vector to the `multibindings' map.
   * Each element of multibindings_vector is a pair, where the first element is the multibinding and the second is the
   * corresponding MULTIBINDING_VECTOR_CREATOR entry.
   */
  static void addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
                               FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                               std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>&& multibindings_vector);

};

} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDING_NORMALIZATION_H
