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
#include "fruit/impl/metaprogramming.h"
#include "fruit/impl/demangle_type_name.h"
#include "fruit/impl/type_info.h"

#include "fruit/impl/component_storage.h"
#include "fruit/impl/component_storage.templates.h"

using std::cout;
using std::endl;

namespace fruit {
namespace impl {

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
                      std::make_move_iterator(other.typeRegistry.begin()),
                      std::make_move_iterator(other.typeRegistry.end()));
  
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.typeRegistryForMultibindings.capacity() > typeRegistryForMultibindings.capacity()) {
    std::swap(typeRegistryForMultibindings, other.typeRegistryForMultibindings);
  }
  typeRegistryForMultibindings.insert(typeRegistryForMultibindings.end(),
                                      std::make_move_iterator(other.typeRegistryForMultibindings.begin()),
                                      std::make_move_iterator(other.typeRegistryForMultibindings.end()));
}

void ComponentStorage::createBindingData(const TypeInfo* typeInfo,
                                         BindingData::create_t create,
                                         BindingData::createArgument_t createArgument) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::createBindingData for type " << typeInfo->name() << std::endl;
#endif
  auto x = std::make_pair(typeInfo, BindingData(create, createArgument));
  if (typeRegistryArray_numUsed < max_num_immediate_bindings) {
    typeRegistryArray[typeRegistryArray_numUsed] = x;
    ++typeRegistryArray_numUsed;
  } else {
    typeRegistry.push_back(x);
  }
}

void ComponentStorage::createBindingData(const TypeInfo* typeInfo,
                                         BindingData::object_t storedSingleton,
                                         BindingData::destroy_t destroy) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::createBindingData for type " << typeInfo->name() << std::endl;
#endif
  auto x = std::make_pair(typeInfo, BindingData(destroy, storedSingleton));
  if (typeRegistryArray_numUsed < max_num_immediate_bindings) {
    typeRegistryArray[typeRegistryArray_numUsed] = x;
    ++typeRegistryArray_numUsed;
  } else {
    typeRegistry.push_back(x);
  }
}

void ComponentStorage::createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                                        BindingData::create_t create,
                                                        BindingData::createArgument_t createArgument,
                                                        std::shared_ptr<char>(*createSet)(InjectorStorage&)) {
  BindingDataForMultibinding bindingDataForMultibinding;
  bindingDataForMultibinding.bindingData = BindingData(create, createArgument);
  bindingDataForMultibinding.getSingletonSet = createSet;
  
  typeRegistryForMultibindings.emplace_back(typeInfo, bindingDataForMultibinding);
}

void ComponentStorage::createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                                        BindingData::object_t storedSingleton,
                                                        BindingData::destroy_t destroy,
                                                        std::shared_ptr<char>(*createSet)(InjectorStorage&)) {
  BindingDataForMultibinding bindingDataForMultibinding;
  bindingDataForMultibinding.bindingData = BindingData(destroy, storedSingleton);
  bindingDataForMultibinding.getSingletonSet = createSet;
  
  typeRegistryForMultibindings.emplace_back(typeInfo, bindingDataForMultibinding);
}

void ComponentStorage::printError(const std::string& message) {
  cout << "Fatal injection error: " << message << endl;
}

} // namespace impl
} // namespace fruit
