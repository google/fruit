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
#include "fruit/impl/util/demangle_type_name.h"
#include "fruit/impl/util/type_info.h"

#include "fruit/impl/storage/component_storage.h"

using std::cout;
using std::endl;

namespace fruit {
namespace impl {

void ComponentStorage::fatal(const std::string& error) {
  std::cerr << "Fatal injection error: " << error << std::endl;
  exit(1);
}

void ComponentStorage::install(ComponentStorage other) {
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.typeRegistry.capacity() > typeRegistry.capacity()) {
    std::swap(typeRegistry, other.typeRegistry);
  }
  for (size_t i = 0; i < other.typeRegistryArray_numUsed; i++) {
    if (typeRegistryArray_numUsed < max_num_immediate_bindings) {
      typeRegistryArray[typeRegistryArray_numUsed] = other.typeRegistryArray[i];
      ++typeRegistryArray_numUsed;
    } else {
      typeRegistry.push_back(other.typeRegistryArray[i]);
    }
  }
  typeRegistry.insert(typeRegistry.end(),
                      other.typeRegistry.begin(),
                      other.typeRegistry.end());
  
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.typeRegistryForMultibindings.capacity() > typeRegistryForMultibindings.capacity()) {
    std::swap(typeRegistryForMultibindings, other.typeRegistryForMultibindings);
  }
  typeRegistryForMultibindings.insert(typeRegistryForMultibindings.end(),
                                      other.typeRegistryForMultibindings.begin(),
                                      other.typeRegistryForMultibindings.end());
  
  
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.compressedBindings.capacity() > compressedBindings.capacity()) {
    swap(other.compressedBindings, compressedBindings);
  }
  compressedBindings.insert(compressedBindings.end(), other.compressedBindings.begin(), other.compressedBindings.end());
  
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.multibindingDeps.capacity() > multibindingDeps.capacity()) {
    swap(other.multibindingDeps, multibindingDeps);
  }
  multibindingDeps.insert(multibindingDeps.end(), other.multibindingDeps.begin(), other.multibindingDeps.end());
}

void ComponentStorage::createBindingData(TypeId typeInfo,
                                         BindingData bindingData) {
  if (typeRegistryArray_numUsed < max_num_immediate_bindings) {
    typeRegistryArray[typeRegistryArray_numUsed] = std::make_pair(typeInfo, bindingData);
    ++typeRegistryArray_numUsed;
  } else {
    typeRegistry.emplace_back(typeInfo, bindingData);
  }
}

void ComponentStorage::createMultibindingData(TypeId typeInfo,
                                              MultibindingData::create_t create,
                                              std::shared_ptr<char>(*createSet)(InjectorStorage&)) {
  MultibindingData multibindingData;
  multibindingData.create = create;
  multibindingData.getSingletonsVector = createSet;
  
  typeRegistryForMultibindings.emplace_back(typeInfo, multibindingData);
}

void ComponentStorage::createMultibindingData(TypeId typeInfo,
                                              MultibindingData::object_t storedSingleton,
                                              std::shared_ptr<char>(*createSet)(InjectorStorage&)) {
  MultibindingData multibindingData;
  multibindingData.object = storedSingleton;
  multibindingData.getSingletonsVector = createSet;
  
  typeRegistryForMultibindings.emplace_back(typeInfo, multibindingData);
}

ComponentStorage& ComponentStorage::flushBindings() {
  for (size_t i = 0; i < typeRegistryArray_numUsed; i++) {
    typeRegistry.push_back(typeRegistryArray[i]);
  }
  return *this;
}

} // namespace impl
} // namespace fruit
