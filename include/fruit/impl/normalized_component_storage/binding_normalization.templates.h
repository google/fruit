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

using LazyComponentWithNoArgs = ComponentStorageEntry::LazyComponentWithNoArgs;
using LazyComponentWithArgs = ComponentStorageEntry::LazyComponentWithArgs;

namespace {

std::string multipleBindingsError(TypeId type) {
  return "Fatal injection error: the type " + type.type_info->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

auto createLazyComponentWithNoArgsSet = []() {
  return createHashSetWithCustomFunctors<LazyComponentWithNoArgs>(
      [](const LazyComponentWithNoArgs& x) {
        return x.hashCode();
      },
      [](const LazyComponentWithNoArgs& x, const LazyComponentWithNoArgs& y) {
        return x == y;
      });
};

auto createLazyComponentWithArgsSet = []() {
  return createHashSetWithCustomFunctors<LazyComponentWithArgs>(
      [](const LazyComponentWithArgs& x) {
        return x.component->hashCode();
      },
      [](const LazyComponentWithArgs& x, const LazyComponentWithArgs& y) {
        return *x.component == *y.component;
      });
};

} // namespace


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
void BindingNormalization::normalizeBindingsHelper(
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
    GetCreate get_create) {

  binding_data_map = createHashMap<TypeId, ComponentStorageEntry>();

  std::vector<ComponentStorageEntry> expanded_entries_vector;

  // These sets contain the lazy components whose expansion has already completed.
  auto fully_expanded_components_with_no_args = createLazyComponentWithNoArgsSet();
  auto fully_expanded_components_with_args = createLazyComponentWithArgsSet();

  // These sets contain the elements with kind *_END_MARKER in entries_to_process.
  // For component with args, these sets do *not* own the objects, entries_to_process does.
  auto components_with_no_args_with_expansion_in_progress = createLazyComponentWithNoArgsSet();
  auto components_with_args_with_expansion_in_progress = createLazyComponentWithArgsSet();

  std::vector<ComponentStorageEntry> entries_to_process(toplevel_entries.begin(), toplevel_entries.end());
  toplevel_entries.clear();

  // When we expand a lazy component, instead of removing it from the stack we change its kind (in entries_to_process)
  // to one of the *_END_MARKER kinds. This allows to keep track of the "call stack" for the expansion.

  while (!entries_to_process.empty()) {
    ComponentStorageEntry entry = entries_to_process.back();

    switch (entry.kind) {
    case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
      {
        entries_to_process.pop_back();

        auto itr = find_normalized_binding(entry.type_id);
        if (is_valid_itr(itr)) {
          if (!is_normalized_binding_itr_for_constructed_object(itr)
              || get_object_ptr(itr) != entry.binding_for_constructed_object.object_ptr) {
            std::cerr << multipleBindingsError(entry.type_id) << std::endl;
            exit(1);
          }
          // Otherwise ok, duplicate but consistent binding.
          break;
        }

        ComponentStorageEntry& entry_in_map = binding_data_map[entry.type_id];
        if (entry_in_map.type_id.type_info != nullptr) {
          if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT
              || entry.binding_for_constructed_object.object_ptr
                  != entry_in_map.binding_for_constructed_object.object_ptr) {
            std::cerr << multipleBindingsError(entry.type_id) << std::endl;
            exit(1);
          }
          // Otherwise ok, duplicate but consistent binding.
          break;
        }

        // New binding, add it to the map.
        entry_in_map = std::move(entry);
      }
      break;

    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
      {
        entries_to_process.pop_back();

        auto itr = find_normalized_binding(entry.type_id);
        if (is_valid_itr(itr)) {
          if (is_normalized_binding_itr_for_constructed_object(itr)
              || get_create(itr) != entry.binding_for_object_to_construct.create) {
            std::cerr << multipleBindingsError(entry.type_id) << std::endl;
            exit(1);
          }
          // Otherwise ok, duplicate but consistent binding.
          break;
        }

        ComponentStorageEntry& entry_in_map = binding_data_map[entry.type_id];
        fixed_size_allocator_data.addType(entry.type_id);
        if (entry_in_map.type_id.type_info != nullptr) {
          if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
              || entry.binding_for_object_to_construct.create
                  != entry_in_map.binding_for_object_to_construct.create) {
            std::cerr << multipleBindingsError(entry.type_id) << std::endl;
            exit(1);
          }
          // Otherwise ok, duplicate but consistent binding.
          break;
        }

        // New binding, add it to the map.
        entry_in_map = std::move(entry);
      }
      break;

    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
      {
        entries_to_process.pop_back();

        auto itr = find_normalized_binding(entry.type_id);
        if (is_valid_itr(itr)) {
          if (is_normalized_binding_itr_for_constructed_object(itr)
              || get_create(itr) != entry.binding_for_object_to_construct.create) {
            std::cerr << multipleBindingsError(entry.type_id) << std::endl;
            exit(1);
          }
          // Otherwise ok, duplicate but consistent binding.
          break;
        }

        ComponentStorageEntry& entry_in_map = binding_data_map[entry.type_id];
        fixed_size_allocator_data.addExternallyAllocatedType(entry.type_id);
        if (entry_in_map.type_id.type_info != nullptr) {
          if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION
              || entry.binding_for_object_to_construct.create
                  != entry_in_map.binding_for_object_to_construct.create) {
            std::cerr << multipleBindingsError(entry.type_id) << std::endl;
            exit(1);
          }
          // Otherwise ok, duplicate but consistent binding.
          break;
        }

        // New binding, add it to the map.
        entry_in_map = std::move(entry);
      }
      break;

    case ComponentStorageEntry::Kind::COMPRESSED_BINDING:
      {
        entries_to_process.pop_back();
        handle_compressed_binding(entry);
      }
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
      {
        entries_to_process.pop_back();
        FruitAssert(!entries_to_process.empty());
        ComponentStorageEntry vector_creator_entry = std::move(entries_to_process.back());
        entries_to_process.pop_back();
        FruitAssert(vector_creator_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR);
        handle_multibinding(entry, vector_creator_entry);
      }
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR:
      {
        entries_to_process.pop_back();
        FruitAssert(!entries_to_process.empty());
        ComponentStorageEntry multibinding_entry = std::move(entries_to_process.back());
        entries_to_process.pop_back();
        FruitAssert(
            multibinding_entry.kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT
                || multibinding_entry.kind
                    == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
                || multibinding_entry.kind
                    == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
        handle_multibinding(multibinding_entry, entry);
      }
      break;

    case ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER:
      {
        entries_to_process.pop_back();
        // A lazy component expansion has completed; we now move the component from
        // components_with_*_with_expansion_in_progress to fully_expanded_components_*.

        components_with_no_args_with_expansion_in_progress.erase(entry.lazy_component_with_no_args);
        fully_expanded_components_with_no_args.insert(std::move(entry.lazy_component_with_no_args));
      }
      break;

    case ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER:
      {
        entries_to_process.pop_back();
        // A lazy component expansion has completed; we now move the component from
        // components_with_*_with_expansion_in_progress to fully_expanded_components_*.

        components_with_args_with_expansion_in_progress.erase(entry.lazy_component_with_args);
        fully_expanded_components_with_args.insert(std::move(entry.lazy_component_with_args));
      }
      break;

    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS:
      {
        if (fully_expanded_components_with_args.count(entry.lazy_component_with_args)) {
          // This lazy component was already inserted, skip it.
          entries_to_process.pop_back();
          continue;
        }

        bool actually_inserted =
            components_with_args_with_expansion_in_progress.insert(entry.lazy_component_with_args).second;
        if (!actually_inserted) {
          printLazyComponentInstallationLoop(
              toplevel_component_fun_type_id, entries_to_process, entry);
          exit(1);
        }

#ifdef FRUIT_EXTRA_DEBUG
        std::cout << "Expanding lazy component: " << entry.lazy_component_with_args.component->getFunTypeId() << std::endl;
#endif

        // Instead of removing the component from component.lazy_components, we just change its kind to the
        // corresponding *_END_MARKER kind.
        // When we pop this marker, this component's expansion will be complete.
        entries_to_process.back().kind = ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER;

        // Note that this can also add other lazy components, so the resulting bindings can have a non-intuitive
        // (although deterministic) order.
        entries_to_process.back().lazy_component_with_args.component->addBindings(entries_to_process);

        break;
      }

    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS:
      {
        if (fully_expanded_components_with_no_args.count(entry.lazy_component_with_no_args)) {
          // This lazy component was already inserted, skip it.
          entries_to_process.pop_back();
          continue;
        }

        bool actually_inserted =
            components_with_no_args_with_expansion_in_progress.insert(entry.lazy_component_with_no_args).second;
        if (!actually_inserted) {
          printLazyComponentInstallationLoop(
              toplevel_component_fun_type_id, entries_to_process, entry);
          exit(1);
        }

    #ifdef FRUIT_EXTRA_DEBUG
        std::cout << "Expanding lazy component: " << entry.type_id << std::endl;
    #endif

        // Instead of removing the component from component.lazy_components, we just change its kind to the
        // corresponding *_END_MARKER kind.
        // When we pop this marker, this component's expansion will be complete.
        entries_to_process.back().kind = ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER;

        // Note that this can also add other lazy components, so the resulting bindings can have a non-intuitive
        // (although deterministic) order.
        entries_to_process.back().lazy_component_with_no_args.addBindings(entries_to_process);

        break;
      }

    default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)entries_to_process.back().kind << std::endl;
#endif
      FruitAssert(false);
    }
  }

  FruitAssert(components_with_no_args_with_expansion_in_progress.empty());
  FruitAssert(components_with_args_with_expansion_in_progress.empty());
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
    GetCreate get_create) {

  multibindings = base_multibindings;

  fixed_size_allocator_data = base_fixed_size_allocator_data;

  std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>> multibindings_vector;

  HashMap<TypeId, ComponentStorageEntry> binding_data_map;
  multibindings_vector.clear();

  normalizeBindingsHelper(
      std::move(toplevel_entries),
      fixed_size_allocator_data,
      toplevel_component_fun_type_id,
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

  HashSet<TypeId> binding_compressions_to_undo = createHashSet<TypeId>();
  for (const ComponentStorageEntry& entry : new_bindings_vector) {
    switch (entry.kind) {
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
      FruitAssert(false);
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
  BindingNormalization::addMultibindings(multibindings, fixed_size_allocator_data, std::move(multibindings_vector));
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDING_NORMALIZATION_TEMPLATES_H
