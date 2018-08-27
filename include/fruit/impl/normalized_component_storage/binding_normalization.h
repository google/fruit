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

#if !IN_FRUIT_CPP_FILE
// We don't want to include it in public headers to save some compile time.
#error "binding_normalization.h included in non-cpp file."
#endif

#include <fruit/impl/component_storage/component_storage_entry.h>
#include <fruit/impl/data_structures/arena_allocator.h>
#include <fruit/impl/data_structures/fixed_size_allocator.h>
#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>
#include <fruit/impl/util/hash_helpers.h>

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
  using BindingCompressionInfoMap =
      HashMapWithArenaAllocator<TypeId, NormalizedComponentStorage::CompressedBindingUndoInfo>;

  using LazyComponentWithNoArgs = ComponentStorageEntry::LazyComponentWithNoArgs;
  using LazyComponentWithArgs = ComponentStorageEntry::LazyComponentWithArgs;

  using LazyComponentWithNoArgsSet = NormalizedComponentStorage::LazyComponentWithNoArgsSet;
  using LazyComponentWithArgsSet = NormalizedComponentStorage::LazyComponentWithArgsSet;

  using LazyComponentWithNoArgsReplacementMap = NormalizedComponentStorage::LazyComponentWithNoArgsReplacementMap;
  using LazyComponentWithArgsReplacementMap = NormalizedComponentStorage::LazyComponentWithArgsReplacementMap;

  /**
   * Normalizes the toplevel entries and performs binding compression.
   * This does *not* keep track of what binding compressions were performed, so they can't be undone. When we might need
   * to undo the binding compression, use normalizeBindingsWithUndoableBindingCompression() instead.
   */
  static void normalizeBindingsWithPermanentBindingCompression(
      FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data, MemoryPool& memory_pool,
      const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
      std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
      std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings);

  /**
   * Normalizes the toplevel entries and performs binding compression, but keeps track of which compressions were
   * performed so that we can later undo some of them if needed.
   * This is more expensive than normalizeBindingsWithPermanentBindingCompression(), use that when it suffices.
   */
  static void normalizeBindingsWithUndoableBindingCompression(
      FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data, MemoryPool& memory_pool,
      MemoryPool& memory_pool_for_fully_expanded_components_maps,
      MemoryPool& memory_pool_for_component_replacements_maps,
      const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
      std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
      std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
      BindingCompressionInfoMap& bindingCompressionInfoMap,
      LazyComponentWithNoArgsSet& fully_expanded_components_with_no_args,
      LazyComponentWithArgsSet& fully_expanded_components_with_args,
      LazyComponentWithNoArgsReplacementMap& component_with_no_args_replacements,
      LazyComponentWithArgsReplacementMap& component_with_args_replacements);

  static void normalizeBindingsAndAddTo(
      FixedSizeVector<ComponentStorageEntry>&& toplevel_entries, MemoryPool& memory_pool,
      const NormalizedComponentStorage& base_normalized_component,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
      std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& new_bindings_vector,
      std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings);

private:
  using multibindings_vector_elem_t = std::pair<ComponentStorageEntry, ComponentStorageEntry>;
  using multibindings_vector_t = std::vector<multibindings_vector_elem_t, ArenaAllocator<multibindings_vector_elem_t>>;

  /**
   * Adds the multibindings in multibindings_vector to the `multibindings' map.
   * Each element of multibindings_vector is a pair, where the first element is the multibinding and the second is the
   * corresponding MULTIBINDING_VECTOR_CREATOR entry.
   */
  static void addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
                               FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                               const multibindings_vector_t& multibindings_vector);

  static void printLazyComponentInstallationLoop(
      const std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& entries_to_process,
      const ComponentStorageEntry& last_entry);

  /**
   * Normalizes the toplevel entries (but doesn't perform binding compression).
   */
  template <typename... Functors>
  static void normalizeBindings(FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
                                FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                                MemoryPool& memory_pool, MemoryPool& memory_pool_for_fully_expanded_components_maps,
                                MemoryPool& memory_pool_for_component_replacements_maps,
                                HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>& binding_data_map,
                                Functors... functors);

  struct BindingCompressionInfo {
    TypeId i_type_id;
    ComponentStorageEntry::BindingForObjectToConstruct::create_t create_i_with_compression;
  };

  /**
   * Normalizes the toplevel entries and performs binding compression.
   * - SaveCompressedBindingUndoInfo should have an operator()(TypeId, CompressedBindingUndoInfo) that will be called
   *   with (c_type_id, undo_info) for each binding compression that was applied (and that therefore might need to be
   *   undone later).
   */
  template <typename SaveCompressedBindingUndoInfo, typename SaveFullyExpandedComponentsWithNoArgs,
            typename SaveFullyExpandedComponentsWithArgs, typename SaveComponentReplacementsWithNoArgs,
            typename SaveComponentReplacementsWithArgs>
  static void normalizeBindingsWithBindingCompression(
      FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data, MemoryPool& memory_pool,
      MemoryPool& memory_pool_for_fully_expanded_components_maps,
      MemoryPool& memory_pool_for_component_replacements_maps,
      const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
      std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
      std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
      SaveCompressedBindingUndoInfo save_compressed_binding_undo_info,
      SaveFullyExpandedComponentsWithNoArgs save_fully_expanded_components_with_no_args,
      SaveFullyExpandedComponentsWithArgs save_fully_expanded_components_with_args,
      SaveComponentReplacementsWithNoArgs save_component_replacements_with_no_args,
      SaveComponentReplacementsWithArgs save_component_replacements_with_args);

  /**
   * bindingCompressionInfoMap is an output parameter. This function will store information on all performed binding
   * compressions in that map, to allow them to be undone later, if necessary.
   * compressed_bindings_map is a map CtypeId -> (ItypeId, bindingData)
   * - SaveCompressedBindingUndoInfo should have an operator()(TypeId, CompressedBindingUndoInfo) that will be called
   *   with (c_type_id, undo_info) for each binding compression that was applied (and that therefore might need to be
   *   undone later).
   */
  template <typename SaveCompressedBindingUndoInfo>
  static std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>
  performBindingCompression(HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>&& binding_data_map,
                            HashMapWithArenaAllocator<TypeId, BindingCompressionInfo>&& compressed_bindings_map,
                            MemoryPool& memory_pool, const multibindings_vector_t& multibindings_vector,
                            const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
                            SaveCompressedBindingUndoInfo save_compressed_binding_undo_info);

  static void handlePreexistingLazyComponentWithArgsReplacement(ComponentStorageEntry& replaced_component_entry,
                                                                const ComponentStorageEntry& preexisting_replacement,
                                                                ComponentStorageEntry& new_replacement);

  static void handlePreexistingLazyComponentWithNoArgsReplacement(ComponentStorageEntry& replaced_component_entry,
                                                                  const ComponentStorageEntry& preexisting_replacement,
                                                                  ComponentStorageEntry& new_replacement);

  template <typename HandleCompressedBinding, typename HandleMultibinding, typename FindNormalizedBinding,
            typename IsValidItr, typename IsNormalizedBindingItrForConstructedObject, typename GetObjectPtr,
            typename GetCreate, typename IsComponentWithNoArgsAlreadyExpandedInNormalizedComponent,
            typename IsComponentWithArgsAlreadyExpandedInNormalizedComponent,
            typename SaveFullyExpandedComponentsWithNoArgs, typename SaveFullyExpandedComponentsWithArgs,
            typename GetComponentWithNoArgsReplacementInNormalizedComponent,
            typename GetComponentWithArgsReplacementInNormalizedComponent,
            typename IsLazyComponentWithNoArgsIteratorValid, typename IsLazyComponentWithArgsIteratorValid,
            typename DereferenceLazyComponentWithNoArgsIterator, typename DereferenceLazyComponentWithArgsIterator,
            typename SaveComponentReplacementsWithNoArgs, typename SaveComponentReplacementsWithArgs>
  struct BindingNormalizationFunctors {

    /**
     * This should have an operator()(ComponentStorageEntry&) that will be called for each COMPRESSED_BINDING entry.
     */
    HandleCompressedBinding handle_compressed_binding;

    /**
     * This should have an
     * operator()(ComponentStorageEntry& multibinding_entry, ComponentStorageEntry& multibinding_vector_creator_entry)
     * that will be called for each multibinding entry.
     */
    HandleMultibinding handle_multibinding;

    /**
     * This should have a
     * NormalizedBindingItr operator()(TypeId)
     * that returns a NormalizedBindingItr describing whether the binding is present in a base component (if any).
     */
    FindNormalizedBinding find_normalized_binding;

    /**
     * This should have a
     * bool operator()(NormalizedBindingItr)
     */
    IsValidItr is_valid_itr;

    /**
     * This should have a
     * bool operator()(NormalizedBindingItr)
     * (that can only be used when IsValidItr returns true).
     */
    IsNormalizedBindingItrForConstructedObject is_normalized_binding_itr_for_constructed_object;

    /**
     * This should have a
     * ComponentStorageEntry::BindingForConstructedObject::object_ptr_t operator()(NormalizedBindingItr)
     * (that can only be used when IsNormalizedBindingItrForConstructedObject returns true).
     */
    GetObjectPtr get_object_ptr;

    /**
     * This should have a
     * ComponentStorageEntry::BindingForObjectToConstruct::create_t operator()(NormalizedBindingItr)
     * (that can only be used when IsNormalizedBindingItrForConstructedObject returns false).
     */
    GetCreate get_create;

    IsComponentWithNoArgsAlreadyExpandedInNormalizedComponent
        is_component_with_no_args_already_expanded_in_normalized_component;
    IsComponentWithArgsAlreadyExpandedInNormalizedComponent
        is_component_with_args_already_expanded_in_normalized_component;
    SaveFullyExpandedComponentsWithNoArgs save_fully_expanded_components_with_no_args;
    SaveFullyExpandedComponentsWithArgs save_fully_expanded_components_with_args;

    /**
     * Gets a LazyComponentWithNoArgsIterator pointing to the replacement for the given lazy component in the normalized
     * component (if any).
     */
    GetComponentWithNoArgsReplacementInNormalizedComponent
        get_component_with_no_args_replacement_in_normalized_component;

    /**
     * Gets a LazyComponentWithArgsIterator pointing to the replacement for the given lazy component in the normalized
     * component (if any).
     */
    GetComponentWithArgsReplacementInNormalizedComponent get_component_with_args_replacement_in_normalized_component;

    IsLazyComponentWithNoArgsIteratorValid is_lazy_component_with_no_args_iterator_valid;
    IsLazyComponentWithArgsIteratorValid is_lazy_component_with_args_iterator_valid;

    DereferenceLazyComponentWithNoArgsIterator dereference_lazy_component_with_no_args_iterator;
    DereferenceLazyComponentWithArgsIterator dereference_lazy_component_with_args_iterator;

    SaveComponentReplacementsWithNoArgs save_component_replacements_with_no_args;
    SaveComponentReplacementsWithArgs save_component_replacements_with_args;
  };

  /**
   * This struct groups all data structures available during binding normalization, to avoid mentioning them in all
   * handle*Binding functions below.
   */
  template <typename... Functors>
  struct BindingNormalizationContext {
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data;
    MemoryPool& memory_pool;
    MemoryPool& memory_pool_for_fully_expanded_components_maps;
    MemoryPool& memory_pool_for_component_replacements_maps;
    HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>& binding_data_map;
    BindingNormalizationFunctors<Functors...> functors;

    // These are in reversed order (note that toplevel_entries must also be in reverse order).
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>> entries_to_process;

    // These sets contain the lazy components whose expansion has already completed.
    LazyComponentWithNoArgsSet fully_expanded_components_with_no_args =
        NormalizedComponentStorage::createLazyComponentWithNoArgsSet(20 /* capacity */,
                                                                     memory_pool_for_fully_expanded_components_maps);
    LazyComponentWithArgsSet fully_expanded_components_with_args =
        NormalizedComponentStorage::createLazyComponentWithArgsSet(20 /* capacity */,
                                                                   memory_pool_for_fully_expanded_components_maps);

    // These sets contain the elements with kind *_END_MARKER in entries_to_process.
    // For component with args, these sets do *not* own the objects, entries_to_process does.
    LazyComponentWithNoArgsSet components_with_no_args_with_expansion_in_progress =
        NormalizedComponentStorage::createLazyComponentWithNoArgsSet(20 /* capacity */, memory_pool);
    LazyComponentWithArgsSet components_with_args_with_expansion_in_progress =
        NormalizedComponentStorage::createLazyComponentWithArgsSet(20 /* capacity */, memory_pool);

    // These sets contain Component replacements, as mappings componentToReplace->replacementComponent.
    LazyComponentWithNoArgsReplacementMap component_with_no_args_replacements =
        NormalizedComponentStorage::createLazyComponentWithNoArgsReplacementMap(
            20 /* capacity */, memory_pool_for_component_replacements_maps);
    LazyComponentWithArgsReplacementMap component_with_args_replacements =
        NormalizedComponentStorage::createLazyComponentWithArgsReplacementMap(
            20 /* capacity */, memory_pool_for_component_replacements_maps);

    BindingNormalizationContext(FixedSizeVector<ComponentStorageEntry>& toplevel_entries,
                                FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                                MemoryPool& memory_pool, MemoryPool& memory_pool_for_fully_expanded_components_maps,
                                MemoryPool& memory_pool_for_component_replacements_maps,
                                HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>& binding_data_map,
                                BindingNormalizationFunctors<Functors...> functors);

    BindingNormalizationContext(const BindingNormalizationContext&) = delete;
    BindingNormalizationContext(BindingNormalizationContext&&) = delete;

    BindingNormalizationContext& operator=(const BindingNormalizationContext&) = delete;
    BindingNormalizationContext& operator=(BindingNormalizationContext&&) = delete;

    ~BindingNormalizationContext();
  };

  template <typename... Params>
  static void handleBindingForConstructedObject(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleBindingForObjectToConstructThatNeedsAllocation(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleBindingForObjectToConstructThatNeedsNoAllocation(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleCompressedBinding(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleMultibinding(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleMultibindingVectorCreator(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleComponentWithoutArgsEndMarker(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleComponentWithArgsEndMarker(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleReplacedLazyComponentWithArgs(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleReplacedLazyComponentWithNoArgs(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleLazyComponentWithArgs(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void handleLazyComponentWithNoArgs(BindingNormalizationContext<Params...>& context);

  template <typename... Params>
  static void performComponentReplacement(BindingNormalizationContext<Params...>& context,
                                          const ComponentStorageEntry& replacement);

  static void printMultipleBindingsError(TypeId type);

  static void printIncompatibleComponentReplacementsError(const ComponentStorageEntry& replaced_component_entry,
                                                          const ComponentStorageEntry& replacement_component_entry1,
                                                          const ComponentStorageEntry& replacement_component_entry2);

  static void
  printComponentReplacementFailedBecauseTargetAlreadyExpanded(const ComponentStorageEntry& replaced_component_entry,
                                                              const ComponentStorageEntry& replacement_component_entry);
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDING_NORMALIZATION_H
