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

  /**
   * Normalizes the toplevel entries (but doesn't perform binding compression).
   * Each element of multibindings_vector is a pair, where the first element is the multibinding and the second is the
   * corresponding MULTIBINDING_VECTOR_CREATOR entry.
   */
  static void normalizeBindings(
      FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
      TypeId toplevel_component_fun_type_id,
      const std::vector<TypeId>& exposed_types,
      std::vector<ComponentStorageEntry>& bindings_vector,
      std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
      BindingCompressionInfoMap& bindingCompressionInfoMap);

  /**
   * - FindNormalizedBinding should have a
   *   NormalizedBindingItr operator()(TypeId)
   *   that returns a NormalizedBindingItr describing whether the binding is present in a base component (if any).
   * - IsValidItr should have a
   *   bool operator()(NormalizedBindingItr)
   * - IsNormalizedBindingItrForConstructedObject should have a
   *   bool operator()(NormalizedBindingItr)
   *   (that can only be used when IsValidItr returns true)
   * - GetObjectPtr should have a
   *   ComponentStorageEntry::BindingForConstructedObject::object_ptr_t operator()(NormalizedBindingItr)
   *   (that can only be used when IsNormalizedBindingItrForConstructedObject returns true)
   * - GetCreate should have a
   *   ComponentStorageEntry::BindingForObjectToConstruct::create_t operator()(NormalizedBindingItr)
   *   (that can only be used when IsNormalizedBindingItrForConstructedObject returns false).
   */
  template <
      typename FindNormalizedBinding,
      typename IsValidItr,
      typename IsNormalizedBindingItrForConstructedObject,
      typename GetObjectPtr,
      typename GetCreate>
  static void normalizeBindingsAndAddTo(
      FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
      TypeId toplevel_component_fun_type_id,
      const FixedSizeAllocator::FixedSizeAllocatorData& base_fixed_size_allocator_data,
      const std::unordered_map<TypeId, NormalizedMultibindingSet>& base_multibindings,
      const NormalizedComponentStorage::BindingCompressionInfoMap& base_binding_compression_info_map,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
      std::vector<ComponentStorageEntry>& new_bindings_vector,
      std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
      FindNormalizedBinding find_normalized_binding,
      IsValidItr is_valid_itr,
      IsNormalizedBindingItrForConstructedObject is_normalized_binding_itr_for_constructed_object,
      GetObjectPtr get_object_ptr,
      GetCreate get_create);

  /**
   * Adds the multibindings in multibindings_vector to the `multibindings' map.
   * Each element of multibindings_vector is a pair, where the first element is the multibinding and the second is the
   * corresponding MULTIBINDING_VECTOR_CREATOR entry.
   */
  static void addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
                               FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                               std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>&& multibindings_vector);

private:

  static void printLazyComponentInstallationLoop(
      TypeId toplevel_component_fun_type_id,
      const std::vector<ComponentStorageEntry>& entries_to_process,
      const ComponentStorageEntry& last_entry);

  /**
   * Normalizes the toplevel entries (but doesn't perform binding compression).
   * - HandleCompressedBinding should have an operator()(ComponentStorageEntry&) that will be called for each
   *   COMPRESSED_BINDING entry.
   * - HandleMultibinding should have an
   *   operator()(ComponentStorageEntry& multibinding_entry, ComponentStorageEntry& multibinding_vector_creator_entry)
   *   that will be called for each multibinding entry.
   * - FindNormalizedBinding should have a
   *   NormalizedBindingItr operator()(TypeId)
   *   that returns a NormalizedBindingItr describing whether the binding is present in a base component (if any).
   * - IsValidItr should have a
   *   bool operator()(NormalizedBindingItr)
   * - IsNormalizedBindingItrForConstructedObject should have a
   *   bool operator()(NormalizedBindingItr)
   *   (that can only be used when IsValidItr returns true)
   * - GetObjectPtr should have a
   *   ComponentStorageEntry::BindingForConstructedObject::object_ptr_t operator()(NormalizedBindingItr)
   *   (that can only be used when IsNormalizedBindingItrForConstructedObject returns true)
   * - GetCreate should have a
   *   ComponentStorageEntry::BindingForObjectToConstruct::create_t operator()(NormalizedBindingItr)
   *   (that can only be used when IsNormalizedBindingItrForConstructedObject returns false).
   */
  template <
      typename HandleCompressedBinding,
      typename HandleMultibinding,
      typename FindNormalizedBinding,
      typename IsValidItr,
      typename IsNormalizedBindingItrForConstructedObject,
      typename GetObjectPtr,
      typename GetCreate>
  static void normalizeBindingsHelper(
      FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
      TypeId toplevel_component_fun_type_id,
      HashMap<TypeId, ComponentStorageEntry>& binding_data_map,
      HandleCompressedBinding handle_compressed_binding,
      HandleMultibinding handle_multibinding,
      FindNormalizedBinding find_normalized_binding,
      IsValidItr is_valid_itr,
      IsNormalizedBindingItrForConstructedObject is_normalized_binding_itr_for_constructed_object,
      GetObjectPtr get_object_ptr,
      GetCreate get_create);

  struct BindingCompressionInfo {
    TypeId i_type_id;
    ComponentStorageEntry::BindingForObjectToConstruct::create_t create_i_with_compression;
  };

  // bindingCompressionInfoMap is an output parameter. This function will store
  // information on all performed binding compressions
  // in that map, to allow them to be undone later, if necessary.
  // compressed_bindings_map is a map CtypeId -> (ItypeId, bindingData)
  static std::vector<ComponentStorageEntry> performBindingCompression(
      HashMap<TypeId, ComponentStorageEntry>&& binding_data_map,
      HashMap<TypeId, BindingCompressionInfo>&& compressed_bindings_map,
      const std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>& multibindings_vector,
      const std::vector<TypeId>& exposed_types,
      BindingCompressionInfoMap& bindingCompressionInfoMap);
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDING_NORMALIZATION_H
