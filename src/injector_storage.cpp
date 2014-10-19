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
#include "fruit/impl/component.utils.h"

#include "fruit/impl/injector_storage.h"

using std::cout;
using std::endl;

namespace fruit {
namespace impl {

void InjectorStorage::ensureConstructedMultibinding(BindingDataVectorForMultibinding& bindingDataForMultibinding) {
  for (BindingDataVectorForMultibinding::Elem& bindingData : bindingDataForMultibinding.bindingDatas) {
    if (bindingData.object == nullptr) {
      std::tie(bindingData.object, bindingData.destroy) = bindingData.create(*this);
    }
  }
}

inline BindingDataVectorForMultibinding* InjectorStorage::getBindingDataVectorForMultibinding(const TypeInfo* typeInfo) {
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
    std::vector<BindingDataVectorForMultibinding::Elem>& bindingDatas = elem.second.bindingDatas;
    for (const BindingDataVectorForMultibinding::Elem& bindingData : bindingDatas) {
      if (bindingData.object != nullptr && bindingData.destroy != nullptr) {
        bindingData.destroy(bindingData.object);
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

InjectorStorage::InjectorStorage(NormalizedComponentStorage&& storage1)
  : storage(std::move(storage1)) {
  // The +1 is because we waste the first byte (singletonStorageLastUsed points to the beginning of storage).
  singletonStorageBegin = new char[storage.total_size + 1];
  singletonStorageLastUsed = singletonStorageBegin;
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
