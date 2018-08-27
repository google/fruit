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

#include <fruit/impl/data_structures/semistatic_graph.templates.h>
#include <fruit/impl/injector/injector_storage.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.templates.h>
#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>

using std::cout;
using std::endl;

using namespace fruit::impl;

namespace fruit {
namespace impl {

void BindingNormalization::printLazyComponentInstallationLoop(
    const std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& entries_to_process,
    const ComponentStorageEntry& last_entry) {
  std::cerr << "Found a loop while expanding components passed to PartialComponent::install()." << std::endl;
  std::cerr << "Component installation trace (from top-level to the most deeply-nested):" << std::endl;
  for (const ComponentStorageEntry& entry : entries_to_process) {
    switch (entry.kind) {
    case ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER:
      if (entry.type_id == last_entry.type_id &&
          last_entry.kind == ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS &&
          *entry.lazy_component_with_args.component == *last_entry.lazy_component_with_args.component) {
        std::cerr << "<-- The loop starts here" << std::endl;
      }
      std::cerr << std::string(entry.lazy_component_with_args.component->getFunTypeId()) << std::endl;
      break;

    case ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER:
      if (entry.type_id == last_entry.type_id &&
          last_entry.kind == ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS &&
          entry.lazy_component_with_no_args.erased_fun == last_entry.lazy_component_with_no_args.erased_fun) {
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
    const ComponentStorageEntry& replaced_component_entry, const ComponentStorageEntry& replacement_component_entry1,
    const ComponentStorageEntry& replacement_component_entry2) {
  using fun_t = void (*)();

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

  constexpr static bool function_pointers_have_same_size = sizeof(void*) == sizeof(fun_t);
  if (function_pointers_have_same_size) {
    std::cerr << "Fatal injection error: the component function at " << reinterpret_cast<void*>(replaced_fun_address)
              << " with signature " << std::string(replaced_component_entry.type_id)
              << " was replaced (using .replace(...).with(...)) with both the component function at "
              << reinterpret_cast<void*>(replacement_fun_address1) << " with signature "
              << std::string(replacement_component_entry1.type_id) << " and the component function at "
              << reinterpret_cast<void*>(replacement_fun_address2) << " with signature "
              << std::string(replacement_component_entry2.type_id) << " ." << std::endl;
  } else {
    std::cerr << "Fatal injection error: a component function with signature "
              << std::string(replaced_component_entry.type_id)
              << " was replaced (using .replace(...).with(...)) with both a component function with signature "
              << std::string(replacement_component_entry1.type_id) << " and another component function with signature "
              << std::string(replacement_component_entry2.type_id) << " ." << std::endl;
  }
  exit(1);
}

void BindingNormalization::printComponentReplacementFailedBecauseTargetAlreadyExpanded(
    const ComponentStorageEntry& replaced_component_entry, const ComponentStorageEntry& replacement_component_entry) {
  using fun_t = void (*)();

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

  constexpr static bool function_pointers_have_same_size = sizeof(void*) == sizeof(fun_t);
  if (function_pointers_have_same_size) {
    std::cerr << "Fatal injection error: unable to replace (using .replace(...).with(...)) the component function at "
              << reinterpret_cast<void*>(replaced_fun_address) << " with signature "
              << std::string(replaced_component_entry.type_id) << " with the component function at "
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

void BindingNormalization::addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
                                            FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                                            const multibindings_vector_t& multibindingsVector) {

#if FRUIT_EXTRA_DEBUG
  std::cout << "InjectorStorage: adding multibindings:" << std::endl;
#endif
  // Now we must merge multiple bindings for the same type.
  for (auto i = multibindingsVector.begin(); i != multibindingsVector.end(); ++i) {
    const ComponentStorageEntry& multibinding_entry = i->first;
    const ComponentStorageEntry& multibinding_vector_creator_entry = i->second;
    FruitAssert(multibinding_entry.kind ==
                    ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION ||
                multibinding_entry.kind ==
                    ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION ||
                multibinding_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT);
    FruitAssert(multibinding_vector_creator_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR);
    NormalizedMultibindingSet& b = multibindings[multibinding_entry.type_id];

    // Might be set already, but we need to set it if there was no multibinding for this type.
    b.get_multibindings_vector = multibinding_vector_creator_entry.multibinding_vector_creator.get_multibindings_vector;

    switch (i->first.kind) { // LCOV_EXCL_BR_LINE
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT: {
      NormalizedMultibinding normalized_multibinding;
      normalized_multibinding.is_constructed = true;
      normalized_multibinding.object = i->first.multibinding_for_constructed_object.object_ptr;
      b.elems.push_back(std::move(normalized_multibinding));
    } break;

    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION: {
      fixed_size_allocator_data.addExternallyAllocatedType(i->first.type_id);
      NormalizedMultibinding normalized_multibinding;
      normalized_multibinding.is_constructed = false;
      normalized_multibinding.create = i->first.multibinding_for_object_to_construct.create;
      b.elems.push_back(std::move(normalized_multibinding));
    } break;

    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION: {
      fixed_size_allocator_data.addType(i->first.type_id);
      NormalizedMultibinding normalized_multibinding;
      normalized_multibinding.is_constructed = false;
      normalized_multibinding.create = i->first.multibinding_for_object_to_construct.create;
      b.elems.push_back(std::move(normalized_multibinding));
    } break;

    default:
#if FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)i->first.kind << std::endl;
#endif
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }
  }
}

void BindingNormalization::normalizeBindingsWithUndoableBindingCompression(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data, MemoryPool& memory_pool,
    MemoryPool& memory_pool_for_fully_expanded_components_maps, MemoryPool& memory_pool_for_component_replacements_maps,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
    BindingCompressionInfoMap& bindingCompressionInfoMap,
    LazyComponentWithNoArgsSet& fully_expanded_components_with_no_args,
    LazyComponentWithArgsSet& fully_expanded_components_with_args,
    LazyComponentWithNoArgsReplacementMap& component_with_no_args_replacements,
    LazyComponentWithArgsReplacementMap& component_with_args_replacements) {

  FruitAssert(bindingCompressionInfoMap.empty());

  normalizeBindingsWithBindingCompression(
      std::move(toplevel_entries), fixed_size_allocator_data, memory_pool,
      memory_pool_for_fully_expanded_components_maps, memory_pool_for_component_replacements_maps, exposed_types,
      bindings_vector, multibindings,
      [&bindingCompressionInfoMap](TypeId c_type_id, NormalizedComponentStorage::CompressedBindingUndoInfo undo_info) {
        bindingCompressionInfoMap[c_type_id] = undo_info;
      },
      [&fully_expanded_components_with_no_args](LazyComponentWithNoArgsSet& fully_expanded_components) {
        fully_expanded_components_with_no_args = std::move(fully_expanded_components);
        fully_expanded_components.clear();
      },
      [&fully_expanded_components_with_args](LazyComponentWithArgsSet& fully_expanded_components) {
        fully_expanded_components_with_args = std::move(fully_expanded_components);
        fully_expanded_components.clear();
      },
      [&component_with_no_args_replacements](LazyComponentWithNoArgsReplacementMap& component_replacements) {
        component_with_no_args_replacements = std::move(component_replacements);
        component_replacements.clear();
      },
      [&component_with_args_replacements](LazyComponentWithArgsReplacementMap& component_replacements) {
        component_with_args_replacements = std::move(component_replacements);
        component_replacements.clear();
      });
}

void BindingNormalization::normalizeBindingsWithPermanentBindingCompression(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data, MemoryPool& memory_pool,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings) {
  normalizeBindingsWithBindingCompression(
      std::move(toplevel_entries), fixed_size_allocator_data, memory_pool, memory_pool, memory_pool, exposed_types,
      bindings_vector, multibindings, [](TypeId, NormalizedComponentStorage::CompressedBindingUndoInfo) {},
      [](LazyComponentWithNoArgsSet&) {}, [](LazyComponentWithArgsSet&) {},
      [](LazyComponentWithNoArgsReplacementMap&) {}, [](LazyComponentWithArgsReplacementMap&) {});
}

void BindingNormalization::normalizeBindingsAndAddTo(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries, MemoryPool& memory_pool,
    const NormalizedComponentStorage& base_normalized_component,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& new_bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings) {

  multibindings = base_normalized_component.multibindings;

  fixed_size_allocator_data = base_normalized_component.fixed_size_allocator_data;

  multibindings_vector_t multibindings_vector =
      multibindings_vector_t(ArenaAllocator<multibindings_vector_elem_t>(memory_pool));

  HashMapWithArenaAllocator<TypeId, ComponentStorageEntry> binding_data_map =
      createHashMapWithArenaAllocator<TypeId, ComponentStorageEntry>(20 /* capacity */, memory_pool);

  using Graph = NormalizedComponentStorage::Graph;

  normalizeBindings(
      std::move(toplevel_entries), fixed_size_allocator_data, memory_pool, memory_pool, memory_pool, binding_data_map,
      [](ComponentStorageEntry) {},
      [&multibindings_vector](ComponentStorageEntry multibinding, ComponentStorageEntry multibinding_vector_creator) {
        multibindings_vector.emplace_back(multibinding, multibinding_vector_creator);
      },
      [&base_normalized_component](TypeId type_id) { return base_normalized_component.bindings.find(type_id); },
      [&base_normalized_component](Graph::const_node_iterator itr) {
        return !(itr == base_normalized_component.bindings.end());
      },
      [](Graph::const_node_iterator itr) { return itr.isTerminal(); },
      [](Graph::const_node_iterator itr) { return itr.getNode().object; },
      [](Graph::const_node_iterator itr) { return itr.getNode().create; },
      [&base_normalized_component](const LazyComponentWithNoArgs& lazy_component) {
        return base_normalized_component.fully_expanded_components_with_no_args.count(lazy_component) != 0;
      },
      [&base_normalized_component](const LazyComponentWithArgs& lazy_component) {
        return base_normalized_component.fully_expanded_components_with_args.count(lazy_component) != 0;
      },
      [](LazyComponentWithNoArgsSet&) {}, [](LazyComponentWithArgsSet&) {},
      [&base_normalized_component](const LazyComponentWithNoArgs& lazy_component) {
        return base_normalized_component.component_with_no_args_replacements.find(lazy_component);
      },
      [&base_normalized_component](const LazyComponentWithArgs& lazy_component) {
        return base_normalized_component.component_with_args_replacements.find(lazy_component);
      },
      [&base_normalized_component](typename LazyComponentWithNoArgsReplacementMap::const_iterator itr) {
        return itr != base_normalized_component.component_with_no_args_replacements.end();
      },
      [&base_normalized_component](typename LazyComponentWithArgsReplacementMap::const_iterator itr) {
        return itr != base_normalized_component.component_with_args_replacements.end();
      },
      [](typename LazyComponentWithNoArgsReplacementMap::const_iterator itr) { return itr->second; },
      [](typename LazyComponentWithArgsReplacementMap::const_iterator itr) { return itr->second; },
      [](LazyComponentWithNoArgsReplacementMap&) {}, [](LazyComponentWithArgsReplacementMap&) {});

  // Copy the normalized bindings into the result vector.
  new_bindings_vector.clear();
  new_bindings_vector.reserve(binding_data_map.size());
  for (auto& p : binding_data_map) {
    new_bindings_vector.push_back(p.second);
  }

  // Determine what binding compressions must be undone.

  HashSetWithArenaAllocator<TypeId> binding_compressions_to_undo =
      createHashSetWithArenaAllocator<TypeId>(20 /* capacity */, memory_pool);
  for (const ComponentStorageEntry& entry : new_bindings_vector) {
    switch (entry.kind) { // LCOV_EXCL_BR_LINE
    case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
      break;

    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION: {
      const BindingDeps* entry_deps = entry.binding_for_object_to_construct.deps;
      for (std::size_t i = 0; i < entry_deps->num_deps; ++i) {
        auto binding_compression_itr = base_normalized_component.binding_compression_info_map.find(entry_deps->deps[i]);
        if (binding_compression_itr != base_normalized_component.binding_compression_info_map.end() &&
            binding_compression_itr->second.i_type_id != entry.type_id) {
          // The binding compression for `p.second.getDeps()->deps[i]' must be undone because something
          // different from binding_compression_itr->iTypeId is now bound to it.
          binding_compressions_to_undo.insert(entry_deps->deps[i]);
        }
      }
    } break;

    default:
#if FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)entry.kind << std::endl;
#endif
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
      break;
    }
  }

  // Step 3: undo any binding compressions that can no longer be applied.
  for (TypeId cTypeId : binding_compressions_to_undo) {
    auto binding_compression_itr = base_normalized_component.binding_compression_info_map.find(cTypeId);
    FruitAssert(binding_compression_itr != base_normalized_component.binding_compression_info_map.end());
    FruitAssert(!(base_normalized_component.bindings.find(binding_compression_itr->second.i_type_id) ==
                  base_normalized_component.bindings.end()));

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

#if FRUIT_EXTRA_DEBUG
    std::cout << "InjectorStorage: undoing binding compression for: " << binding_compression_itr->second.i_type_id
              << "->" << cTypeId << std::endl;
#endif
  }

  // Step 4: Add multibindings.
  BindingNormalization::addMultibindings(multibindings, fixed_size_allocator_data, multibindings_vector);
}

void BindingNormalization::handlePreexistingLazyComponentWithArgsReplacement(
    ComponentStorageEntry& replaced_component_entry, const ComponentStorageEntry& preexisting_replacement,
    ComponentStorageEntry& new_replacement) {
  switch (new_replacement.kind) { // LCOV_EXCL_BR_LINE
  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
    if (preexisting_replacement.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS ||
        preexisting_replacement.lazy_component_with_no_args.erased_fun !=
            new_replacement.lazy_component_with_no_args.erased_fun) {
      printIncompatibleComponentReplacementsError(replaced_component_entry, new_replacement, preexisting_replacement);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }

    // Duplicate but consistent replacement, we'll ignore it.
    replaced_component_entry.lazy_component_with_args.destroy();
    break;

  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    if (preexisting_replacement.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS ||
        !(*preexisting_replacement.lazy_component_with_args.component ==
          *new_replacement.lazy_component_with_args.component)) {
      printIncompatibleComponentReplacementsError(replaced_component_entry, new_replacement, preexisting_replacement);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }

    // Duplicate but consistent replacement, we'll ignore it.
    replaced_component_entry.lazy_component_with_args.destroy();
    new_replacement.lazy_component_with_args.destroy();
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }
}

void BindingNormalization::handlePreexistingLazyComponentWithNoArgsReplacement(
    ComponentStorageEntry& replaced_component_entry, const ComponentStorageEntry& preexisting_replacement,
    ComponentStorageEntry& new_replacement) {
  switch (new_replacement.kind) { // LCOV_EXCL_BR_LINE
  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS:
    if (preexisting_replacement.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS ||
        preexisting_replacement.lazy_component_with_no_args.erased_fun !=
            new_replacement.lazy_component_with_no_args.erased_fun) {
      printIncompatibleComponentReplacementsError(replaced_component_entry, new_replacement, preexisting_replacement);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }

    // Duplicate but consistent replacement, we'll ignore it.
    break;

  case ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    if (new_replacement.kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS ||
        !(*preexisting_replacement.lazy_component_with_args.component ==
          *new_replacement.lazy_component_with_args.component)) {
      printIncompatibleComponentReplacementsError(replaced_component_entry, new_replacement, preexisting_replacement);
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }

    // Duplicate but consistent replacement, we'll ignore it.
    new_replacement.lazy_component_with_args.destroy();
    break;

  default:
    FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
  }
}

} // namespace impl
// We need a LCOV_EXCL_BR_LINE below because for some reason gcov/lcov think there's a branch there.
} // namespace fruit LCOV_EXCL_BR_LINE
