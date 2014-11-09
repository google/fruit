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

void InjectorStorage::normalizeTypeRegistryVector(std::vector<std::pair<TypeId, BindingData>>& typeRegistryVector) {
  std::sort(typeRegistryVector.begin(), typeRegistryVector.end());
  
  // Now duplicates (either consistent or non-consistent) might exist.
  auto firstFreePos = typeRegistryVector.begin();
  for (auto i = typeRegistryVector.begin(); i != typeRegistryVector.end(); /* no increment */) {
    TypeId typeId = i->first;
    BindingData& x = i->second;
    *firstFreePos = *i;
    ++firstFreePos;
    
    // Check that other bindings for the same type (if any) are equal.
    for (++i; i != typeRegistryVector.end() && i->first == typeId; ++i) {
      if (!(x == i->second)) {
        std::cerr << multipleBindingsError(typeId) << std::endl;
        exit(1);
      }
    }
  }
  typeRegistryVector.erase(firstFreePos, typeRegistryVector.end());
}

void InjectorStorage::addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingData>& typeRegistryForMultibindings,
                                       std::size_t& total_size,
                                       std::vector<std::pair<TypeId, MultibindingData>>&& typeRegistryVectorForMultibindings) {
  
  std::sort(typeRegistryVectorForMultibindings.begin(), typeRegistryVectorForMultibindings.end(), 
            typeInfoLessThanForMultibindings);
  
  // Now we must merge multiple bindings for the same type.
  for (auto i = typeRegistryVectorForMultibindings.begin(); i != typeRegistryVectorForMultibindings.end(); /* no increment */) {
    std::pair<TypeId, MultibindingData>& x = *i;
    NormalizedMultibindingData& b = typeRegistryForMultibindings[x.first];
    
    // Might be set already, but we need to set it if there was no multibinding for this type.
    b.getSingletonsVector = x.second.getSingletonsVector;
    
    // Insert all multibindings for this type (note that x is also inserted here).
    for (; i != typeRegistryVectorForMultibindings.end() && i->first == x.first; ++i) {
      b.elems.push_back(NormalizedMultibindingData::Elem(i->second));
      total_size += InjectorStorage::maximumRequiredSpace(x.first);
    }
  }
}

InjectorStorage::InjectorStorage(ComponentStorage&& component)
  : normalizedComponentStoragePtr(new NormalizedComponentStorage(std::move(component))),
    // TODO: Remove the move operation here once the shallow copy optimization for SemistaticGraph is in place.
    typeRegistry(std::move(normalizedComponentStoragePtr->typeRegistry)),
    typeRegistryForMultibindings(std::move(normalizedComponentStoragePtr->typeRegistryForMultibindings)) {

  std::size_t total_size = normalizedComponentStoragePtr->total_size;
  
  // The +1 is because we waste the first byte (singletonStorageLastUsed points to the beginning of storage).
  singletonStorageBegin = new char[total_size + 1];
  singletonStorageLastUsed = singletonStorageBegin;
  
#ifdef FRUIT_EXTRA_DEBUG
  typeRegistry.checkFullyConstructed();
#endif
}

InjectorStorage::InjectorStorage(const NormalizedComponentStorage& normalizedComponent,
                                 ComponentStorage&& component)
  : typeRegistryForMultibindings(normalizedComponent.typeRegistryForMultibindings) {

  std::size_t total_size = normalizedComponent.total_size;
  
  component.flushBindings();
  
  // Step 1: Remove duplicates among the new bindings, and check for inconsistent bindings within `component' alone.
  normalizeTypeRegistryVector(component.typeRegistry);
  
  // Step 2: Filter out already-present bindings, and check for inconsistent bindings between `normalizedComponent' and
  // `component'.
  auto itr = std::remove_if(component.typeRegistry.begin(), component.typeRegistry.end(),
                            [&normalizedComponent](const std::pair<TypeId, BindingData>& p) {
                              auto node_itr = normalizedComponent.typeRegistry.find(p.first);
                              if (node_itr == normalizedComponent.typeRegistry.end()) {
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
  component.typeRegistry.erase(itr, component.typeRegistry.end());
  
  typeRegistry = Graph(normalizedComponent.typeRegistry,
                       NormalizedComponentStorage::BindingDataNodeIter{component.typeRegistry.begin()},
                       NormalizedComponentStorage::BindingDataNodeIter{component.typeRegistry.end()});
  
  // Step 3: Update total_size taking into account the new bindings.
  for (auto& p : component.typeRegistry) {
    total_size += maximumRequiredSpace(p.first);
  }
  
  // Step 4: Add multibindings.
  addMultibindings(typeRegistryForMultibindings, total_size, std::move(component.typeRegistryForMultibindings));
  
  // The +1 is because we waste the first byte (singletonStorageLastUsed points to the beginning of storage).
  singletonStorageBegin = new char[total_size + 1];
  singletonStorageLastUsed = singletonStorageBegin;
  
#ifdef FRUIT_EXTRA_DEBUG
  typeRegistry.checkFullyConstructed();
#endif
}

void InjectorStorage::ensureConstructedMultibinding(NormalizedMultibindingData& bindingDataForMultibinding) {
  for (NormalizedMultibindingData::Elem& elem : bindingDataForMultibinding.elems) {
    if (elem.object == nullptr) {
      std::tie(elem.object, elem.destroy) = elem.create(*this);
    }
  }
}

void InjectorStorage::clear() {
  // Multibindings can depend on bindings, but not vice-versa and they also can't depend on other multibindings.
  // Delete them in any order.
  for (const auto& p : typeRegistryForMultibindings) {
    for (const NormalizedMultibindingData::Elem& elem : p.second.elems) {
      if (elem.object != nullptr && elem.destroy != nullptr) {
        elem.destroy(elem.object);
      }
    }
  }
  
  for (auto i = onDestruction.rbegin(), i_end = onDestruction.rend(); i != i_end; ++i) {
    BindingData::destroy_t destroy = *i;
    destroy(*this);
  }
  onDestruction.clear();
  delete [] singletonStorageBegin;
}


InjectorStorage::~InjectorStorage() {
  clear();
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
  for (auto& typeInfoInfoPair : typeRegistryForMultibindings) {
    typeInfoInfoPair.second.getSingletonsVector(*this);
  }
}

} // namespace impl
} // namespace fruit
