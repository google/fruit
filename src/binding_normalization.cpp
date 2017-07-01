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
#include <fruit/impl/component_storage/component_storage.h>
#include <fruit/impl/data_structures/semistatic_graph.templates.h>
#include <fruit/impl/meta/basics.h>
#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>
#include <fruit/impl/normalized_component_storage/binding_normalization.h>

using std::cout;
using std::endl;

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

void printLazyComponentInstallationLoop(TypeId toplevel_component_fun_type_id,
                                        const std::vector<ComponentStorageEntry>& entries_to_process,
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

  switch (last_entry.kind) {
  case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS:
    std::cerr << std::string(last_entry.lazy_component_with_args.component->getFunTypeId()) << std::endl;
    break;

  case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS:
    std::cerr << std::string(last_entry.type_id) << std::endl;
    break;

  default:
    break;
  }
}

struct BindingCompressionInfo {
  TypeId i_type_id;
  ComponentStorageEntry::BindingForObjectToConstruct::create_t create_i_with_compression;
};

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

void BindingNormalization::split_component_storage_entries(
    std::vector<ComponentStorageEntry>&& all_entries_vector,
    std::vector<ComponentStorageEntry>& bindings_vector,
    std::vector<ComponentStorageEntry>& compressed_bindings_vector,
    std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>& multibindings_vector) {
  bindings_vector.clear();
  compressed_bindings_vector.clear();
  multibindings_vector.clear();
  for (auto itr = all_entries_vector.begin(), itr_end = all_entries_vector.end(); itr != itr_end; ++itr) {
    switch (itr->kind) {
    case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
      bindings_vector.push_back(std::move(*itr));
      break;

    case ComponentStorageEntry::Kind::COMPRESSED_BINDING:
      compressed_bindings_vector.push_back(std::move(*itr));
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
      FruitAssert(itr + 1 != itr_end);
      FruitAssert((itr + 1)->kind == ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR);
      multibindings_vector.emplace_back(std::move(*itr), std::move(*(itr + 1)));
      ++itr;
      break;

    case ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR:
      FruitAssert(itr + 1 != itr_end);
      FruitAssert(
          (itr + 1)->kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT
          || (itr + 1)->kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
          || (itr + 1)->kind == ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
      multibindings_vector.emplace_back(std::move(*(itr + 1)), std::move(*itr));
      ++itr;
      break;

    // At this point these should not exist.
    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS:
    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS:
    case ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER:
    case ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER:

    default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)itr->kind << std::endl;
#endif
      FruitAssert(false);
    }
  }
  all_entries_vector.clear();
}

void BindingNormalization::normalizeBindings(
    std::vector<ComponentStorageEntry>& bindings_vector,
    std::vector<ComponentStorageEntry>&& compressed_bindings_vector,
    const std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>& multibindings_vector,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    const std::vector<TypeId>& exposed_types,
    BindingCompressionInfoMap& bindingCompressionInfoMap) {
  HashMap<TypeId, ComponentStorageEntry> binding_data_map =
      createHashMap<TypeId, ComponentStorageEntry>(bindings_vector.size());

  for (auto& binding_entry : bindings_vector) {
    ComponentStorageEntry& entry_in_map = binding_data_map[binding_entry.type_id];
    switch (binding_entry.kind) {
      case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
        {
          if (entry_in_map.type_id.type_info != nullptr) {
            if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT
                || binding_entry.binding_for_constructed_object.object_ptr
                    != entry_in_map.binding_for_constructed_object.object_ptr) {
              std::cerr << multipleBindingsError(binding_entry.type_id) << std::endl;
              exit(1);
            }
            // Otherwise ok, duplicate but consistent binding.
          } else {
            // New binding, add it to the map.
            entry_in_map = std::move(binding_entry);
          }
        }
        break;
      case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
        {
          fixed_size_allocator_data.addType(binding_entry.type_id);
          if (entry_in_map.type_id.type_info != nullptr) {
            if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
                || binding_entry.binding_for_object_to_construct.create
                    != entry_in_map.binding_for_object_to_construct.create) {
              std::cerr << multipleBindingsError(binding_entry.type_id) << std::endl;
              exit(1);
            }
            // Otherwise ok, duplicate but consistent binding.
          } else {
            // New binding, add it to the map.
            entry_in_map = std::move(binding_entry);
          }
        }
        break;

      case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
        {
          fixed_size_allocator_data.addExternallyAllocatedType(binding_entry.type_id);
          if (entry_in_map.type_id.type_info != nullptr) {
            if (entry_in_map.kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION
                || binding_entry.binding_for_object_to_construct.create
                    != entry_in_map.binding_for_object_to_construct.create) {
              std::cerr << multipleBindingsError(binding_entry.type_id) << std::endl;
              exit(1);
            }
            // Otherwise ok, duplicate but consistent binding.
          } else {
            // New binding, add it to the map.
            entry_in_map = std::move(binding_entry);
          }
        }
        break;

      default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)binding_entry.kind << std::endl;
#endif
        FruitAssert(false);
    }
  }

  // Remove duplicates from `compressedBindingsVector'.

  // CtypeId -> (ItypeId, bindingData)
  HashMap<TypeId, BindingCompressionInfo> compressed_bindings_map =
      createHashMap<TypeId, BindingCompressionInfo>(compressed_bindings_vector.size());

  // This also removes any duplicates. No need to check for multiple I->C, I2->C mappings, will filter these out later when
  // considering deps.
  for (ComponentStorageEntry& compressed_binding : compressed_bindings_vector) {
    FruitAssert(compressed_binding.kind == ComponentStorageEntry::Kind::COMPRESSED_BINDING);
    BindingCompressionInfo& compression_info = compressed_bindings_map[compressed_binding.compressed_binding.c_type_id];
    compression_info.i_type_id = compressed_binding.type_id;
    compression_info.create_i_with_compression = compressed_binding.compressed_binding.create;
  }

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

  bindingCompressionInfoMap =
      createHashMap<TypeId, NormalizedComponentStorage::CompressedBindingUndoInfo>(compressed_bindings_map.size());

  // Now perform the binding compression.
  for (auto& entry : compressed_bindings_map) {
    TypeId c_id = entry.first;
    TypeId i_id = entry.second.i_type_id;
    auto i_binding_data = binding_data_map.find(i_id);
    auto c_binding_data = binding_data_map.find(c_id);
    FruitAssert(i_binding_data != binding_data_map.end());
    FruitAssert(c_binding_data != binding_data_map.end());
    NormalizedComponentStorage::CompressedBindingUndoInfo& undo_info = bindingCompressionInfoMap[c_id];
    undo_info.i_type_id = i_id;
    FruitAssert(i_binding_data->second.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION);
    undo_info.i_binding = i_binding_data->second.binding_for_object_to_construct;
    FruitAssert(
        c_binding_data->second.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION
        || c_binding_data->second.kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION);
    undo_info.c_binding = c_binding_data->second.binding_for_object_to_construct;
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
  bindings_vector.clear();
  bindings_vector.reserve(binding_data_map.size());
  for (auto& p : binding_data_map) {
    bindings_vector.push_back(p.second);
  }
}

void BindingNormalization::addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingSet>&
                                                multibindings,
                                            FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                                            std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>>&& multibindingsVector) {

  std::sort(multibindingsVector.begin(), multibindingsVector.end(),
            [](const std::pair<ComponentStorageEntry, ComponentStorageEntry>& binding1,
               const std::pair<ComponentStorageEntry, ComponentStorageEntry>& binding2) {
              return binding1.first.type_id < binding2.first.type_id;
            });

#ifdef FRUIT_EXTRA_DEBUG
  std::cout << "InjectorStorage: adding multibindings:" << std::endl;
#endif
  // Now we must merge multiple bindings for the same type.
  for (auto i = multibindingsVector.begin(); i != multibindingsVector.end(); /* no increment */) {
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

#ifdef FRUIT_EXTRA_DEBUG
    std::cout << "Multibindings for " << multibinding_entry.type_id << " :" << std::endl;
#endif
    // Insert all multibindings for this type (note that multibinding_entry is also inserted here).
    for (; i != multibindingsVector.end() && i->first.type_id == multibinding_entry.type_id; ++i) {
      switch (i->first.kind) {
      case ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT:
        {
          NormalizedMultibinding normalized_multibinding;
          normalized_multibinding.is_constructed = true;
          normalized_multibinding.object = i->first.multibinding_for_constructed_object.object_ptr;
          b.elems.push_back(std::move(normalized_multibinding));
#ifdef FRUIT_EXTRA_DEBUG
          std::cout << "MULTIBINDING_FOR_CONSTRUCTED_OBJECT binding for object " << normalized_multibinding.object << std::endl;
#endif
        }
        break;

      case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
        {
          fixed_size_allocator_data.addExternallyAllocatedType(i->first.type_id);
          NormalizedMultibinding normalized_multibinding;
          normalized_multibinding.is_constructed = false;
          normalized_multibinding.create = i->first.multibinding_for_object_to_construct.create;
          b.elems.push_back(std::move(normalized_multibinding));
#ifdef FRUIT_EXTRA_DEBUG
          std::cout << "MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION binding with create = " << normalized_multibinding.create << std::endl;
#endif
        }
        break;

      case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
        {
          fixed_size_allocator_data.addType(i->first.type_id);
          NormalizedMultibinding normalized_multibinding;
          normalized_multibinding.is_constructed = false;
          normalized_multibinding.create = i->first.multibinding_for_object_to_construct.create;
          b.elems.push_back(std::move(normalized_multibinding));
#ifdef FRUIT_EXTRA_DEBUG
          std::cout << "MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION binding with create = " << normalized_multibinding.create << std::endl;
#endif
        }
        break;

      default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)i->first.kind << std::endl;
#endif
        FruitAssert(false);
      }
    }
#ifdef FRUIT_EXTRA_DEBUG
    std::cout << std::endl;
#endif
  }
}

void BindingNormalization::expandLazyComponents(
    std::vector<ComponentStorageEntry>& entries, TypeId toplevel_component_fun_type_id) {
  // These sets contain the lazy components whose expansion has already completed.
  auto fully_expanded_components_with_no_args = createLazyComponentWithNoArgsSet();
  auto fully_expanded_components_with_args = createLazyComponentWithArgsSet();

  // These sets contain the same elements as components_expansion_stack_*.
  // For component with args, these sets do *not* own the objects, components_expansion_stack_* does.
  auto components_with_no_args_with_expansion_in_progress = createLazyComponentWithNoArgsSet();
  auto components_with_args_with_expansion_in_progress = createLazyComponentWithArgsSet();

  std::vector<ComponentStorageEntry> entries_to_process = std::move(entries);
  // We'll use this to contain the results.
  entries.clear();

  // When we expand a lazy component, instead of removing it from the stack we change its kind (in entries_to_process)
  // to one of the *_END_MARKER kinds. This allows to keep track of the "call stack" for the expansion.

  while (!entries_to_process.empty()) {
    switch (entries_to_process.back().kind) {
    case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
    case ComponentStorageEntry::Kind::COMPRESSED_BINDING:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
    case ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
    case ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR:
      entries.push_back(std::move(entries_to_process.back()));
      entries_to_process.pop_back();
      break;

    case ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER:
      {
        ComponentStorageEntry entry = std::move(entries_to_process.back());
        entries_to_process.pop_back();
        // A lazy component expansion has completed; we now move the component from
        // components_with_*_with_expansion_in_progress to fully_expanded_components_*.

        components_with_no_args_with_expansion_in_progress.erase(entry.lazy_component_with_no_args);
        fully_expanded_components_with_no_args.insert(std::move(entry.lazy_component_with_no_args));

        break;
      }
    case ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER:
      {
        ComponentStorageEntry entry = std::move(entries_to_process.back());
        entries_to_process.pop_back();
        // A lazy component expansion has completed; we now move the component from
        // components_with_*_with_expansion_in_progress to fully_expanded_components_*.

        components_with_args_with_expansion_in_progress.erase(entry.lazy_component_with_args);
        fully_expanded_components_with_args.insert(std::move(entry.lazy_component_with_args));

        break;
      }

    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS:
      {
        ComponentStorageEntry& entry = entries_to_process.back();

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
        entry.kind = ComponentStorageEntry::Kind::COMPONENT_WITH_ARGS_END_MARKER;

        // Note that this can also add other lazy components, so the resulting bindings can have a non-intuitive
        // (although deterministic) order.
        entry.lazy_component_with_args.component->addBindings(entries_to_process);

        // The addBindings call might have resized the `entries` vector, so the `entry`
        // reference may no longer be valid here. But it's fine, we don't need it anymore.

        break;
      }

    case ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS:
      {
        ComponentStorageEntry& entry = entries_to_process.back();

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
        entry.kind = ComponentStorageEntry::Kind::COMPONENT_WITHOUT_ARGS_END_MARKER;

        // Note that this can also add other lazy components, so the resulting bindings can have a non-intuitive
        // (although deterministic) order.
        entry.lazy_component_with_no_args.addBindings(entries_to_process);

        // The addBindings call might have resized the `entries` vector, so the `entry`
        // reference may no longer be valid here. But it's fine, we don't need it anymore.

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

} // namespace impl
} // namespace fruit
