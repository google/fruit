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

#include <cstdlib>
#include <memory>
#include <functional>
#include <vector>
#include <iostream>
#include <algorithm>
#include "fruit/impl/demangle_type_name.h"
#include "fruit/impl/type_info.h"

#include "fruit/impl/normalized_component_storage.h"
#include <fruit/impl/component_storage.h>

using std::cout;
using std::endl;

using namespace fruit;
using namespace fruit::impl;

namespace {
  
inline std::string multipleBindingsError(TypeId typeInfo) {
  return "Fatal injection error: the type " + typeInfo->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

auto typeInfoLessThanForMultibindings = [](const std::pair<TypeId, BindingDataForMultibinding>& x,
                                           const std::pair<TypeId, BindingDataForMultibinding>& y) {
  return x.first < y.first;
};

inline size_t maximumRequiredSpace(TypeId typeInfo) {
  return typeInfo->alignment() + typeInfo->size() - 1;
}

} // namespace

namespace fruit {
namespace impl {

NormalizedComponentStorage::NormalizedComponentStorage(BindingVectors&& bindingVectors)
  : NormalizedComponentStorage() {
  
  std::vector<std::pair<TypeId, BindingData>>& typeRegistryVector = bindingVectors.first;
  std::vector<std::pair<TypeId, BindingDataForMultibinding>>& typeRegistryVectorForMultibindings = bindingVectors.second;
  
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
  
  for (const auto& p : typeRegistryVector) {
    total_size += maximumRequiredSpace(p.first);
  }
  
  typeRegistry = SemistaticMap<TypeId, BindingData>(typeRegistryVector);
  
  std::sort(typeRegistryVectorForMultibindings.begin(), typeRegistryVectorForMultibindings.end(), typeInfoLessThanForMultibindings);
  
  // Now we must merge multiple bindings for the same type.
  for (auto i = typeRegistryVectorForMultibindings.begin(); i != typeRegistryVectorForMultibindings.end(); /* no increment */) {
    std::pair<TypeId, BindingDataForMultibinding>& x = *i;
    
    BindingDataVectorForMultibinding& b = typeRegistryForMultibindings[x.first];
    b.getSingletonsVector = x.second.getSingletonsVector;
    
    // Insert all multibindings for this type (note that x is also inserted here).
    for (; i != typeRegistryVectorForMultibindings.end() && i->first == x.first; ++i) {
      b.bindingDatas.push_back(BindingDataVectorForMultibinding::Elem(i->second));
    }
  }
  
  for (const auto& typeInfoDataPair : typeRegistryForMultibindings) {
    total_size += maximumRequiredSpace(typeInfoDataPair.first) * typeInfoDataPair.second.bindingDatas.size();
  }
}

// TODO: This can't be inline (let alone defined as `=default') with GCC 4.8, while it would work anyway with Clang.
// Consider minimizing the testcase and filing a bug.
NormalizedComponentStorage::~NormalizedComponentStorage() {
}

fruit::impl::NormalizedComponentStorage
NormalizedComponentStorage::mergeComponentStorages(fruit::impl::NormalizedComponentStorage&& normalizedStorage,
                                                   fruit::impl::ComponentStorage&& storage) {
  storage.flushBindings();

  for (auto& p : storage.typeRegistry) {
    TypeId typeId = p.first;
    bool was_bound = false;
    normalizedStorage.typeRegistry.insert(typeId, p.second, [&was_bound,typeId](const BindingData& b1, const BindingData& b2) {
      if (!(b1 == b2)) {
        std::cerr << multipleBindingsError(typeId) << std::endl;
        exit(1);
      }
      // If not, the type already has this binding, do nothing.
      was_bound = true;
      return b1;
    });
    if (!was_bound) {
      normalizedStorage.total_size += maximumRequiredSpace(typeId);
    }
  }
  
  for (auto& p : storage.typeRegistryForMultibindings) {
    TypeId typeId = p.first;
    BindingDataVectorForMultibinding& b = normalizedStorage.typeRegistryForMultibindings[typeId];
    
    // Might be set already, but we need to set it if there was no multibinding for this type.
    b.getSingletonsVector = p.second.getSingletonsVector;
    
    b.bindingDatas.push_back(BindingDataVectorForMultibinding::Elem(p.second));
    
    normalizedStorage.total_size += maximumRequiredSpace(typeId);
  }
  
  return std::move(normalizedStorage);
}

} // namespace impl
} // namespace fruit
