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
#include "fruit/impl/component.utils.h"

#include "fruit/impl/injector_storage.h"

using std::cout;
using std::endl;

namespace fruit {
namespace impl {

void InjectorStorage::ensureConstructedMultibinding(BindingDataVectorForMultibinding& bindingDataForMultibinding) {
  for (BindingData& bindingData : bindingDataForMultibinding.bindingDatas) {
    if (!bindingData.isCreated()) {
      bindingData.create(*this);
    }
  }
}

void* InjectorStorage::getPtr(const TypeInfo* typeInfo) {
  BindingData& bindingData = storage.typeRegistry.at(typeInfo);
  ensureConstructed(typeInfo, bindingData);
  return bindingData.getStoredSingleton();
}

inline NormalizedComponentStorage::BindingDataVectorForMultibinding* InjectorStorage::getBindingDataVectorForMultibinding(const TypeInfo* typeInfo) {
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
    std::vector<BindingData>& bindingDatas = elem.second.bindingDatas;
    for (const BindingData& bindingData : bindingDatas) {
      if (bindingData.isCreated() && bindingData.getDestroy() != nullptr) {
        bindingData.getDestroy()(bindingData.getStoredSingleton());
      }
    }
  }
  
  for (auto i = createdSingletons.rbegin(), i_end = createdSingletons.rend(); i != i_end; ++i) {
    BindingData& bindingData = storage.typeRegistry.at(*i);
    // Note: if this was a binding or user-provided object, the object is NOT destroyed.
    if (bindingData.isCreated()) {
      bindingData.getDestroy()(bindingData.getStoredSingleton());
    }
  }
  createdSingletons.clear();
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
  BindingDataVectorForMultibinding* bindingDataVector = getBindingDataVectorForMultibinding(typeInfo);
  if (bindingDataVector == nullptr) {
    // Not registered.
    return nullptr;
  }
  return bindingDataVector->getSingletonsVector(*this).get();
}

void InjectorStorage::eagerlyInjectMultibindings() {
  for (auto& typeInfoInfoPair : storage.typeRegistryForMultibindings) {
    typeInfoInfoPair.second.getSingletonsVector(*this);
  }
}

} // namespace impl
} // namespace fruit
