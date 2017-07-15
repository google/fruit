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

#ifndef FRUIT_BINDING_NORMALIZATION_TEMPLATES_H
#define FRUIT_BINDING_NORMALIZATION_TEMPLATES_H

#ifndef IN_FRUIT_CPP_FILE
// We don't want to include it in public headers to save some compile time.
#error "binding_normalization.templates.h included in non-cpp file."
#endif

#include <fruit/impl/component_storage/component_storage_entry.h>
#include <fruit/impl/util/type_info.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.h>

using namespace fruit::impl;

namespace fruit {
namespace impl {

template <
    typename HandleCompressedBinding,
    typename HandleMultibinding,
    typename FindNormalizedBinding,
    typename IsValidItr,
    typename IsNormalizedBindingItrForConstructedObject,
    typename GetObjectPtr,
    typename GetCreate>
BindingNormalization::BindingNormalizationContext<
      HandleCompressedBinding,
      HandleMultibinding,
      FindNormalizedBinding,
      IsValidItr,
      IsNormalizedBindingItrForConstructedObject,
      GetObjectPtr,
      GetCreate>::BindingNormalizationContext(
    FixedSizeVector<ComponentStorageEntry>& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    TypeId toplevel_component_fun_type_id,
    MemoryPool& memory_pool,
    HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>& binding_data_map,
    HandleCompressedBinding handle_compressed_binding,
    HandleMultibinding handle_multibinding,
    FindNormalizedBinding find_normalized_binding,
    IsValidItr is_valid_itr,
    IsNormalizedBindingItrForConstructedObject is_normalized_binding_itr_for_constructed_object,
    GetObjectPtr get_object_ptr,
    GetCreate get_create)
  : fixed_size_allocator_data(fixed_size_allocator_data),
    toplevel_component_fun_type_id(toplevel_component_fun_type_id),
    memory_pool(memory_pool),
    binding_data_map(binding_data_map),
    handle_compressed_binding(handle_compressed_binding),
    handle_multibinding(handle_multibinding),
    find_normalized_binding(find_normalized_binding),
    is_valid_itr(is_valid_itr),
    is_normalized_binding_itr_for_constructed_object(is_normalized_binding_itr_for_constructed_object),
    get_object_ptr(get_object_ptr),
    get_create(get_create),
    entries_to_process(
        toplevel_entries.begin(), toplevel_entries.end(), ArenaAllocator<ComponentStorageEntry>(memory_pool)) {

  toplevel_entries.clear();
}

template <
    typename HandleCompressedBinding,
    typename HandleMultibinding,
    typename FindNormalizedBinding,
    typename IsValidItr,
    typename IsNormalizedBindingItrForConstructedObject,
    typename GetObjectPtr,
    typename GetCreate>
BindingNormalization::BindingNormalizationContext<
      HandleCompressedBinding,
      HandleMultibinding,
      FindNormalizedBinding,
      IsValidItr,
      IsNormalizedBindingItrForConstructedObject,
      GetObjectPtr,
      GetCreate>::~BindingNormalizationContext() {
  FruitAssert(components_with_no_args_with_expansion_in_progress.empty());
  FruitAssert(components_with_args_with_expansion_in_progress.empty());

  for (const ComponentStorageEntry::LazyComponentWithArgs& x : fully_expanded_components_with_args) {
    x.destroy();
  }
}


template <
    typename HandleCompressedBinding,
    typename HandleMultibinding,
    typename FindNormalizedBinding,
    typename IsValidItr,
    typename IsNormalizedBindingItrForConstructedObject,
    typename GetObjectPtr,
    typename GetCreate>
void BindingNormalization::normalizeBindings(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    TypeId toplevel_component_fun_type_id,
    MemoryPool& memory_pool,
    HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>& binding_data_map,
    HandleCompressedBinding handle_compressed_binding,
    HandleMultibinding handle_multibinding,
    FindNormalizedBinding find_normalized_binding,
    IsValidItr is_valid_itr,
    IsNormalizedBindingItrForConstructedObject is_normalized_binding_itr_for_constructed_object,
    GetObjectPtr get_object_ptr,
    GetCreate get_create) {

  FruitAssert(binding_data_map.empty());

  BindingNormalizationContext<
      HandleCompressedBinding,
      HandleMultibinding,
      FindNormalizedBinding,
      IsValidItr,
      IsNormalizedBindingItrForConstructedObject,
      GetObjectPtr,
      GetCreate> context(
          toplevel_entries,
          fixed_size_allocator_data,
          toplevel_component_fun_type_id,
          memory_pool,
          binding_data_map,
          handle_compressed_binding,
          handle_multibinding,
          find_normalized_binding,
          is_valid_itr,
          is_normalized_binding_itr_for_constructed_object,
          get_object_ptr,
          get_create);

  // When we expand a lazy component, instead of removing it from the stack we change its kind (in entries_to_process)
  // to one of the *_END_MARKER kinds. This allows to keep track of the "call stack" for the expansion.

  while (!context.entries_to_process.empty()) {
    switch (context.entries_to_process.back().kind) { // LCOV_EXCL_BR_LINE
    case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
      handleBindingForConstructedObject(context);
      break;

    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
      handleBindingForObjectToConstructThatNeedsAllocation(context);
      break;

    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
      handleBindingForObjectToConstructThatNeedsNoAllocation(context);
      break;

    case ComponentStorageEntry::Kind::COMPRESSED_BINDING:
      handleCompressedBinding(context);
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
      handleMultibinding(context);
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR:
      handleMultibindingVectorCreator(context);
      break;

    case ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER:
      handleComponentWithoutArgsEndMarker(context);
      break;

    case ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER:
      handleComponentWithArgsEndMarker(context);
      break;

    case ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS:
      handleReplacedLazyComponentWithArgs(context);
      break;

    case ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_NO_ARGS:
      handleReplacedLazyComponentWithNoArgs(context);
      break;

    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS:
      handleLazyComponentWithArgs(context);
      break;

    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS:
      handleLazyComponentWithNoArgs(context);
      break;

    default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)context.entries_to_process.back().kind << std::endl;
#endif
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
  }
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleBindingForConstructedObject(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT);

  context.entries_to_process.pop_back();

  auto itr = context.find_normalized_binding(entry.type_id);
  if (context.is_valid_itr(itr)) {
    if (!context.is_normalized_binding_itr_for_constructed_object(itr)
        || context.get_object_ptr(itr) != entry.binding_for_constructed_object.object_ptr) {
      printMultipleBindingsError(entry.type_id);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
    // Otherwise ok, duplicate but consistent binding.
    return;
  }

  ComponentStorageEntry& entry_in_map = context.binding_data_map[entry.type_id];
  if (entry_in_map.type_id.type_info != nullptr) {
    if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT
        || entry.binding_for_constructed_object.object_ptr
            != entry_in_map.binding_for_constructed_object.object_ptr) {
      printMultipleBindingsError(entry.type_id);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
    // Otherwise ok, duplicate but consistent binding.
    return;
  }

  // New binding, add it to the map.
  entry_in_map = std::move(entry);
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleBindingForObjectToConstructThatNeedsAllocation(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION);
  context.entries_to_process.pop_back();

  auto itr = context.find_normalized_binding(entry.type_id);
  if (context.is_valid_itr(itr)) {
    if (context.is_normalized_binding_itr_for_constructed_object(itr)
        || context.get_create(itr) != entry.binding_for_object_to_construct.create) {
      printMultipleBindingsError(entry.type_id);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
    // Otherwise ok, duplicate but consistent binding.
    return;
  }

  ComponentStorageEntry& entry_in_map = context.binding_data_map[entry.type_id];
  context.fixed_size_allocator_data.addType(entry.type_id);
  if (entry_in_map.type_id.type_info != nullptr) {
    if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
        || entry.binding_for_object_to_construct.create
            != entry_in_map.binding_for_object_to_construct.create) {
      printMultipleBindingsError(entry.type_id);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
    // Otherwise ok, duplicate but consistent binding.
    return;
  }

  // New binding, add it to the map.
  entry_in_map = std::move(entry);
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleBindingForObjectToConstructThatNeedsNoAllocation(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
  context.entries_to_process.pop_back();

  auto itr = context.find_normalized_binding(entry.type_id);
  if (context.is_valid_itr(itr)) {
    if (context.is_normalized_binding_itr_for_constructed_object(itr)
        || context.get_create(itr) != entry.binding_for_object_to_construct.create) {
      printMultipleBindingsError(entry.type_id);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
    // Otherwise ok, duplicate but consistent binding.
    return;
  }

  ComponentStorageEntry& entry_in_map = context.binding_data_map[entry.type_id];
  context.fixed_size_allocator_data.addExternallyAllocatedType(entry.type_id);
  if (entry_in_map.type_id.type_info != nullptr) {
    if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION
        || entry.binding_for_object_to_construct.create
            != entry_in_map.binding_for_object_to_construct.create) {
      printMultipleBindingsError(entry.type_id);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
    // Otherwise ok, duplicate but consistent binding.
    return;
  }

  // New binding, add it to the map.
  entry_in_map = std::move(entry);
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleCompressedBinding(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::COMPRESSED_BINDING);
  context.entries_to_process.pop_back();
  context.handle_compressed_binding(entry);
}

template <typename... Params>
void BindingNormalization::handleMultibinding(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT
      || entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
      || entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
  context.entries_to_process.pop_back();
  FruitAssert(!context.entries_to_process.empty());
  ComponentStorageEntry vector_creator_entry = std::move(context.entries_to_process.back());
  context.entries_to_process.pop_back();
  FruitAssert(vector_creator_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR);
  context.handle_multibinding(entry, vector_creator_entry);
}

template <typename... Params>
void BindingNormalization::handleMultibindingVectorCreator(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR);
  context.entries_to_process.pop_back();
  FruitAssert(!context.entries_to_process.empty());
  ComponentStorageEntry multibinding_entry = std::move(context.entries_to_process.back());
  context.entries_to_process.pop_back();
  FruitAssert(
      multibinding_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT
          || multibinding_entry.kind
              == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
          || multibinding_entry.kind
              == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
  context.handle_multibinding(multibinding_entry, entry);
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleComponentWithoutArgsEndMarker(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER);
  context.entries_to_process.pop_back();
  // A lazy component expansion has completed; we now move the component from
  // components_with_*_with_expansion_in_progress to fully_expanded_components_*.

  context.components_with_no_args_with_expansion_in_progress.erase(entry.lazy_component_with_no_args);
  context.fully_expanded_components_with_no_args.insert(std::move(entry.lazy_component_with_no_args));
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleComponentWithArgsEndMarker(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER);
  context.entries_to_process.pop_back();
  // A lazy component expansion has completed; we now move the component from
  // components_with_*_with_expansion_in_progress to fully_expanded_components_*.

  context.components_with_args_with_expansion_in_progress.erase(entry.lazy_component_with_args);
  context.fully_expanded_components_with_args.insert(std::move(entry.lazy_component_with_args));
}

template <typename... Params>
void BindingNormalization::handleReplacedLazyComponentWithArgs(BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS);
  ComponentStorageEntry replaced_component_entry = std::move(entry);
  context.entries_to_process.pop_back();
  FruitAssert(!context.entries_to_process.empty());

  ComponentStorageEntry replacement_component_entry = std::move(context.entries_to_process.back());
  context.entries_to_process.pop_back();
  FruitAssert(
      replacement_component_entry.kind == ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS
      || replacement_component_entry.kind == ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS);

  if (context.components_with_args_with_expansion_in_progress.count(entry.lazy_component_with_args) != 0
      || context.fully_expanded_components_with_args.count(entry.lazy_component_with_args) != 0) {
    printComponentReplacementFailedBecauseTargetAlreadyExpanded(replaced_component_entry, replacement_component_entry);
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  ComponentStorageEntry& replacement_component_entry_in_map =
      context.component_with_args_replacements[replaced_component_entry.lazy_component_with_args];
  if (replacement_component_entry_in_map.type_id.type_info == nullptr) {
    // We just inserted replaced_component_entry.lazy_component_with_args in the map, so it's now owned by the
    // map.
    replacement_component_entry_in_map = replacement_component_entry;
  } else {
    // The map already contained a replacement component, we must check that they are consistent.
    switch (replacement_component_entry.kind) { // LCOV_EXCL_BR_LINE
    case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
      if (replacement_component_entry_in_map.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS
          || replacement_component_entry_in_map.lazy_component_with_no_args.erased_fun
              != replacement_component_entry.lazy_component_with_no_args.erased_fun) {
        printIncompatibleComponentReplacementsError(
            replaced_component_entry, replacement_component_entry, replacement_component_entry_in_map);
        FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
      }

      // Duplicate but consistent replacement, we'll ignore it.
      replaced_component_entry.lazy_component_with_args.destroy();
      break;

    case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
      if (replacement_component_entry_in_map.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS
          || !(*replacement_component_entry_in_map.lazy_component_with_args.component
              == *replacement_component_entry.lazy_component_with_args.component)) {
        printIncompatibleComponentReplacementsError(
            replaced_component_entry, replacement_component_entry, replacement_component_entry_in_map);
        FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
      }

      // Duplicate but consistent replacement, we'll ignore it.
      replaced_component_entry.lazy_component_with_args.destroy();
      replacement_component_entry.lazy_component_with_args.destroy();
      break;

    default:
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
  }
}

template <typename... Params>
void BindingNormalization::handleReplacedLazyComponentWithNoArgs(BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_NO_ARGS);
  ComponentStorageEntry replaced_component_entry = std::move(entry);
  context.entries_to_process.pop_back();
  FruitAssert(!context.entries_to_process.empty());

  ComponentStorageEntry replacement_component_entry = std::move(context.entries_to_process.back());
  context.entries_to_process.pop_back();
  FruitAssert(
      replacement_component_entry.kind == ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS
      || replacement_component_entry.kind == ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS);

  if (context.components_with_no_args_with_expansion_in_progress.count(entry.lazy_component_with_no_args) != 0
      || context.fully_expanded_components_with_no_args.count(entry.lazy_component_with_no_args) != 0) {
    printComponentReplacementFailedBecauseTargetAlreadyExpanded(replaced_component_entry, replacement_component_entry);
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  ComponentStorageEntry& replacement_component_entry_in_map =
      context.component_with_no_args_replacements[replaced_component_entry.lazy_component_with_no_args];
  if (replacement_component_entry_in_map.type_id.type_info == nullptr) {
    // We just inserted replaced_component_entry.lazy_component_with_args in the map, so it's now owned by the
    // map.
    replacement_component_entry_in_map = replacement_component_entry;
  } else {
    // The map already contained a replacement component, we must check that they are consistent.
    switch (replacement_component_entry.kind) { // LCOV_EXCL_BR_LINE
    case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
      if (replacement_component_entry_in_map.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS
          || replacement_component_entry_in_map.lazy_component_with_no_args.erased_fun
              != replacement_component_entry.lazy_component_with_no_args.erased_fun) {
        printIncompatibleComponentReplacementsError(
            replaced_component_entry, replacement_component_entry, replacement_component_entry_in_map);
        FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
      }

      // Duplicate but consistent replacement, we'll ignore it.
      break;

    case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
      if (replacement_component_entry_in_map.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS
          || !(*replacement_component_entry_in_map.lazy_component_with_args.component
              == *replacement_component_entry.lazy_component_with_args.component)) {
        printIncompatibleComponentReplacementsError(
            replaced_component_entry, replacement_component_entry, replacement_component_entry_in_map);
        FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
      }

      // Duplicate but consistent replacement, we'll ignore it.
      replacement_component_entry.lazy_component_with_args.destroy();
      break;

    default:
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
  }
}

template <typename... Params>
void BindingNormalization::performComponentReplacement(
    BindingNormalizationContext<Params...>& context, const ComponentStorageEntry& replacement) {

  // Instead of removing the entry from context.entries_to_process, we just replace it with the new component entry.

  ComponentStorageEntry& entry = context.entries_to_process.back();

  switch (replacement.kind) { // LCOV_EXCL_BR_LINE
  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    entry.kind = ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS;
    entry.type_id = replacement.type_id;
    entry.lazy_component_with_args = replacement.lazy_component_with_args.copy();
    break;

  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
    entry.kind = ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS;
    entry.type_id = replacement.type_id;
    entry.lazy_component_with_no_args = replacement.lazy_component_with_no_args;
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleLazyComponentWithArgs(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS);
  if (context.fully_expanded_components_with_args.count(entry.lazy_component_with_args)) {
    // This lazy component was already inserted, skip it.
    entry.lazy_component_with_args.destroy();
    context.entries_to_process.pop_back();
    return;
  }

  auto replacement_component_itr = context.component_with_args_replacements.find(entry.lazy_component_with_args);
  if (replacement_component_itr != context.component_with_args_replacements.end()) {
    entry.lazy_component_with_args.destroy();
    performComponentReplacement(context, replacement_component_itr->second);
    return;
  }

  bool actually_inserted =
      context.components_with_args_with_expansion_in_progress.insert(entry.lazy_component_with_args).second;
  if (!actually_inserted) {
    printLazyComponentInstallationLoop(
        context.toplevel_component_fun_type_id, context.entries_to_process, entry);
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

#ifdef FRUIT_EXTRA_DEBUG
  std::cout << "Expanding lazy component: " << entry.lazy_component_with_args.component->getFunTypeId() << std::endl;
#endif

  // Instead of removing the component from component.lazy_components, we just change its kind to the
  // corresponding *_END_MARKER kind.
  // When we pop this marker, this component's expansion will be complete.
  context.entries_to_process.back().kind = ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER;

  // Note that this can also add other lazy components, so the resulting bindings can have a non-intuitive
  // (although deterministic) order.
  context.entries_to_process.back().lazy_component_with_args.component->addBindings(context.entries_to_process);
}

template <typename... Params>
FRUIT_ALWAYS_INLINE inline void BindingNormalization::handleLazyComponentWithNoArgs(
    BindingNormalizationContext<Params...>& context) {
  ComponentStorageEntry entry = context.entries_to_process.back();
  FruitAssert(entry.kind == ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS);
  if (context.fully_expanded_components_with_no_args.count(entry.lazy_component_with_no_args)) {
    // This lazy component was already inserted, skip it.
    context.entries_to_process.pop_back();
    return;
  }

  auto replacement_component_itr = context.component_with_no_args_replacements.find(entry.lazy_component_with_no_args);
  if (replacement_component_itr != context.component_with_no_args_replacements.end()) {
    performComponentReplacement(context, replacement_component_itr->second);
    return;
  }

  bool actually_inserted =
      context.components_with_no_args_with_expansion_in_progress.insert(entry.lazy_component_with_no_args).second;
  if (!actually_inserted) {
    printLazyComponentInstallationLoop(
        context.toplevel_component_fun_type_id, context.entries_to_process, entry);
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

#ifdef FRUIT_EXTRA_DEBUG
  std::cout << "Expanding lazy component: " << entry.type_id << std::endl;
#endif

  // Instead of removing the component from component.lazy_components, we just change its kind to the
  // corresponding *_END_MARKER kind.
  // When we pop this marker, this component's expansion will be complete.
  context.entries_to_process.back().kind = ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER;

  // Note that this can also add other lazy components, so the resulting bindings can have a non-intuitive
  // (although deterministic) order.
  context.entries_to_process.back().lazy_component_with_no_args.addBindings(context.entries_to_process);
}

template <
    typename FindNormalizedBinding,
    typename IsValidItr,
    typename IsNormalizedBindingItrForConstructedObject,
    typename GetObjectPtr,
    typename GetCreate>
void BindingNormalization::normalizeBindingsAndAddTo(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    TypeId toplevel_component_fun_type_id,
    MemoryPool& memory_pool,
    const FixedSizeAllocator::FixedSizeAllocatorData& base_fixed_size_allocator_data,
    const std::unordered_map<TypeId, NormalizedMultibindingSet>& base_multibindings,
    const NormalizedComponentStorage::BindingCompressionInfoMap& base_binding_compression_info_map,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& new_bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
    FindNormalizedBinding find_normalized_binding,
    IsValidItr is_valid_itr,
    IsNormalizedBindingItrForConstructedObject is_normalized_binding_itr_for_constructed_object,
    GetObjectPtr get_object_ptr,
    GetCreate get_create) {

  multibindings = base_multibindings;

  fixed_size_allocator_data = base_fixed_size_allocator_data;

  multibindings_vector_t multibindings_vector =
      multibindings_vector_t(ArenaAllocator<multibindings_vector_elem_t>(memory_pool));

  HashMapWithArenaAllocator<TypeId, ComponentStorageEntry> binding_data_map =
      createHashMapWithArenaAllocator<TypeId, ComponentStorageEntry>(memory_pool);

  normalizeBindings(
      std::move(toplevel_entries),
      fixed_size_allocator_data,
      toplevel_component_fun_type_id,
      memory_pool,
      binding_data_map,
      [](ComponentStorageEntry) {},
      [&multibindings_vector](ComponentStorageEntry multibinding,
                              ComponentStorageEntry multibinding_vector_creator) {
        multibindings_vector.emplace_back(multibinding, multibinding_vector_creator);
      },
      find_normalized_binding,
      is_valid_itr,
      is_normalized_binding_itr_for_constructed_object,
      get_object_ptr,
      get_create);

  // Copy the normalized bindings into the result vector.
  new_bindings_vector.clear();
  new_bindings_vector.reserve(binding_data_map.size());
  for (auto& p : binding_data_map) {
    new_bindings_vector.push_back(p.second);
  }

  // Determine what binding compressions must be undone.

  HashSetWithArenaAllocator<TypeId> binding_compressions_to_undo =
      createHashSetWithArenaAllocator<TypeId>(memory_pool);
  for (const ComponentStorageEntry& entry : new_bindings_vector) {
    switch (entry.kind) { // LCOV_EXCL_BR_LINE
    case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
      break;

    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION:
      {
        const BindingDeps *entry_deps = entry.binding_for_object_to_construct.deps;
        for (std::size_t i = 0; i < entry_deps->num_deps; ++i) {
          auto binding_compression_itr =
              base_binding_compression_info_map.find(entry_deps->deps[i]);
          if (binding_compression_itr != base_binding_compression_info_map.end()
              && binding_compression_itr->second.i_type_id != entry.type_id) {
            // The binding compression for `p.second.getDeps()->deps[i]' must be undone because something
            // different from binding_compression_itr->iTypeId is now bound to it.
            binding_compressions_to_undo.insert(entry_deps->deps[i]);
          }
        }
      }
      break;

    default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)entry.kind << std::endl;
#endif
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
      break;
    }
  }

  // Step 3: undo any binding compressions that can no longer be applied.
  for (TypeId cTypeId : binding_compressions_to_undo) {
    auto binding_compression_itr = base_binding_compression_info_map.find(cTypeId);
    FruitAssert(binding_compression_itr != base_binding_compression_info_map.end());
    FruitAssert(is_valid_itr(find_normalized_binding(binding_compression_itr->second.i_type_id)));

    ComponentStorageEntry c_binding;
    c_binding.type_id = cTypeId;
    c_binding.kind = ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION;
    c_binding.binding_for_object_to_construct = binding_compression_itr->second.c_binding;

    ComponentStorageEntry i_binding;
    i_binding.type_id = binding_compression_itr->second.i_type_id;
    i_binding.kind = ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION;
    i_binding.binding_for_object_to_construct = binding_compression_itr->second.i_binding;

    new_bindings_vector.push_back(std::move(c_binding));
    // This TypeId is already in normalized_component.bindings, we overwrite it here.
    new_bindings_vector.push_back(std::move(i_binding));

#ifdef FRUIT_EXTRA_DEBUG
    std::cout << "InjectorStorage: undoing binding compression for: " << binding_compression_itr->second.i_type_id << "->" << cTypeId << std::endl;
#endif
  }

  // Step 4: Add multibindings.
  BindingNormalization::addMultibindings(multibindings, fixed_size_allocator_data, multibindings_vector);
}

template <typename SaveCompressedBindingUndoInfo>
std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>> BindingNormalization::performBindingCompression(
    HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>&& binding_data_map,
    HashMapWithArenaAllocator<TypeId, BindingCompressionInfo>&& compressed_bindings_map,
    MemoryPool& memory_pool,
    const multibindings_vector_t& multibindings_vector,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    SaveCompressedBindingUndoInfo save_compressed_binding_undo_info) {
  using result_t = std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>;
  result_t result = result_t(ArenaAllocator<ComponentStorageEntry>(memory_pool));

  // We can't compress the binding if C is a dep of a multibinding.
  for (const std::pair<ComponentStorageEntry, ComponentStorageEntry>& multibinding_entry_pair : multibindings_vector) {
    const ComponentStorageEntry& entry = multibinding_entry_pair.first;
    FruitAssert(entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT
        || entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
        || entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
    if (entry.kind != ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT) {
      const BindingDeps* deps = entry.multibinding_for_object_to_construct.deps;
      FruitAssert(deps != nullptr);
      for (std::size_t i = 0; i < deps->num_deps; ++i) {
        compressed_bindings_map.erase(deps->deps[i]);
#ifdef FRUIT_EXTRA_DEBUG
        std::cout << "InjectorStorage: ignoring compressed binding for " << deps->deps[i] << " because it's a dep of a multibinding." << std::endl;
#endif
      }
    }
  }

  // We can't compress the binding if C is an exposed type (but I is likely to be exposed instead).
  for (TypeId type : exposed_types) {
    compressed_bindings_map.erase(type);
#ifdef FRUIT_EXTRA_DEBUG
    std::cout << "InjectorStorage: ignoring compressed binding for " << type << " because it's an exposed type." << std::endl;
#endif
  }

  // We can't compress the binding if some type X depends on C and X!=I.
  for (auto& binding_data_map_entry : binding_data_map) {
    TypeId x_id = binding_data_map_entry.first;
    ComponentStorageEntry entry = binding_data_map_entry.second;
    FruitAssert(entry.kind == ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT
        || entry.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
        || entry.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);

    if (entry.kind != ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT) {
      for (std::size_t i = 0; i < entry.binding_for_object_to_construct.deps->num_deps; ++i) {
        TypeId c_id = entry.binding_for_object_to_construct.deps->deps[i];
        auto itr = compressed_bindings_map.find(c_id);
        if (itr != compressed_bindings_map.end() && itr->second.i_type_id != x_id) {
          compressed_bindings_map.erase(itr);
#ifdef FRUIT_EXTRA_DEBUG
          std::cout << "InjectorStorage: ignoring compressed binding for " << c_id << " because the type " <<  x_id << " depends on it." << std::endl;
#endif
        }
      }
    }
  }

  // Two pairs of compressible bindings (I->C) and (C->X) can not exist (the C of a compressible binding is always bound either
  // using constructor binding or provider binding, it can't be a binding itself). So no need to check for that.

  // Now perform the binding compression.
  for (auto& entry : compressed_bindings_map) {
    TypeId c_id = entry.first;
    TypeId i_id = entry.second.i_type_id;
    auto i_binding_data = binding_data_map.find(i_id);
    auto c_binding_data = binding_data_map.find(c_id);
    FruitAssert(i_binding_data != binding_data_map.end());
    FruitAssert(c_binding_data != binding_data_map.end());
    NormalizedComponentStorage::CompressedBindingUndoInfo undo_info;
    undo_info.i_type_id = i_id;
    FruitAssert(i_binding_data->second.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
    undo_info.i_binding = i_binding_data->second.binding_for_object_to_construct;
    FruitAssert(
        c_binding_data->second.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION
        || c_binding_data->second.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION);
    undo_info.c_binding = c_binding_data->second.binding_for_object_to_construct;
    save_compressed_binding_undo_info(c_id, undo_info);

    // Note that even if I is the one that remains, C is the one that will be allocated, not I.

    i_binding_data->second.kind = c_binding_data->second.kind;
    i_binding_data->second.binding_for_object_to_construct.create = entry.second.create_i_with_compression;
    i_binding_data->second.binding_for_object_to_construct.deps =
        c_binding_data->second.binding_for_object_to_construct.deps;
    binding_data_map.erase(c_binding_data);
#ifdef FRUIT_EXTRA_DEBUG
    std::cout << "InjectorStorage: performing binding compression for the edge " << i_id << "->" << c_id << std::endl;
#endif
  }

  // Copy the normalized bindings into the result vector.
  result.reserve(binding_data_map.size());
  for (auto& p : binding_data_map) {
    result.push_back(p.second);
  }

  return result;
}

template <typename SaveCompressedBindingUndoInfo>
void BindingNormalization::normalizeBindingsWithBindingCompression(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    TypeId toplevel_component_fun_type_id,
    MemoryPool& memory_pool,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
    SaveCompressedBindingUndoInfo save_compressed_binding_undo_info) {

  HashMapWithArenaAllocator<TypeId, ComponentStorageEntry> binding_data_map =
      createHashMapWithArenaAllocator<TypeId, ComponentStorageEntry>(memory_pool);
  // CtypeId -> (ItypeId, bindingData)
  HashMapWithArenaAllocator<TypeId, BindingNormalization::BindingCompressionInfo> compressed_bindings_map =
      createHashMapWithArenaAllocator<TypeId, BindingCompressionInfo>(memory_pool);

  multibindings_vector_t multibindings_vector =
      multibindings_vector_t(ArenaAllocator<multibindings_vector_elem_t>(memory_pool));

  struct DummyIterator {};

  normalizeBindings(
      std::move(toplevel_entries),
      fixed_size_allocator_data,
      toplevel_component_fun_type_id,
      memory_pool,
      binding_data_map,
      [&compressed_bindings_map](ComponentStorageEntry entry) {
        BindingCompressionInfo& compression_info = compressed_bindings_map[entry.compressed_binding.c_type_id];
        compression_info.i_type_id = entry.type_id;
        compression_info.create_i_with_compression = entry.compressed_binding.create;
      },
      [&multibindings_vector](ComponentStorageEntry multibinding,
                              ComponentStorageEntry multibinding_vector_creator) {
        multibindings_vector.emplace_back(multibinding, multibinding_vector_creator);
      },
      [](TypeId) { return DummyIterator(); },
      [](DummyIterator) { return false; },
      [](DummyIterator) { return false; },
      [](DummyIterator) { return nullptr; },
      [](DummyIterator) { return nullptr; });

  bindings_vector =
      BindingNormalization::performBindingCompression(
          std::move(binding_data_map),
          std::move(compressed_bindings_map),
          memory_pool,
          multibindings_vector,
          exposed_types,
          save_compressed_binding_undo_info);

  addMultibindings(
      multibindings,
      fixed_size_allocator_data,
      multibindings_vector);
}

inline BindingNormalization::LazyComponentWithNoArgsSet BindingNormalization::createLazyComponentWithNoArgsSet(
    MemoryPool& memory_pool)  {
  return createHashSetWithArenaAllocatorAndCustomFunctors<LazyComponentWithNoArgs>(
      memory_pool,
      HashLazyComponentWithNoArgs(),
      std::equal_to<LazyComponentWithNoArgs>());
}

inline BindingNormalization::LazyComponentWithArgsSet BindingNormalization::createLazyComponentWithArgsSet(
    MemoryPool& memory_pool) {
  return createHashSetWithArenaAllocatorAndCustomFunctors<LazyComponentWithArgs>(
      memory_pool,
      HashLazyComponentWithArgs(),
      LazyComponentWithArgsEqualTo());
}

inline BindingNormalization::LazyComponentWithNoArgsReplacementMap
    BindingNormalization::createLazyComponentWithNoArgsReplacementMap(MemoryPool& memory_pool) {
  return createHashMapWithArenaAllocatorAndCustomFunctors<LazyComponentWithNoArgs, ComponentStorageEntry>(
      memory_pool,
      HashLazyComponentWithNoArgs(),
      std::equal_to<LazyComponentWithNoArgs>());
};

inline BindingNormalization::LazyComponentWithArgsReplacementMap
    BindingNormalization::createLazyComponentWithArgsReplacementMap(MemoryPool& memory_pool) {
  return createHashMapWithArenaAllocatorAndCustomFunctors<LazyComponentWithArgs, ComponentStorageEntry>(
      memory_pool,
      HashLazyComponentWithArgs(),
      LazyComponentWithArgsEqualTo());
};


} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDING_NORMALIZATION_TEMPLATES_H
