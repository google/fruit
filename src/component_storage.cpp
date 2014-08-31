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

namespace {
  
std::string multipleBindingsError(const fruit::impl::TypeInfo* typeInfo) {
  return "the type " + typeInfo->name() + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

} // namespace

namespace fruit {
namespace impl {

bool ComponentStorage::BindingData::operator==(const BindingData& other) const {
  // `destroy' is intentionally not compared.
  // If the others are equal it should also be equal. If it isn't, the two BindingData structs
  // are still equivalent because they produce the same injected object.
  return std::tie(      storedSingleton,       create,       createArgument)
      == std::tie(other.storedSingleton, other.create, other.createArgument);
}

bool ComponentStorage::BindingData::operator<(const BindingData& other) const {
  // `destroy' is intentionally not compared.
  // If the others are equal it should also be equal. If it isn't, the two BindingData structs
  // are still equivalent because they produce the same injected object.
  return std::tie(      storedSingleton,       create,       createArgument)
       < std::tie(other.storedSingleton, other.create, other.createArgument);
}

void ComponentStorage::install(ComponentStorage other) {
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.typeRegistry.capacity() > typeRegistry.capacity()) {
    std::swap(typeRegistry, other.typeRegistry);
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
                                         void* (*create)(InjectorStorage&, void*),
                                         void* createArgument,
                                         void (*destroy)(void*)) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::createBindingData for type " << typeInfo->name() << std::endl;
#endif
  BindingData bindingData;
  bindingData.create = create;
  bindingData.createArgument = createArgument;
  bindingData.storedSingleton = nullptr;
  bindingData.destroy = destroy;
  typeRegistry.emplace_back(typeInfo, bindingData);
}

void ComponentStorage::createBindingData(const TypeInfo* typeInfo,
                                         void* storedSingleton,
                                         void (*destroy)(void*)) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::createBindingData for type " << typeInfo->name() << std::endl;
#endif
  BindingData bindingData;
  bindingData.storedSingleton = storedSingleton;
  bindingData.create = nullptr;
  bindingData.createArgument = nullptr;
  bindingData.destroy = destroy;
  typeRegistry.emplace_back(typeInfo, bindingData);
}

void ComponentStorage::createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                                     void* (*create)(InjectorStorage&, void*),
                                                     void* createArgument,
                                                     void (*destroy)(void*),
                                                     std::shared_ptr<char>(*createSet)(InjectorStorage&)) {
  BindingData bindingData;
  bindingData.create = create;
  bindingData.createArgument = createArgument;
  bindingData.storedSingleton = nullptr;
  bindingData.destroy = destroy;
  
  BindingDataForMultibinding bindingDataForMultibinding;
  bindingDataForMultibinding.bindingData = bindingData;
  bindingDataForMultibinding.getSingletonSet = createSet;
  
  typeRegistryForMultibindings.emplace_back(typeInfo, bindingDataForMultibinding);
}

void ComponentStorage::createBindingDataForMultibinding(const TypeInfo* typeInfo,
                                                     void* storedSingleton,
                                                     void (*destroy)(void*),
                                                     std::shared_ptr<char>(*createSet)(InjectorStorage&)) {
  BindingData bindingData;
  bindingData.storedSingleton = storedSingleton;
  bindingData.create = nullptr;
  bindingData.createArgument = nullptr;
  bindingData.destroy = destroy;
  
  BindingDataForMultibinding bindingDataForMultibinding;
  bindingDataForMultibinding.bindingData = bindingData;
  bindingDataForMultibinding.getSingletonSet = createSet;
  
  typeRegistryForMultibindings.emplace_back(typeInfo, bindingDataForMultibinding);
}

void ComponentStorage::printError(const std::string& message) {
  cout << "Fatal injection error: " << message << endl;
}

} // namespace impl
} // namespace fruit
