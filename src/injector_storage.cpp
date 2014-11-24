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
#include <functional>
#include <vector>
#include <iostream>
#include <algorithm>
#include "fruit/impl/util/demangle_type_name.h"
#include "fruit/impl/util/type_info.h"

#include "fruit/impl/storage/injector_storage.h"
#include "fruit/impl/storage/component_storage.h"
#include "fruit/impl/data_structures/semistatic_graph.templates.h"

using std::cout;
using std::endl;

using namespace fruit::impl;

namespace {

std::string multipleBindingsError(TypeId typeId) {
  return "Fatal injection error: the type " + typeId.type_info->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

auto typeInfoLessThanForMultibindings = [](const std::pair<TypeId, MultibindingData>& x,
                                           const std::pair<TypeId, MultibindingData>& y) {
  return x.first < y.first;
};

} // namespace

namespace fruit {
namespace impl {

void InjectorStorage::fatal(const std::string& error) {
  std::cerr << "Fatal injection error: " << error << std::endl;
  exit(1);
}

void InjectorStorage::normalizeBindings(std::vector<std::pair<TypeId, BindingData>>& bindingsVector,
                                        std::size_t& total_size,
                                        std::vector<CompressedBinding>&& compressedBindingsVector,
                                        const std::vector<std::pair<TypeId, MultibindingData>>& multibindingsVector,
                                        std::initializer_list<TypeId> exposedTypes) {
  std::unordered_map<TypeId, BindingData> bindingDataMap;
  
  for (auto& p : bindingsVector) {
    auto itr = bindingDataMap.find(p.first);
    if (itr != bindingDataMap.end()) {
      if (!(p.second == itr->second)) {
        std::cerr << multipleBindingsError(p.first) << std::endl;
        exit(1);
      }
      // Otherwise ok, duplicate but consistent binding.
      
    } else {
      // New binding, add it to the map.
      bindingDataMap[p.first] = p.second;
    }
  }
  
  for (const auto& p : bindingsVector) {
    total_size += FixedSizeAllocator::maximumRequiredSpace(p.first);
  }
  
  // Remove duplicates from `compressedBindingsVector'.
  
  // CtypeId -> (ItypeId, bindingData)
  std::unordered_map<TypeId, std::pair<TypeId, BindingData>> compressedBindingsMap;
  
  // This also removes any duplicates. No need to check for multiple I->C, I2->C mappings, will filter these out later when 
  // considering deps.
  for (CompressedBinding& compressedBinding : compressedBindingsVector) {
    compressedBindingsMap[compressedBinding.classId] 
      = std::make_pair(compressedBinding.interfaceId, compressedBinding.bindingData);
  }
  
  // We can't compress the binding if C is a dep of a multibinding.
  for (auto p : multibindingsVector) {
    const BindingDeps* deps = p.second.deps;
    if (deps != nullptr) {
      for (std::size_t i = 0; i < deps->num_deps; ++i) {
        compressedBindingsMap.erase(deps->deps[i]);
      }
    }
  }
  
  // We can't compress the binding if C is an exposed type (but I is likely to be exposed instead).
  for (TypeId typeId : exposedTypes) {
    compressedBindingsMap.erase(typeId);
  }
  
  // We can't compress the binding if some type X depends on C and X!=I.
  for (auto& p : bindingDataMap) {
    TypeId xId = p.first;
    BindingData bindingData = p.second;
    if (!bindingData.isCreated()) {
      for (std::size_t i = 0; i < bindingData.getDeps()->num_deps; ++i) {
        TypeId cId = bindingData.getDeps()->deps[i];
        auto itr = compressedBindingsMap.find(cId);
        if (itr != compressedBindingsMap.end() && itr->second.first != xId) {
          compressedBindingsMap.erase(itr);
        }
      }
    }
  }
  
  // Two pairs of compressible bindings (I->C) and (C->X) can not exist (the C of a compressible binding is always bound either
  // using constructor binding or provider binding, it can't be a binding itself). So no need to check for that.
  
  // Now perform the binding compression.
  for (auto& p : compressedBindingsMap) {
    TypeId cId = p.first;
    TypeId iId = p.second.first;
    BindingData bindingData = p.second.second;
    bindingDataMap[iId] = bindingData;
    bindingDataMap.erase(cId);
    // Note that even if I is the one that remains, C is the one that will be allocated, not I.
    total_size -= FixedSizeAllocator::maximumRequiredSpace(iId);
  }
  
  // Copy the resulting bindings back into the vector.
    bindingsVector.clear();
  for (auto& p : bindingDataMap) {
        bindingsVector.push_back(p);
  }
}

void InjectorStorage::addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingData>& multibindings,
                                       std::size_t& total_size,
                                       std::vector<std::pair<TypeId, MultibindingData>>&& multibindingsVector) {
  
  std::sort(multibindingsVector.begin(), multibindingsVector.end(), 
            typeInfoLessThanForMultibindings);
  
  // Now we must merge multiple bindings for the same type.
  for (auto i = multibindingsVector.begin(); i != multibindingsVector.end(); /* no increment */) {
    std::pair<TypeId, MultibindingData>& x = *i;
    NormalizedMultibindingData& b = multibindings[x.first];
    
    // Might be set already, but we need to set it if there was no multibinding for this type.
    b.getSingletonsVector = x.second.getSingletonsVector;
    
    // Insert all multibindings for this type (note that x is also inserted here).
    for (; i != multibindingsVector.end() && i->first == x.first; ++i) {
      b.elems.push_back(NormalizedMultibindingData::Elem(i->second));
      total_size += FixedSizeAllocator::maximumRequiredSpace(x.first);
    }
  }
}

InjectorStorage::InjectorStorage(ComponentStorage&& component, std::initializer_list<TypeId> exposedTypes)
  : normalizedComponentStoragePtr(new NormalizedComponentStorage(std::move(component), exposedTypes)),
    allocator(normalizedComponentStoragePtr->total_size),
    // TODO: Remove the move operation here once the shallow copy optimization for SemistaticGraph is in place.
    bindings(std::move(normalizedComponentStoragePtr->bindings)),
    multibindings(std::move(normalizedComponentStoragePtr->multibindings)) {

  onDestruction.reserve(bindings.size());
  
#ifdef FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

InjectorStorage::InjectorStorage(const NormalizedComponentStorage& normalizedComponent,
                                 ComponentStorage&& component,
                                 std::initializer_list<TypeId> exposedTypes)
  : multibindings(normalizedComponent.multibindings) {

  std::size_t total_size = normalizedComponent.total_size;
  
  component.flushBindings();
  
  // Step 1: Remove duplicates among the new bindings, and check for inconsistent bindings within `component' alone.
  normalizeBindings(component.bindings,
                    total_size,
                    std::move(component.compressedBindings),
                    component.multibindings,
                    exposedTypes);
  
  // Step 1: Filter out already-present bindings, and check for inconsistent bindings between `normalizedComponent' and
  // `component'.
  auto itr = std::remove_if(component.bindings.begin(), component.bindings.end(),
                            [&normalizedComponent](const std::pair<TypeId, BindingData>& p) {
                              auto node_itr = normalizedComponent.bindings.find(p.first);
                              if (node_itr == normalizedComponent.bindings.end()) {
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
  component.bindings.erase(itr, component.bindings.end());
  
  bindings = Graph(normalizedComponent.bindings,
                   NormalizedComponentStorage::BindingDataNodeIter{component.bindings.begin()},
                   NormalizedComponentStorage::BindingDataNodeIter{component.bindings.end()});
  
  // Step 4: Add multibindings.
  addMultibindings(multibindings, total_size, std::move(component.multibindings));
  
  allocator = FixedSizeAllocator(total_size);
  
  onDestruction.reserve(bindings.size());
  
#ifdef FRUIT_EXTRA_DEBUG
  bindings.checkFullyConstructed();
#endif
}

void InjectorStorage::ensureConstructedMultibinding(NormalizedMultibindingData& bindingDataForMultibinding) {
  for (NormalizedMultibindingData::Elem& elem : bindingDataForMultibinding.elems) {
    if (elem.object == nullptr) {
      elem.object = elem.create(*this);
    }
  }
}

InjectorStorage::~InjectorStorage() {
  for (auto i = onDestruction.rbegin(), i_end = onDestruction.rend(); i != i_end; ++i) {
    BindingData::destroy_t destroy = i->first;
    void* p = i->second;
    destroy(p);
  }
  onDestruction.clear();
}

void* InjectorStorage::getMultibindings(TypeId typeInfo) {
  NormalizedMultibindingData* bindingDataVector = getNormalizedMultibindingData(typeInfo);
  if (bindingDataVector == nullptr) {
    // Not registered.
    return nullptr;
  }
  return bindingDataVector->getSingletonsVector(*this).get();
}

void InjectorStorage::eagerlyInjectMultibindings() {
  for (auto& typeInfoInfoPair : multibindings) {
    typeInfoInfoPair.second.getSingletonsVector(*this);
  }
}

} // namespace impl
} // namespace fruit
