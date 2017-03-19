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

#include <fruit/impl/storage/injector_storage.h>
#include <fruit/impl/storage/component_storage.h>
#include <fruit/impl/data_structures/semistatic_graph.templates.h>
#include <fruit/impl/meta/basics.h>
#include <fruit/impl/storage/normalized_component_storage.h>

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

InjectorStorage::InjectorStorage(const ComponentStorage& component, const std::vector<TypeId>& exposed_types)
  : normalized_component_storage_ptr(new NormalizedComponentStorage(component, exposed_types)),
    allocator(normalized_component_storage_ptr->fixed_size_allocator_data),
    bindings(normalized_component_storage_ptr->bindings, (DummyNode<TypeId, NormalizedBindingData>*)nullptr, (DummyNode<TypeId, NormalizedBindingData>*)nullptr),
    multibindings(std::move(normalized_component_storage_ptr->multibindings)) {

#ifdef FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

InjectorStorage::InjectorStorage(const NormalizedComponentStorage& normalized_component,
                                 const ComponentStorage& component,
                                 std::vector<TypeId>&& exposed_types)
  : multibindings(normalized_component.multibindings) {

  FixedSizeAllocator::FixedSizeAllocatorData fixed_size_allocator_data = normalized_component.fixed_size_allocator_data;
  
  ;
  
  // Step 1: Remove duplicates among the new bindings, and check for inconsistent bindings within `component' alone.
  // Note that we do NOT use component.compressed_bindings here, to avoid having to check if these compressions can be undone.
  // We don't expect many binding compressions here that weren't already performed in the normalized component.
  BindingNormalization::BindingCompressionInfoMap bindingCompressionInfoMapUnused;
  std::vector<std::pair<TypeId, BindingData>> normalized_bindings =
      BindingNormalization::normalizeBindings(component.bindings,
                                              fixed_size_allocator_data,
                                              std::vector<CompressedBinding>{},
                                              component.multibindings,
                                              std::move(exposed_types),
                                              bindingCompressionInfoMapUnused);
  FruitAssert(bindingCompressionInfoMapUnused.empty());
  
  HashSet<TypeId> binding_compressions_to_undo = createHashSet<TypeId>();
  
  // Step 2: Filter out already-present bindings, and check for inconsistent bindings between `normalizedComponent' and
  // `component'. Also determine what binding compressions must be undone
  auto itr = std::remove_if(normalized_bindings.begin(), normalized_bindings.end(),
                            [&normalized_component, &binding_compressions_to_undo](const std::pair<TypeId, BindingData>& p) {
                              if (!p.second.isCreated()) {
                                for (std::size_t i = 0; i < p.second.getDeps()->num_deps; ++i) {
                                  auto binding_compression_itr = 
                                      normalized_component.bindingCompressionInfoMap->find(p.second.getDeps()->deps[i]);
                                  if (binding_compression_itr != normalized_component.bindingCompressionInfoMap->end()
                                      && binding_compression_itr->second.iTypeId != p.first) {
                                    // The binding compression for `p.second.getDeps()->deps[i]' must be undone because something
                                    // different from binding_compression_itr->iTypeId is now bound to it.
                                    binding_compressions_to_undo.insert(p.second.getDeps()->deps[i]);
                                  }
                                }
                              }
                              auto node_itr = normalized_component.bindings.find(p.first);
                              if (node_itr == normalized_component.bindings.end()) {
                                // Not bound yet, keep the new binding.
                                return false;
                              }
                              if (!(node_itr.getNode() == NormalizedBindingData(p.second))) {
                                std::cerr << multipleBindingsError(p.first) << std::endl;
                                exit(1);
                              }
                              // Already bound in the same way. Skip the new binding.
                              return true;
                            });
  normalized_bindings.erase(itr, normalized_bindings.end());
  
  // Step 3: undo any binding compressions that can no longer be applied.
  for (TypeId cTypeId : binding_compressions_to_undo) {
    auto binding_compression_itr = normalized_component.bindingCompressionInfoMap->find(cTypeId);
    FruitAssert(binding_compression_itr != normalized_component.bindingCompressionInfoMap->end());
    FruitAssert(!binding_compression_itr->second.iBinding.needsAllocation());
    normalized_bindings.emplace_back(cTypeId, binding_compression_itr->second.cBinding);
    // This TypeId is already in normalized_component.bindings, we overwrite it here.
    FruitAssert(!(normalized_component.bindings.find(binding_compression_itr->second.iTypeId) == normalized_component.bindings.end()));
    normalized_bindings.emplace_back(binding_compression_itr->second.iTypeId, binding_compression_itr->second.iBinding);
#ifdef FRUIT_EXTRA_DEBUG
    std::cout << "InjectorStorage: undoing binding compression for: " << binding_compression_itr->second.iTypeId << "->" << cTypeId << std::endl;  
#endif
  }
  
  bindings = Graph(normalized_component.bindings,
                   BindingDataNodeIter{normalized_bindings.begin()},
                   BindingDataNodeIter{normalized_bindings.end()});
  
  // Step 4: Add multibindings.
  BindingNormalization::addMultibindings(multibindings, fixed_size_allocator_data, std::move(component.multibindings));
  
  allocator = FixedSizeAllocator(fixed_size_allocator_data);
  
#ifdef FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

InjectorStorage::~InjectorStorage() {
}

void InjectorStorage::ensureConstructedMultibinding(NormalizedMultibindingData& bindingDataForMultibinding) {
  for (NormalizedMultibindingData::Elem& elem : bindingDataForMultibinding.elems) {
    if (elem.object == nullptr) {
      elem.object = elem.create(*this);
    }
  }
}

void* InjectorStorage::getMultibindings(TypeId typeInfo) {
  NormalizedMultibindingData* bindingDataVector = getNormalizedMultibindingData(typeInfo);
  if (bindingDataVector == nullptr) {
    // Not registered.
    return nullptr;
  }
  return bindingDataVector->get_multibindings_vector(*this).get();
}

void InjectorStorage::eagerlyInjectMultibindings() {
  for (auto& typeInfoInfoPair : multibindings) {
    typeInfoInfoPair.second.get_multibindings_vector(*this);
  }
}

} // namespace impl
} // namespace fruit
