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

using std::cout;
using std::endl;

namespace {
  
std::string multipleBindingsError(const fruit::impl::TypeInfo* typeInfo) {
  return "Fatal injection error: the type " + typeInfo->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

bool typeInfoLessThan(const std::pair<const fruit::impl::TypeInfo*, fruit::impl::NormalizedComponentStorage::BindingData>& x,
                      const std::pair<const fruit::impl::TypeInfo*, fruit::impl::NormalizedComponentStorage::BindingData>& y) {
  return x.first < y.first;
}

bool typeInfoLessThanForMultibindings(const std::pair<const fruit::impl::TypeInfo*, fruit::impl::NormalizedComponentStorage::BindingDataForMultibinding>& x,
                                      const std::pair<const fruit::impl::TypeInfo*, fruit::impl::NormalizedComponentStorage::BindingDataForMultibinding>& y) {
  return x.first < y.first;
}

bool typeInfoLessThanForMultibindingSet(const std::pair<const fruit::impl::TypeInfo*, fruit::impl::NormalizedComponentStorage::BindingDataSetForMultibinding>& x,
                                        const std::pair<const fruit::impl::TypeInfo*, fruit::impl::NormalizedComponentStorage::BindingDataSetForMultibinding>& y) {
  return x.first < y.first;
}

} // namespace

namespace fruit {
namespace impl {

NormalizedComponentStorage::NormalizedComponentStorage(BindingVectors&& bindingVectors) {
#ifndef FRUIT_NO_SPARSE_HASH
  typeRegistry.set_empty_key(nullptr);
  typeRegistryForMultibindings.set_empty_key(nullptr);
#endif
  
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
    
    BindingDataSetForMultibinding b;
    b.getSingletonSet = x.second.getSingletonSet;
    
    // Insert all multibindings for this type (note that x is also inserted here).
    for (; i != typeRegistryVectorForMultibindings.end() && i->first == x.first; ++i) {
      b.bindingDatas.insert(i->second.bindingData);
    }
    
    typeRegistryForMultibindings[x.first] = b;
  }
  
  total_size = 0;
  for (const auto& typeInfoDataPair : typeRegistry) {
    const TypeInfo* typeInfo = typeInfoDataPair.first;
    total_size += typeInfo->alignment() + typeInfo->size() - 1;
  }
  for (const auto& typeInfoDataPair : typeRegistryForMultibindings) {
    const TypeInfo* typeInfo = typeInfoDataPair.first;
    total_size += (typeInfo->alignment() + typeInfo->size() - 1) * typeInfoDataPair.second.bindingDatas.size();
  }
}

} // namespace impl
} // namespace fruit
