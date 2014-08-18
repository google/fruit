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

void ComponentStorage::ensureConstructed(const TypeInfo* typeInfo, BindingData& bindingData) {
  if (bindingData.storedSingleton == nullptr) {
    FruitCheck(bool(bindingData.create), [=](){return "attempting to create an instance for the type " + typeInfo->name() + " but there is no create operation";});
    bindingData.storedSingleton = bindingData.create(*this, bindingData.createArgument);
    // This can happen if the user-supplied provider returns nullptr.
    check(bindingData.storedSingleton != nullptr, [=](){return "attempting to get an instance for the type " + typeInfo->name() + " but got nullptr";});
    
    createdSingletons.push_back(typeInfo);
  }
}

void ComponentStorage::ensureConstructedMultibinding(const TypeInfo* typeInfo, BindingDataForMultibinding& bindingDataForMultibinding) {
  // When we construct a singleton in a BindingData we change the order, so we can't do it for bindingDatas already in a set.
  // We need to create a new set.
  std::set<BindingData> newBindingDatas;
  for (BindingData bindingData : bindingDataForMultibinding.bindingDatas) {
    if (bindingData.storedSingleton == nullptr) {
      FruitCheck(bool(bindingData.create), [=](){return "attempting to create an instance for the type " + typeInfo->name() + " but there is no create operation";});
      bindingData.storedSingleton = bindingData.create(*this, bindingData.createArgument);
      // This can happen if the user-supplied provider returns nullptr.
      check(bindingData.storedSingleton != nullptr, [=](){return "attempting to get an instance for the type " + typeInfo->name() + " but got nullptr";});
    }
    newBindingDatas.insert(bindingData);
  }
  std::swap(bindingDataForMultibinding.bindingDatas, newBindingDatas);
}

void* ComponentStorage::getPtr(const TypeInfo* typeInfo) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::getPtr(" << typeInfo->name() << ")" << std::endl;
#endif
  auto itr = typeRegistry.find(typeInfo);
  if (itr == typeRegistry.end()) {
    FruitCheck(false, [=](){return "attempting to getPtr() on a non-registered type: " + typeInfo->name();});
  }
  ensureConstructed(typeInfo, itr->second);
  return itr->second.storedSingleton;
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
  FruitCheck(other.createdSingletons.empty(), "Attempting to install a component that has already started creating instances");
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

void ComponentStorage::clear() {
  // Multibindings can depend on bindings, but not vice-versa and they also can't depend on other multibindings.
  // Delete them in any order.
  for (auto& elem : typeRegistryForMultibindings) {
    std::set<BindingData>& bindingDatas = elem.second.bindingDatas;
    for (const BindingData& bindingData : bindingDatas) {
      if (bindingData.storedSingleton != nullptr) {
        bindingData.destroy(bindingData.storedSingleton);
      }
    }
  }
  
  for (auto i = createdSingletons.rbegin(), i_end = createdSingletons.rend(); i != i_end; ++i) {
    std::unordered_map<const TypeInfo*, BindingData>::iterator itr = typeRegistry.find(*i);
    FruitCheck(itr != typeRegistry.end(), "internal error: attempting to destroy an non-registered type");
    BindingData& bindingData = itr->second;
    // Note: if this was a binding or user-provided object, the object is NOT destroyed.
    if (bindingData.storedSingleton != nullptr) {
      bindingData.destroy(bindingData.storedSingleton);
    }
  }
  createdSingletons.clear();
  typeRegistry.clear();
  typeRegistryForMultibindings.clear();
  if (singletonStorageBegin != nullptr) {
    delete [] singletonStorageBegin;
  }
}

ComponentStorage::~ComponentStorage() {
  clear();
}

ComponentStorage& ComponentStorage::operator=(const ComponentStorage& other) {
  // Can't copy the component once it starts owning resources (singleton instances).
  FruitCheck(other.createdSingletons.empty(), "Attempting to copy a component that has already started creating instances");
  ComponentStorage tmp(other);
  swap(tmp);
  return *this;
}

ComponentStorage& ComponentStorage::operator=(ComponentStorage&& other) {
  swap(other);
  return *this;
}

void ComponentStorage::createBindingData(const TypeInfo* typeInfo,
                                      void* (*create)(ComponentStorage&, void*),
                                      void* createArgument,
                                      void (*destroy)(void*)) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::createBindingData for type " << typeInfo->name() << std::endl;
#endif
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
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
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
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
                                                     void* (*create)(ComponentStorage&, void*),
                                                     void* createArgument,
                                                     void (*destroy)(void*),
                                                     std::shared_ptr<char>(*createSet)(ComponentStorage&)) {
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  
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
                                                     std::shared_ptr<char>(*createSet)(ComponentStorage&)) {
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  
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

void ComponentStorage::becomeInjector() {
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


void* ComponentStorage::getMultibindings(const TypeInfo* typeInfo) {
  auto itr = typeRegistryForMultibindings.find(typeInfo);
  if (itr == typeRegistryForMultibindings.end()) {
    // Not registered.
    return nullptr;
  }
  return itr->second.getSingletonSet(*this).get();
}

void ComponentStorage::eagerlyInjectMultibindings() {
  for (auto& typeInfoInfoPair : typeRegistryForMultibindings) {
    typeInfoInfoPair.second.getSingletonSet(*this);
  }
}

} // namespace impl
} // namespace fruit
