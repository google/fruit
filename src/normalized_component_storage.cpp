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
#include "fruit/impl/metaprogramming.h"
#include "fruit/impl/demangle_type_name.h"
#include "fruit/impl/type_info.h"

#include "fruit/impl/normalized_component_storage.h"
#include <fruit/impl/component_storage.h>

using std::cout;
using std::endl;

using namespace fruit;
using namespace fruit::impl;

namespace {
  
inline std::string multipleBindingsError(const TypeInfo* typeInfo) {
  return "Fatal injection error: the type " + typeInfo->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

auto typeInfoLessThan = [](const std::pair<const TypeInfo*, NormalizedComponentStorage::BindingData>& x,
                           const std::pair<const TypeInfo*, NormalizedComponentStorage::BindingData>& y) {
  return x.first < y.first;
};

auto typeInfoLessThanForMultibindings = [](const std::pair<const TypeInfo*, NormalizedComponentStorage::BindingDataForMultibinding>& x,
                                           const std::pair<const TypeInfo*, NormalizedComponentStorage::BindingDataForMultibinding>& y) {
  return x.first < y.first;
};

inline size_t maximumRequiredSpace(const TypeInfo* typeInfo) {
  return typeInfo->alignment() + typeInfo->size() - 1;
}

} // namespace

namespace fruit {
namespace impl {

NormalizedComponentStorage::NormalizedComponentStorage(BindingVectors&& bindingVectors)
  : NormalizedComponentStorage() {
  
  std::vector<std::pair<const TypeInfo*, BindingData>>& typeRegistryVector = bindingVectors.first;
  std::vector<std::pair<const TypeInfo*, BindingDataForMultibinding>>& typeRegistryVectorForMultibindings = bindingVectors.second;
  
  std::sort(typeRegistryVector.begin(), typeRegistryVector.end(), typeInfoLessThan);
  
  // Now duplicates (either consistent or non-consistent) might exist.
  auto firstFreePos = typeRegistryVector.begin();
  for (auto i = typeRegistryVector.begin(); i != typeRegistryVector.end(); /* no increment */) {
    std::pair<const TypeInfo*, BindingData>& x = *i;
    *firstFreePos = *i;
    ++firstFreePos;
    
    // Check that other bindings for the same type (if any) are equal.
    for (++i; i != typeRegistryVector.end() && i->first == x.first; ++i) {
      if (x != *i) {
        std::cerr << multipleBindingsError(x.first) << std::endl;
        abort();
      }
    }
  }
  typeRegistry.insert(typeRegistryVector.begin(), firstFreePos);
  
  std::sort(typeRegistryVectorForMultibindings.begin(), typeRegistryVectorForMultibindings.end(), typeInfoLessThanForMultibindings);
  
  // Now we must merge multiple bindings for the same type.
  for (auto i = typeRegistryVectorForMultibindings.begin(); i != typeRegistryVectorForMultibindings.end(); /* no increment */) {
    std::pair<const TypeInfo*, BindingDataForMultibinding>& x = *i;
    
    BindingDataSetForMultibinding& b = typeRegistryForMultibindings[x.first];
    b.getSingletonSet = x.second.getSingletonSet;
    
    // Insert all multibindings for this type (note that x is also inserted here).
    for (; i != typeRegistryVectorForMultibindings.end() && i->first == x.first; ++i) {
      b.bindingDatas.insert(i->second.bindingData);
    }
  }
  
  total_size = 0;
  for (const auto& typeInfoDataPair : typeRegistry) {
    total_size += maximumRequiredSpace(typeInfoDataPair.first);
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

  for (auto& x : storage.typeRegistry) {
    auto itr = normalizedStorage.typeRegistry.find(x.first);
    if (itr != normalizedStorage.typeRegistry.end()) {
      if (!(x.second == itr->second)) {
        std::cerr << multipleBindingsError(x.first) << std::endl;
        abort();
      }
      // If not, the type already has this binding, do nothing.
    } else {
      // The type is not bound, add the binding and update total_size.
      normalizedStorage.typeRegistry[x.first] = x.second;
      normalizedStorage.total_size += maximumRequiredSpace(x.first);
    }
  }
  
  for (auto& x : storage.typeRegistryForMultibindings) {
    BindingDataSetForMultibinding& b = normalizedStorage.typeRegistryForMultibindings[x.first];
    
    // Might be set already, but we need to set it if there was no multibinding for this type.
    b.getSingletonSet = x.second.getSingletonSet;
    
    size_t old_size = b.bindingDatas.size();
    b.bindingDatas.insert(x.second.bindingData);
    
    if (old_size < b.bindingDatas.size()) {
      // Inserted a new multibinding.
      normalizedStorage.total_size += maximumRequiredSpace(x.first);
    }
  }
  
  return std::move(normalizedStorage);
}

} // namespace impl
} // namespace fruit
