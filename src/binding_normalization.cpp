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

void BindingNormalization::normalizeBindings(
    FixedSizeVector<ComponentStorageEntry>&& toplevel_entries,
    FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
    TypeId toplevel_component_fun_type_id,
    MemoryPool& memory_pool,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>& bindings_vector,
    std::unordered_map<TypeId, NormalizedMultibindingSet>& multibindings,
    BindingCompressionInfoMap& bindingCompressionInfoMap) {

  HashMapWithArenaAllocator<TypeId, ComponentStorageEntry> binding_data_map =
      createHashMapWithArenaAllocator<TypeId, ComponentStorageEntry>(memory_pool);
  // CtypeId -> (ItypeId, bindingData)
  HashMapWithArenaAllocator<TypeId, BindingNormalization::BindingCompressionInfo> compressed_bindings_map =
      createHashMapWithArenaAllocator<TypeId, BindingCompressionInfo>(memory_pool);

  multibindings_vector_t multibindings_vector =
      multibindings_vector_t(ArenaAllocator<multibindings_vector_elem_t>(memory_pool));

  struct DummyIterator {};

  normalizeBindingsHelper(
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
          bindingCompressionInfoMap);

  addMultibindings(
      multibindings,
      fixed_size_allocator_data,
      multibindings_vector);
}

std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>> BindingNormalization::performBindingCompression(
    HashMapWithArenaAllocator<TypeId, ComponentStorageEntry>&& binding_data_map,
    HashMapWithArenaAllocator<TypeId, BindingCompressionInfo>&& compressed_bindings_map,
    MemoryPool& memory_pool,
    const multibindings_vector_t& multibindings_vector,
    const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    BindingCompressionInfoMap& bindingCompressionInfoMap) {
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
  result.reserve(binding_data_map.size());
  for (auto& p : binding_data_map) {
    result.push_back(p.second);
  }

  return result;
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

    switch (i->first.kind) {
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
      FruitAssert(false);
    }
  }
}

} // namespace impl
} // namespace fruit
