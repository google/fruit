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
  
std::string multipleBindingsError(fruit::impl::TypeIndex typeIndex) {
  return "the type " + demangleTypeName(typeIndex.name()) + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved components bound this type but didn't expose it in the component signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one component (and then hidden), if this type is auto-injected in another component.\n"
        + "If the source of the problem is unclear, try exposing this type in all the component signatures where it's bound; if no component hides it this can't happen.\n";
}

} // namespace

namespace fruit {
namespace impl {

bool ComponentStorage::TypeInfo::operator<(const TypeInfo& other) const {
  // `destroy' is intentionally not compared.
  // If the others are equal it should also be equal. If it isn't, the two TypeInfo structs
  // are still equivalent because they produce the same injected object.
  return std::tie(      storedSingleton,       create,       createArgument)
       < std::tie(other.storedSingleton, other.create, other.createArgument);
}

void ComponentStorage::ensureConstructed(TypeIndex typeIndex, TypeInfo& typeInfo) {
  if (typeInfo.storedSingleton == nullptr) {
    FruitCheck(bool(typeInfo.create), [=](){return "attempting to create an instance for the type " + demangleTypeName(typeIndex.name()) + " but there is no create operation";});
    typeInfo.storedSingleton = typeInfo.create(*this, typeInfo.createArgument);
    // This can happen if the user-supplied provider returns nullptr.
    check(typeInfo.storedSingleton != nullptr, [=](){return "attempting to get an instance for the type " + demangleTypeName(typeIndex.name()) + " but got nullptr";});
    
    createdSingletons.push_back(typeIndex);
  }
}

void ComponentStorage::ensureConstructedMultibinding(TypeIndex typeIndex, TypeInfoForMultibinding& typeInfoForMultibinding) {
  // When we construct a singleton in a TypeInfo we change the order, so we can't do it for typeInfos already in a set.
  // We need to create a new set.
  std::set<TypeInfo> newTypeInfos;
  for (TypeInfo typeInfo : typeInfoForMultibinding.typeInfos) {
    if (typeInfo.storedSingleton == nullptr) {
      FruitCheck(bool(typeInfo.create), [=](){return "attempting to create an instance for the type " + demangleTypeName(typeIndex.name()) + " but there is no create operation";});
      typeInfo.storedSingleton = typeInfo.create(*this, typeInfo.createArgument);
      // This can happen if the user-supplied provider returns nullptr.
      check(typeInfo.storedSingleton != nullptr, [=](){return "attempting to get an instance for the type " + demangleTypeName(typeIndex.name()) + " but got nullptr";});
    }
    newTypeInfos.insert(typeInfo);
  }
  std::swap(typeInfoForMultibinding.typeInfos, newTypeInfos);
}

void* ComponentStorage::getPtr(TypeIndex typeIndex) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::getPtr(" << demangleTypeName(typeIndex.name()) << ")" << std::endl;
#endif
  auto itr = typeRegistry.find(typeIndex);
  if (itr == typeRegistry.end()) {
    FruitCheck(false, [=](){return "attempting to getPtr() on a non-registered type: " + demangleTypeName(typeIndex.name());});
  }
  ensureConstructed(typeIndex, itr->second);
  return itr->second.storedSingleton;
}

void ComponentStorage::printError(const std::string& message) {
  cout << "Fatal injection error: " << message << endl;
  cout << "Registered types:" << endl;
  for (auto typePair : typeRegistry) {
    std::cout << demangleTypeName(typePair.first.name()) << std::endl;
  }
  for (auto typePair : typeRegistryForMultibindings) {
    if (!typePair.second.typeInfos.empty()) {
      std::cout << demangleTypeName(typePair.first.name()) << " (multibinding)" << std::endl;
    }
  }
  std::cout << std::endl;
}

void ComponentStorage::install(const ComponentStorage& other) {
  FruitCheck(other.createdSingletons.empty(), "Attempting to install a component that has already started creating instances");
  for (const auto& typeInfoPair : other.typeRegistry) {
    TypeIndex typeIndex = typeInfoPair.first;
    const TypeInfo& theirInfo = typeInfoPair.second;
    if (theirInfo.storedSingleton == nullptr) {
      createTypeInfo(typeIndex, theirInfo.create, theirInfo.createArgument, theirInfo.destroy);
    } else {
      createTypeInfo(typeIndex, theirInfo.storedSingleton, theirInfo.destroy);
    }
  }
  for (const auto& typeInfoPair : other.typeRegistryForMultibindings) {
    TypeIndex typeIndex = typeInfoPair.first;
    for (const TypeInfo& theirInfo : typeInfoPair.second.typeInfos) {
      if (theirInfo.storedSingleton == nullptr) {
        createTypeInfoForMultibinding(typeIndex, theirInfo.create, theirInfo.createArgument, theirInfo.destroy, typeInfoPair.second.getSingletonSet);
      } else {
        createTypeInfoForMultibinding(typeIndex, theirInfo.storedSingleton, theirInfo.destroy, typeInfoPair.second.getSingletonSet);
      }
    }
  }
}

void ComponentStorage::clear() {
  // Multibindings can depend on bindings, but not vice-versa and they also can't depend on other multibindings.
  // Delete them in any order.
  for (auto& elem : typeRegistryForMultibindings) {
    std::set<TypeInfo>& typeInfos = elem.second.typeInfos;
    for (const TypeInfo& typeInfo : typeInfos) {
      if (typeInfo.storedSingleton != nullptr) {
        typeInfo.destroy(typeInfo.storedSingleton);
      }
    }
  }
  
  for (auto i = createdSingletons.rbegin(), i_end = createdSingletons.rend(); i != i_end; ++i) {
    std::unordered_map<TypeIndex, TypeInfo>::iterator itr = typeRegistry.find(*i);
    FruitCheck(itr != typeRegistry.end(), "internal error: attempting to destroy an non-registered type");
    TypeInfo& typeInfo = itr->second;
    // Note: if this was a binding or user-provided object, the object is NOT destroyed.
    if (typeInfo.storedSingleton != nullptr) {
      typeInfo.destroy(typeInfo.storedSingleton);
    }
  }
  createdSingletons.clear();
  typeRegistry.clear();
  typeRegistryForMultibindings.clear();
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

void ComponentStorage::createTypeInfo(TypeIndex typeIndex,
                                      void* (*create)(ComponentStorage&, void*),
                                      void* createArgument,
                                      void (*destroy)(void*)) {
#ifdef FRUIT_EXTRA_DEBUG
  std::cerr << "In ComponentStorage::createTypeInfo for type " << demangleTypeName(typeIndex.name()) << std::endl;
#endif
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  auto itr = typeRegistry.find(typeIndex);
  if (itr == typeRegistry.end()) {
    // This type wasn't registered yet, register it.
    TypeInfo& typeInfo = typeRegistry[typeIndex];
    typeInfo.create = create;
    typeInfo.createArgument = createArgument;
    typeInfo.storedSingleton = nullptr;
    typeInfo.destroy = destroy;
  } else {
    // This type was already registered.
    TypeInfo& ourInfo = itr->second;
    check(ourInfo.storedSingleton == nullptr, [=](){ return multipleBindingsError(typeIndex); });
    // At this point it's guaranteed that ourInfo.storedSingleton==nullptr.
    // If the create operations and arguments are equal ok, but if they aren't we
    // can't be sure that they're equivalent and we abort.
    bool equal = ourInfo.create == create
              && ourInfo.createArgument == createArgument;
    check(equal, [=](){ return multipleBindingsError(typeIndex); });
  }
}

void ComponentStorage::createTypeInfo(TypeIndex typeIndex,
                                      void* storedSingleton,
                                      void (*destroy)(void*)) {
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  auto itr = typeRegistry.find(typeIndex);
  if (itr == typeRegistry.end()) {
    // This type wasn't registered yet, register it.
    TypeInfo& typeInfo = typeRegistry[typeIndex];
    typeInfo.storedSingleton = storedSingleton;
    typeInfo.create = nullptr;
    typeInfo.createArgument = nullptr;
    typeInfo.destroy = destroy;
  } else {
    // This type was already registered.
    TypeInfo& ourInfo = itr->second;
    check(ourInfo.storedSingleton != nullptr, [=](){ return multipleBindingsError(typeIndex); });
    // At this point it's guaranteed that ourInfo.storedSingleton!=nullptr.
    // If the stored singletons and destroy operations are equal ok, but if they aren't we
    // can't be sure that they're equivalent and we abort.
    bool equal = ourInfo.storedSingleton == storedSingleton;
    check(equal, [=](){ return multipleBindingsError(typeIndex); });
  }
}

void ComponentStorage::createTypeInfoForMultibinding(TypeIndex typeIndex,
                                                     void* (*create)(ComponentStorage&, void*),
                                                     void* createArgument,
                                                     void (*destroy)(void*),
                                                     std::shared_ptr<char>(*createSet)(ComponentStorage&)) {
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  
  TypeInfo typeInfo;
  typeInfo.create = create;
  typeInfo.createArgument = createArgument;
  typeInfo.storedSingleton = nullptr;
  typeInfo.destroy = destroy;
  
  // This works no matter whether typeIndex was registered as a multibinding before or not.
  TypeInfoForMultibinding& typeInfoForMultibinding = typeRegistryForMultibindings[typeIndex];
  check(typeInfoForMultibinding.s.get() == nullptr,
        [](){return "Attempting to add a multibinding after retrieving multibindings for the same type.";});
  typeInfoForMultibinding.typeInfos.insert(typeInfo);
  typeInfoForMultibinding.getSingletonSet = createSet;
}

void ComponentStorage::createTypeInfoForMultibinding(TypeIndex typeIndex,
                                                     void* storedSingleton,
                                                     void (*destroy)(void*),
                                                     std::shared_ptr<char>(*createSet)(ComponentStorage&)) {
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  
  TypeInfo typeInfo;
  typeInfo.storedSingleton = storedSingleton;
  typeInfo.create = nullptr;
  typeInfo.createArgument = nullptr;
  typeInfo.destroy = destroy;
  
  TypeInfoForMultibinding& typeInfoForMultibinding = typeRegistryForMultibindings[typeIndex];
  check(typeInfoForMultibinding.s.get() == nullptr,
        [](){return "Attempting to add a multibinding after retrieving multibindings for the same type.";});
  typeInfoForMultibinding.typeInfos.insert(typeInfo);
  typeInfoForMultibinding.getSingletonSet = createSet;
}

void* ComponentStorage::getMultibindings(TypeIndex typeIndex) {
  auto itr = typeRegistryForMultibindings.find(typeIndex);
  if (itr == typeRegistryForMultibindings.end()) {
    // Not registered.
    return nullptr;
  }
  return itr->second.getSingletonSet(*this).get();
}

void ComponentStorage::eagerlyInjectMultibindings() {
  for (auto typeIndexInfoPair : typeRegistryForMultibindings) {
    typeIndexInfoPair.second.getSingletonSet(*this);
  }
}

} // namespace impl
} // namespace fruit
