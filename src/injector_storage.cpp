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
  for (const auto& typePair : storage.typeRegistry) {
    std::cout << typePair.first->name() << std::endl;
  }
  for (const auto& typePair : storage.typeRegistryForMultibindings) {
    if (!typePair.second.bindingDatas.empty()) {
      std::cout << typePair.first->name() << " (multibinding)" << std::endl;
    }
  }
  std::cout << std::endl;
}

NormalizedComponentStorage::BindingData& InjectorStorage::getBindingData(const TypeInfo* typeInfo, const char* errorMessageIfNonExistent) {
  auto itr = storage.typeRegistry.find(typeInfo);
  // Avoids an unused parameter warning when FruitCheck is a no-op.
  (void)errorMessageIfNonExistent;
  assert(itr != storage.typeRegistry.end());
  return itr->second;
}

NormalizedComponentStorage::BindingDataSetForMultibinding* InjectorStorage::getBindingDataSetForMultibinding(const TypeInfo* typeInfo) {
  auto itr = storage.typeRegistryForMultibindings.find(typeInfo);
  if (itr != storage.typeRegistryForMultibindings.end())
    return &(itr->second);
  else
    return nullptr;
}

void InjectorStorage::clear() {
  // Multibindings can depend on bindings, but not vice-versa and they also can't depend on other multibindings.
  // Delete them in any order.
  for (auto& elem : storage.typeRegistryForMultibindings) {
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
  storage = NormalizedComponentStorage();
  delete [] singletonStorageBegin;
}

InjectorStorage::InjectorStorage(NormalizedComponentStorage&& storage1)
  : storage(std::move(storage1)) {
  singletonStorageBegin = new char[storage.total_size];
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
  for (auto& typeInfoInfoPair : storage.typeRegistryForMultibindings) {
    typeInfoInfoPair.second.getSingletonSet(*this);
  }
}

} // namespace impl
} // namespace fruit
