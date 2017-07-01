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

namespace {

std::string multipleBindingsError(TypeId type) {
  return "Fatal injection error: the type " + type.type_info->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

} // namespace

namespace fruit {
namespace impl {

void InjectorStorage::fatal(const std::string& error) {
  std::cerr << "Fatal injection error: " << error << std::endl;
  exit(1);
}

namespace {
  template <typename Id, typename Value>
  struct DummyNode {
    Id getId() {
      return Id();
    }
    bool isTerminal() {
      return false;
    }
    Id* getEdgesBegin() {
      return nullptr;
    }
    Id* getEdgesEnd() {
      return nullptr;
    }
    Value getValue() {
      return Value();
    }
  };
}

InjectorStorage::InjectorStorage(
    ComponentStorage&& component,
    const std::vector<TypeId>& exposed_types,
    TypeId toplevel_component_fun_type_id)
  : normalized_component_storage_ptr(
      new NormalizedComponentStorage(
          std::move(component), exposed_types, toplevel_component_fun_type_id)),
    allocator(normalized_component_storage_ptr->fixed_size_allocator_data),
    bindings(normalized_component_storage_ptr->bindings,
             (DummyNode<TypeId, NormalizedBinding>*)nullptr,
             (DummyNode<TypeId, NormalizedBinding>*)nullptr),
    multibindings(std::move(normalized_component_storage_ptr->multibindings)) {

#ifdef FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

InjectorStorage::InjectorStorage(const NormalizedComponentStorage& normalized_component,
                                 ComponentStorage&& component,
                                 std::vector<TypeId>&& exposed_types,
                                 TypeId toplevel_component_fun_type_id)
  : multibindings(normalized_component.multibindings) {

  FixedSizeAllocator::FixedSizeAllocatorData fixed_size_allocator_data = normalized_component.fixed_size_allocator_data;

  BindingNormalization::expandLazyComponents(component.entries, toplevel_component_fun_type_id);

  std::vector<ComponentStorageEntry> bindings_vector;
  std::vector<ComponentStorageEntry> compressed_bindings_vector;
  std::vector<std::pair<ComponentStorageEntry, ComponentStorageEntry>> multibindings_vector;
  BindingNormalization::split_component_storage_entries(
      std::move(component.entries),
      bindings_vector,
      compressed_bindings_vector,
      multibindings_vector);

  // Step 1: Remove duplicates among the new bindings, and check for inconsistent bindings within `component' alone.
  // Note that we do NOT use component.compressed_bindings here, to avoid having to check if these compressions can be undone.
  // We don't expect many binding compressions here that weren't already performed in the normalized component.
  BindingNormalization::BindingCompressionInfoMap bindingCompressionInfoMapUnused;
  BindingNormalization::normalizeBindings(bindings_vector,
                                          std::vector<ComponentStorageEntry>{},
                                          multibindings_vector,
                                          fixed_size_allocator_data,
                                          std::move(exposed_types),
                                          bindingCompressionInfoMapUnused);
  FruitAssert(bindingCompressionInfoMapUnused.empty());

  HashSet<TypeId> binding_compressions_to_undo = createHashSet<TypeId>();

  // Step 2: Filter out already-present bindings, and check for inconsistent bindings between `normalizedComponent' and
  // `component'. Also determine what binding compressions must be undone
  auto itr = std::remove_if(bindings_vector.begin(), bindings_vector.end(),
                            [&normalized_component, &binding_compressions_to_undo](const ComponentStorageEntry& entry) {
                              switch (entry.kind) {
                              case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
                                {
                                  auto node_itr = normalized_component.bindings.find(entry.type_id);
                                  if (node_itr == normalized_component.bindings.end()) {
                                    // Not bound yet, keep the new binding.
                                    return false;
                                  }
                                  if (!node_itr.isTerminal()
                                      || node_itr.getNode().object != entry.binding_for_constructed_object.object_ptr) {
                                    std::cerr << multipleBindingsError(entry.type_id) << std::endl;
                                    exit(1);
                                  }
                                  // Already bound in the same way. Skip the new binding.
                                  return true;

                                }

                              case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
                              case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
                                {
                                  const BindingDeps* entry_deps = entry.binding_for_object_to_construct.deps;
                                  for (std::size_t i = 0; i < entry_deps->num_deps; ++i) {
                                    auto binding_compression_itr =
                                        normalized_component.bindingCompressionInfoMap->find(entry_deps->deps[i]);
                                    if (binding_compression_itr != normalized_component.bindingCompressionInfoMap->end()
                                        && binding_compression_itr->second.i_type_id != entry.type_id) {
                                      // The binding compression for `p.second.getDeps()->deps[i]' must be undone because something
                                      // different from binding_compression_itr->iTypeId is now bound to it.
                                      binding_compressions_to_undo.insert(entry_deps->deps[i]);
                                    }
                                  }
                                  auto node_itr = normalized_component.bindings.find(entry.type_id);
                                  if (node_itr == normalized_component.bindings.end()) {
                                    // Not bound yet, keep the new binding.
                                    return false;
                                  }
                                  if (node_itr.isTerminal()
                                      || node_itr.getNode().create != entry.binding_for_object_to_construct.create) {
                                    std::cerr << multipleBindingsError(entry.type_id) << std::endl;
                                    exit(1);
                                  }
                                  // Already bound in the same way. Skip the new binding.
                                  return true;

                                }

                              default:
                              #ifdef FRUIT_EXTRA_DEBUG
                                    std::cerr << "Unexpected kind: " << (std::size_t)entry.kind << std::endl;
                              #endif
                                FruitAssert(false);
                                return true;
                              }
                            });
  bindings_vector.erase(itr, bindings_vector.end());

  // Step 3: undo any binding compressions that can no longer be applied.
  for (TypeId cTypeId : binding_compressions_to_undo) {
    auto binding_compression_itr = normalized_component.bindingCompressionInfoMap->find(cTypeId);
    FruitAssert(binding_compression_itr != normalized_component.bindingCompressionInfoMap->end());
    FruitAssert(!(
        normalized_component.bindings.find(binding_compression_itr->second.i_type_id)
            == normalized_component.bindings.end()));

    ComponentStorageEntry c_binding;
    c_binding.type_id = cTypeId;
    c_binding.kind = ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION;
    c_binding.binding_for_object_to_construct = binding_compression_itr->second.c_binding;

    ComponentStorageEntry i_binding;
    i_binding.type_id = binding_compression_itr->second.i_type_id;
    i_binding.kind = ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION;
    i_binding.binding_for_object_to_construct = binding_compression_itr->second.i_binding;

    bindings_vector.push_back(std::move(c_binding));
    // This TypeId is already in normalized_component.bindings, we overwrite it here.
    bindings_vector.push_back(std::move(i_binding));

#ifdef FRUIT_EXTRA_DEBUG
    std::cout << "InjectorStorage: undoing binding compression for: " << binding_compression_itr->second.i_type_id << "->" << cTypeId << std::endl;
#endif
  }

  bindings = Graph(normalized_component.bindings,
                   BindingDataNodeIter{bindings_vector.begin()},
                   BindingDataNodeIter{bindings_vector.end()});

  // Step 4: Add multibindings.
  BindingNormalization::addMultibindings(multibindings, fixed_size_allocator_data, std::move(multibindings_vector));

  allocator = FixedSizeAllocator(fixed_size_allocator_data);

#ifdef FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

InjectorStorage::~InjectorStorage() {
}

void InjectorStorage::ensureConstructedMultibinding(
    NormalizedMultibindingSet& multibinding_set) {
  for (NormalizedMultibinding& multibinding : multibinding_set.elems) {
    if (!multibinding.is_constructed) {
      multibinding.object = multibinding.create(*this);
      multibinding.is_constructed = true;
    }
  }
}

void* InjectorStorage::getMultibindings(TypeId typeInfo) {
  NormalizedMultibindingSet* multibinding_set = getNormalizedMultibindingSet(typeInfo);
  if (multibinding_set == nullptr) {
    // Not registered.
    return nullptr;
  }
  return multibinding_set->get_multibindings_vector(*this).get();
}

void InjectorStorage::eagerlyInjectMultibindings() {
  for (auto& typeInfoInfoPair : multibindings) {
    typeInfoInfoPair.second.get_multibindings_vector(*this);
  }
}

} // namespace impl
} // namespace fruit
