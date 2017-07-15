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

#include <fruit/impl/injector/injector_storage.h>
#include <fruit/impl/data_structures/semistatic_graph.templates.h>
#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.templates.h>

using std::cout;
using std::endl;

using namespace fruit::impl;

namespace fruit {
namespace impl {

void BindingNormalization::printLazyComponentInstallationLoop(
    TypeId toplevel_component_fun_type_id,
    const std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& entries_to_process,
    const ComponentStorageEntry& last_entry) {
  std::cerr << "Found a loop while expanding components passed to PartialComponent::install()." << std::endl;
  std::cerr << "Component installation trace (from top-level to the most deeply-nested):" << std::endl;
  std::cerr << std::string(toplevel_component_fun_type_id) << std::endl;
  for (const ComponentStorageEntry& entry : entries_to_process) {
    switch (entry.kind) {
    case ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER:
      if (entry.type_id == last_entry.type_id
          && last_entry.kind == ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS
          && *entry.lazy_component_with_args.component == *last_entry.lazy_component_with_args.component) {
        std::cerr << "<-- The loop starts here" << std::endl;
      }
      std::cerr << std::string(entry.lazy_component_with_args.component->getFunTypeId()) << std::endl;
      break;

    case ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER:
      if (entry.type_id == last_entry.type_id
          && last_entry.kind == ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS
          && entry.lazy_component_with_no_args.erased_fun == last_entry.lazy_component_with_no_args.erased_fun) {
        std::cerr << "<-- The loop starts here" << std::endl;
      }
      std::cerr << std::string(entry.type_id) << std::endl;
      break;

    default:
      break;
    }
  }

  switch (last_entry.kind) { // LCOV_EXCL_BR_LINE
  case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS:
    std::cerr << std::string(last_entry.lazy_component_with_args.component->getFunTypeId()) << std::endl;
    break;

  case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS:
    std::cerr << std::string(last_entry.type_id) << std::endl;
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  exit(1);
}

void BindingNormalization::printMultipleBindingsError(TypeId type) {
  std::cerr << "Fatal injection error: the type " << type.type_info->name()
            << " was provided more than once, with different bindings." << std::endl
            << "This was not caught at compile time because at least one of the involved components bound this type "
            << "but didn't expose it in the component signature." << std::endl
            << "If the type has a default constructor or an Inject annotation, this problem may arise even if this "
            << "type is bound/provided by only one component (and then hidden), if this type is auto-injected in "
            << "another component." << std::endl
            << "If the source of the problem is unclear, try exposing this type in all the component signatures where "
            << "it's bound; if no component hides it this can't happen." << std::endl;
  exit(1);
}

void BindingNormalization::printIncompatibleComponentReplacementsError(
    const ComponentStorageEntry& replaced_component_entry,
    const ComponentStorageEntry& replacement_component_entry1,
    const ComponentStorageEntry& replacement_component_entry2) {
  using fun_t = void(*)();

  fun_t replaced_fun_address;
  switch (replaced_component_entry.kind) {
  case ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS:
    replaced_fun_address = replaced_component_entry.lazy_component_with_args.component->erased_fun;
    break;

  case ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_NO_ARGS:
    replaced_fun_address = replaced_component_entry.lazy_component_with_no_args.erased_fun;
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  fun_t replacement_fun_address1;
  switch (replacement_component_entry1.kind) {
  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    replacement_fun_address1 = replacement_component_entry1.lazy_component_with_args.component->erased_fun;
    break;

  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
    replacement_fun_address1 = replacement_component_entry1.lazy_component_with_no_args.erased_fun;
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  fun_t replacement_fun_address2;
  switch (replacement_component_entry2.kind) {
  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    replacement_fun_address2 = replacement_component_entry2.lazy_component_with_args.component->erased_fun;
    break;

  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
    replacement_fun_address2 = replacement_component_entry2.lazy_component_with_no_args.erased_fun;
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  if (sizeof(void*) == sizeof(fun_t)) {
    std::cerr << "Fatal injection error: the component function at " << reinterpret_cast<void *>(replaced_fun_address)
              << " with signature " << std::string(replaced_component_entry.type_id)
              << " was replaced (using .replace(...).with(...)) with both the component function at "
              << reinterpret_cast<void *>(replacement_fun_address1) << " with signature "
              << std::string(replacement_component_entry1.type_id)
              << " and the component function at " << reinterpret_cast<void *>(replacement_fun_address2)
              << " with signature "
              << std::string(replacement_component_entry2.type_id) << " ." << std::endl;
  } else {
    std::cerr << "Fatal injection error: a component function with signature "
              << std::string(replaced_component_entry.type_id)
              << " was replaced (using .replace(...).with(...)) with both a component function with signature "
              << std::string(replacement_component_entry1.type_id)
              << " and another component function with signature "
              << std::string(replacement_component_entry2.type_id) << " ." << std::endl;
  }
  exit(1);
}

void BindingNormalization::printComponentReplacementFailedBecauseTargetAlreadyExpanded(
    const ComponentStorageEntry& replaced_component_entry,
    const ComponentStorageEntry& replacement_component_entry) {
  using fun_t = void(*)();

  fun_t replaced_fun_address;
  switch (replaced_component_entry.kind) {
  case ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS:
    replaced_fun_address = replaced_component_entry.lazy_component_with_args.component->erased_fun;
    break;

  case ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_NO_ARGS:
    replaced_fun_address = replaced_component_entry.lazy_component_with_no_args.erased_fun;
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  fun_t replacement_fun_address1;
  switch (replacement_component_entry.kind) {
  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    replacement_fun_address1 = replacement_component_entry.lazy_component_with_args.component->erased_fun;
    break;

  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
    replacement_fun_address1 = replacement_component_entry.lazy_component_with_no_args.erased_fun;
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }

  if (sizeof(void*) == sizeof(fun_t)) {
    std::cerr << "Fatal injection error: unable to replace (using .replace(...).with(...)) the component function at "
              << reinterpret_cast<void*>(replaced_fun_address)
              << " with signature " << std::string(replaced_component_entry.type_id)
              << " with the component function at "
              << reinterpret_cast<void*>(replacement_fun_address1) << " with signature "
              << std::string(replacement_component_entry.type_id)
              << " because the former component function was installed before the .replace(...).with(...)." << std::endl
              << "You should change the order of installation of subcomponents so that .replace(...).with(...) is "
              << "processed before the installation of the component to replace.";
  } else {
    std::cerr << "Fatal injection error: unable to replace (using .replace(...).with(...)) a component function with "
              << "signature " << std::string(replaced_component_entry.type_id)
              << " with a component function at with signature " << std::string(replacement_component_entry.type_id)
              << " because the former component function was installed before the .replace(...).with(...)." << std::endl
              << "You should change the order of installation of subcomponents so that .replace(...).with(...) is "
              << "processed before the installation of the component to replace.";
  }
  exit(1);

}

void BindingNormalization::addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingSet>&
                                                multibindings,
                                            FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                                            const multibindings_vector_t& multibindingsVector) {

#ifdef FRUIT_EXTRA_DEBUG
  std::cout << "InjectorStorage: adding multibindings:" << std::endl;
#endif
  // Now we must merge multiple bindings for the same type.
  for (auto i = multibindingsVector.begin(); i != multibindingsVector.end(); ++i) {
    const ComponentStorageEntry& multibinding_entry = i->first;
    const ComponentStorageEntry& multibinding_vector_creator_entry = i->second;
    FruitAssert(
        multibinding_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION
        || multibinding_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
        || multibinding_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT);
    FruitAssert(multibinding_vector_creator_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR);
    NormalizedMultibindingSet& b = multibindings[multibinding_entry.type_id];

    // Might be set already, but we need to set it if there was no multibinding for this type.
    b.get_multibindings_vector = multibinding_vector_creator_entry.multibinding_vector_creator.get_multibindings_vector;

    switch (i->first.kind) { // LCOV_EXCL_BR_LINE
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT:
      {
        NormalizedMultibinding normalized_multibinding;
        normalized_multibinding.is_constructed = true;
        normalized_multibinding.object = i->first.multibinding_for_constructed_object.object_ptr;
        b.elems.push_back(std::move(normalized_multibinding));
      }
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
      {
        fixed_size_allocator_data.addExternallyAllocatedType(i->first.type_id);
        NormalizedMultibinding normalized_multibinding;
        normalized_multibinding.is_constructed = false;
        normalized_multibinding.create = i->first.multibinding_for_object_to_construct.create;
        b.elems.push_back(std::move(normalized_multibinding));
      }
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
      {
        fixed_size_allocator_data.addType(i->first.type_id);
        NormalizedMultibinding normalized_multibinding;
        normalized_multibinding.is_constructed = false;
        normalized_multibinding.create = i->first.multibinding_for_object_to_construct.create;
        b.elems.push_back(std::move(normalized_multibinding));
      }
      break;

    default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)i->first.kind << std::endl;
#endif
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
  }
}

void BindingNormalization::normalizeBindingsWithUndoableBindingCompression(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    TypeId toplevel_component_fun_type_id,
    MemoryPool& memory_pool,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
    BindingCompressionInfoMap& bindingCompressionInfoMap) {

  FruitAssert(bindingCompressionInfoMap.empty());

  normalizeBindingsWithBindingCompression(
      std::move(toplevel_entries),
      fixed_size_allocator_data,
      toplevel_component_fun_type_id,
      memory_pool,
      exposed_types,
      bindings_vector,
      multibindings,
      [&bindingCompressionInfoMap](
          TypeId c_type_id,
          NormalizedComponentStorage::CompressedBindingUndoInfo undo_info) {
        bindingCompressionInfoMap[c_type_id] = undo_info;
      });
}

void BindingNormalization::normalizeBindingsWithPermanentBindingCompression(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    TypeId toplevel_component_fun_type_id,
    MemoryPool& memory_pool,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings) {
  normalizeBindingsWithBindingCompression(
      std::move(toplevel_entries),
      fixed_size_allocator_data,
      toplevel_component_fun_type_id,
      memory_pool,
      exposed_types,
      bindings_vector,
      multibindings,
      [](TypeId, NormalizedComponentStorage::CompressedBindingUndoInfo) {});
}

} // namespace impl
// We need a LCOV_EXCL_BR_LINE below because for some reason gcov/lcov think there's a branch there.
} // namespace fruit LCOV_EXCL_BR_LINE
