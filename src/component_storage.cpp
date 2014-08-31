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

bool ComponentStorage::BindingData::operator<(const BindingData& other) const {
  // `destroy' is intentionally not compared.
  // If the others are equal it should also be equal. If it isn't, the two BindingData structs
  // are still equivalent because they produce the same injected object.
  return std::tie(      storedSingleton,       create,       createArgument)
       < std::tie(other.storedSingleton, other.create, other.createArgument);
}

void ComponentStorage::printError(const std::string& message) {
  cout << "Fatal injection error: " << message << endl;
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

void ComponentStorage::install(const ComponentStorage& other) {
  for (const auto& bindingDataPair : other.typeRegistry) {
    const TypeInfo* typeInfo = bindingDataPair.first;
    const BindingData& theirInfo = bindingDataPair.second;
    if (theirInfo.storedSingleton == nullptr) {
      createBindingData(typeInfo, theirInfo.create, theirInfo.createArgument, theirInfo.destroy);
    } else {
      createBindingData(typeInfo, theirInfo.storedSingleton, theirInfo.destroy);
    }
  }
  for (const auto& bindingDataPair : other.typeRegistryForMultibindings) {
    const TypeInfo* typeInfo = bindingDataPair.first;
    for (const BindingData& theirInfo : bindingDataPair.second.bindingDatas) {
      if (theirInfo.storedSingleton == nullptr) {
        createBindingDataForMultibinding(typeInfo, theirInfo.create, theirInfo.createArgument, theirInfo.destroy, bindingDataPair.second.getSingletonSet);
      } else {
        createBindingDataForMultibinding(typeInfo, theirInfo.storedSingleton, theirInfo.destroy, bindingDataPair.second.getSingletonSet);
      }
    }
  }
}

void ComponentStorage::createBindingData(const TypeInfo* typeInfo,
                                      void* (*create)(InjectorStorage&, void*),
                                      void* createArgument,
                                      void (*destroy)(void*)) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::createBindingData for type " << typeInfo->name() << std::endl;
#endif
  auto itr = typeRegistry.find(typeInfo);
  if (itr == typeRegistry.end()) {
    // This type wasn't registered yet, register it.
    BindingData& bindingData = typeRegistry[typeInfo];
    bindingData.create = create;
    bindingData.createArgument = createArgument;
    bindingData.storedSingleton = nullptr;
    bindingData.destroy = destroy;
  } else {
    // This type was already registered.
    BindingData& ourInfo = itr->second;
    check(ourInfo.storedSingleton == nullptr, [=](){ return multipleBindingsError(typeInfo); });
    // At this point it's guaranteed that ourInfo.storedSingleton==nullptr.
    // If the create operations and arguments are equal ok, but if they aren't we
    // can't be sure that they're equivalent and we abort.
    bool equal = ourInfo.create == create
              && ourInfo.createArgument == createArgument;
    check(equal, [=](){ return multipleBindingsError(typeInfo); });
  }
}

void ComponentStorage::createBindingData(const TypeInfo* typeInfo,
                                      void* storedSingleton,
                                      void (*destroy)(void*)) {
  auto itr = typeRegistry.find(typeInfo);
  if (itr == typeRegistry.end()) {
    // This type wasn't registered yet, register it.
    BindingData& bindingData = typeRegistry[typeInfo];
    bindingData.storedSingleton = storedSingleton;
    bindingData.create = nullptr;
    bindingData.createArgument = nullptr;
    bindingData.destroy = destroy;
  } else {
    // This type was already registered.
    BindingData& ourInfo = itr->second;
    check(ourInfo.storedSingleton != nullptr, [=](){ return multipleBindingsError(typeInfo); });
    // At this point it's guaranteed that ourInfo.storedSingleton!=nullptr.
    // If the stored singletons and destroy operations are equal ok, but if they aren't we
    // can't be sure that they're equivalent and we abort.
    bool equal = ourInfo.storedSingleton == storedSingleton;
    check(equal, [=](){ return multipleBindingsError(typeInfo); });
  }
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
  
  // This works no matter whether typeInfo was registered as a multibinding before or not.
  BindingDataForMultibinding& bindingDataForMultibinding = typeRegistryForMultibindings[typeInfo];
  check(bindingDataForMultibinding.s.get() == nullptr,
        [](){return "Attempting to add a multibinding after retrieving multibindings for the same type.";});
  bindingDataForMultibinding.bindingDatas.insert(bindingData);
  bindingDataForMultibinding.getSingletonSet = createSet;
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
  
  BindingDataForMultibinding& bindingDataForMultibinding = typeRegistryForMultibindings[typeInfo];
  check(bindingDataForMultibinding.s.get() == nullptr,
        [](){return "Attempting to add a multibinding after retrieving multibindings for the same type.";});
  bindingDataForMultibinding.bindingDatas.insert(bindingData);
  bindingDataForMultibinding.getSingletonSet = createSet;
}

} // namespace impl
} // namespace fruit
