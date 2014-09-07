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

#include "fruit/impl/injector_storage.h"

using std::cout;
using std::endl;

namespace {
  
std::string multipleBindingsError(const fruit::impl::TypeInfo* typeInfo) {
  return "Fatal injection error: the type " + typeInfo->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

bool typeInfoLessThan(const std::pair<const fruit::impl::TypeInfo*, fruit::impl::InjectorStorage::BindingData>& x,
                      const std::pair<const fruit::impl::TypeInfo*, fruit::impl::InjectorStorage::BindingData>& y) {
  return x.first < y.first;
}

bool typeInfoLessThanForMultibindings(const std::pair<const fruit::impl::TypeInfo*, fruit::impl::InjectorStorage::BindingDataForMultibinding>& x,
                                      const std::pair<const fruit::impl::TypeInfo*, fruit::impl::InjectorStorage::BindingDataForMultibinding>& y) {
  return x.first < y.first;
}

bool typeInfoLessThanForMultibindingSet(const std::pair<const fruit::impl::TypeInfo*, fruit::impl::InjectorStorage::BindingDataSetForMultibinding>& x,
                                        const std::pair<const fruit::impl::TypeInfo*, fruit::impl::InjectorStorage::BindingDataSetForMultibinding>& y) {
  return x.first < y.first;
}

} // namespace

namespace fruit {
namespace impl {

void InjectorStorage::ensureConstructed(const TypeInfo* typeInfo, BindingData& bindingData) {
  if (!bindingData.isCreated()) {
    bindingData.create(*this);
    createdSingletons.push_back(typeInfo);
  }
}

void InjectorStorage::ensureConstructedMultibinding(BindingDataSetForMultibinding& bindingDataForMultibinding) {
  // When we construct a singleton in a BindingData we change the order, so we can't do it for bindingDatas already in a set.
  // We need to create a new set.
  std::set<BindingData> newBindingDatas;
  for (BindingData bindingData : bindingDataForMultibinding.bindingDatas) {
    if (!bindingData.isCreated()) {
      bindingData.create(*this);
    }
    newBindingDatas.insert(bindingData);
  }
  std::swap(bindingDataForMultibinding.bindingDatas, newBindingDatas);
}

void* InjectorStorage::getPtr(const TypeInfo* typeInfo) {
  BindingData& bindingData = getBindingData(typeInfo, "attempting to getPtr() on a non-registered type");
  ensureConstructed(typeInfo, bindingData);
  return bindingData.getStoredSingleton();
}

void InjectorStorage::printBindings() {
  cout << "Registered types:" << endl;
  for (const auto& typePair : typeRegistry) {
    std::cout << typePair.first->name() << std::endl;
  }
  for (const auto& typePair : typeRegistryForMultibindings) {
    if (!typePair.second.bindingDatas.empty()) {
      std::cout << typePair.first->name() << " (multibinding)" << std::endl;
    }
  }
  std::cout << std::endl;
}

InjectorStorage::BindingData& InjectorStorage::getBindingData(const TypeInfo* typeInfo, const char* errorMessageIfNonExistent) {
  auto itr = typeRegistry.find(typeInfo);
  // Avoids an unused parameter warning when FruitCheck is a no-op.
  (void)errorMessageIfNonExistent;
  assert(itr != typeRegistry.end());
  return itr->second;
}

InjectorStorage::BindingDataSetForMultibinding* InjectorStorage::getBindingDataSetForMultibinding(const TypeInfo* typeInfo) {
  auto itr = typeRegistryForMultibindings.find(typeInfo);
  if (itr != typeRegistryForMultibindings.end())
    return &(itr->second);
  else
    return nullptr;
}

void InjectorStorage::clear() {
  // Multibindings can depend on bindings, but not vice-versa and they also can't depend on other multibindings.
  // Delete them in any order.
  for (auto& elem : typeRegistryForMultibindings) {
    std::set<BindingData>& bindingDatas = elem.second.bindingDatas;
    for (const BindingData& bindingData : bindingDatas) {
      if (bindingData.isCreated()) {
        bindingData.getDestroy()(bindingData.getStoredSingleton());
      }
    }
  }
  
  for (auto i = createdSingletons.rbegin(), i_end = createdSingletons.rend(); i != i_end; ++i) {
    BindingData& bindingData = getBindingData(*i, "internal error: attempting to destroy an non-registered type");
    // Note: if this was a binding or user-provided object, the object is NOT destroyed.
    if (bindingData.isCreated()) {
      bindingData.getDestroy()(bindingData.getStoredSingleton());
    }
  }
  createdSingletons.clear();
  typeRegistry.clear();
  typeRegistryForMultibindings.clear();
  if (singletonStorageBegin != nullptr) {
    delete [] singletonStorageBegin;
  }
}

InjectorStorage::InjectorStorage(std::vector<std::pair<const TypeInfo*, BindingData>>&& typeRegistryVector,
                                 std::vector<std::pair<const TypeInfo*, BindingDataForMultibinding>>&& typeRegistryVectorForMultibindings){
#ifndef FRUIT_NO_SPARSE_HASH
  typeRegistry.set_empty_key(nullptr);
  typeRegistryForMultibindings.set_empty_key(nullptr);
#endif

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
        std::cerr << multipleBindingsError(x.first);
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
  
  size_t total_size = 0;
  for (const auto& typeInfoDataPair : typeRegistry) {
    const TypeInfo* typeInfo = typeInfoDataPair.first;
    total_size += typeInfo->alignment() + typeInfo->size() - 1;
  }
  for (const auto& typeInfoDataPair : typeRegistryForMultibindings) {
    const TypeInfo* typeInfo = typeInfoDataPair.first;
    total_size += (typeInfo->alignment() + typeInfo->size() - 1) * typeInfoDataPair.second.bindingDatas.size();
  }
  singletonStorageBegin = new char[total_size];
}

InjectorStorage::~InjectorStorage() {
  clear();
}

void* InjectorStorage::getMultibindings(const TypeInfo* typeInfo) {
  BindingDataSetForMultibinding* bindingDataSet = getBindingDataSetForMultibinding(typeInfo);
  if (bindingDataSet == nullptr) {
    // Not registered.
    return nullptr;
  }
  return bindingDataSet->getSingletonSet(*this).get();
}

void InjectorStorage::eagerlyInjectMultibindings() {
  for (auto& typeInfoInfoPair : typeRegistryForMultibindings) {
    typeInfoInfoPair.second.getSingletonSet(*this);
  }
}

} // namespace impl
} // namespace fruit
