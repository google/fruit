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
  
inline std::string multipleBindingsError(const TypeInfo* typeInfo) {
  return "Fatal injection error: the type " + typeInfo->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

auto typeInfoLessThanForMultibindings = [](const std::pair<const TypeInfo*, BindingDataForMultibinding>& x,
                                           const std::pair<const TypeInfo*, BindingDataForMultibinding>& y) {
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
  
  std::vector<BindingData>& typeRegistryVector = bindingVectors.first;
  std::vector<std::pair<const TypeInfo*, BindingDataForMultibinding>>& typeRegistryVectorForMultibindings = bindingVectors.second;
  
  std::sort(typeRegistryVector.begin(), typeRegistryVector.end());
  
  // Now duplicates (either consistent or non-consistent) might exist.
  auto firstFreePos = typeRegistryVector.begin();
  for (auto i = typeRegistryVector.begin(); i != typeRegistryVector.end(); /* no increment */) {
    BindingData& x = *i;
    *firstFreePos = *i;
    ++firstFreePos;
    
    // Check that other bindings for the same type (if any) are equal.
    for (++i; i != typeRegistryVector.end() && i->getKey() == x.getKey(); ++i) {
      if (!(x == *i)) {
        std::cerr << multipleBindingsError(x.getKey()) << std::endl;
        exit(1);
      }
    }
  }
  typeRegistryVector.erase(firstFreePos, typeRegistryVector.end());
  
  for (const BindingData& bindingData : typeRegistryVector) {
    total_size += maximumRequiredSpace(bindingData.getKey());
  }
  
  typeRegistry = SemistaticMap(typeRegistryVector);
  
  std::sort(typeRegistryVectorForMultibindings.begin(), typeRegistryVectorForMultibindings.end(), typeInfoLessThanForMultibindings);
  
  // Now we must merge multiple bindings for the same type.
  for (auto i = typeRegistryVectorForMultibindings.begin(); i != typeRegistryVectorForMultibindings.end(); /* no increment */) {
    std::pair<const TypeInfo*, BindingDataForMultibinding>& x = *i;
    
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

  for (BindingData& bindingData : storage.typeRegistry) {
    bool was_bound = false;
    normalizedStorage.typeRegistry.insert(bindingData, [&was_bound](const BindingData& b1, const BindingData& b2) {
      if (!(b1 == b2)) {
        std::cerr << multipleBindingsError(b1.getKey()) << std::endl;
        exit(1);
      }
      // If not, the type already has this binding, do nothing.
      was_bound = true;
      return b1;
    });
    if (!was_bound) {
      normalizedStorage.total_size += maximumRequiredSpace(bindingData.getKey());
    }
  }
  
  for (auto& x : storage.typeRegistryForMultibindings) {
    BindingDataVectorForMultibinding& b = normalizedStorage.typeRegistryForMultibindings[x.first];
    
    // Might be set already, but we need to set it if there was no multibinding for this type.
    b.getSingletonsVector = x.second.getSingletonsVector;
    
    b.bindingDatas.push_back(BindingDataVectorForMultibinding::Elem(x.second));
    
    normalizedStorage.total_size += maximumRequiredSpace(x.first);
  }
  
  return std::move(normalizedStorage);
}

} // namespace impl
} // namespace fruit
