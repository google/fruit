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

#include "fruit/impl/unsafe_module.h"

using std::cout;
using std::endl;

namespace {
  
std::string multipleBindingsError(fruit::impl::TypeIndex typeIndex) {
  return "the type " + demangleTypeName(typeIndex.name()) + " was provided more than once, with different bindings.\n"
        + "This was not caught at compile time because at least one of the involved modules bound this type but didn't expose it in the module signature.\n"
        + "If the type has a default constructor or an Inject annotation, this problem may arise even if this type is bound/provided by only one module (and then hidden), if this type is auto-injected in another module.\n"
        + "If the source of the problem is unclear, try exposing this type in all the module signatures where it's bound; if no module hides it this can't happen.\n";
}

} // namespace

namespace fruit {
namespace impl {

void* UnsafeModule::getPtr(TypeIndex typeIndex) {
  auto itr = typeRegistry.find(typeIndex);
  FruitCheck(itr != typeRegistry.end(), [=](){return "attempting to getPtr() on a non-registered type: " + demangleTypeName(typeIndex.name());});
  TypeInfo& typeInfo = itr->second;
  if (typeInfo.storedSingleton == nullptr) {
    FruitCheck(bool(typeInfo.create), [=](){return "attempting to create an instance for the type " + demangleTypeName(typeIndex.name()) + " but there is no create operation";});
    std::tie(typeInfo.storedSingleton, typeInfo.destroy) = typeInfo.create(*this, typeInfo.createArgument);
    createdSingletons.push_back(typeIndex);
  }
  void* p = typeInfo.storedSingleton;
  // This can happen if the user-supplied provider returns nullptr.
  check(p != nullptr, [=](){return "attempting to get an instance for the type " + demangleTypeName(typeIndex.name()) + " but got nullptr";});
  return p;
}

void UnsafeModule::printError(const std::string& message) {
  cout << "Fatal injection error: " << message << endl;
  cout << "Registered types:" << endl;
  for (auto typePair : typeRegistry) {
    std::cout << demangleTypeName(typePair.first.name()) << std::endl;
  }
}

void UnsafeModule::install(const UnsafeModule& other) {
  FruitCheck(other.createdSingletons.empty(), "Attempting to install a module that has already started creating instances");
  for (const auto& typeInfoPair : other.typeRegistry) {
    TypeIndex typeIndex = typeInfoPair.first;
    const TypeInfo& theirInfo = typeInfoPair.second;
    if (theirInfo.storedSingleton == nullptr) {
      createTypeInfo(typeIndex, theirInfo.create, theirInfo.createArgument);
    } else {
      createTypeInfo(typeIndex, theirInfo.storedSingleton, theirInfo.destroy);
    }
  }
}

void UnsafeModule::clear() {
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
}

UnsafeModule::~UnsafeModule() {
  clear();
}

UnsafeModule& UnsafeModule::operator=(const UnsafeModule& other) {
  // Can't copy the module once it starts owning resources (singleton instances).
  FruitCheck(other.createdSingletons.empty(), "Attempting to copy a module that has already started creating instances");
  UnsafeModule tmp(other);
  swap(tmp);
  return *this;
}

UnsafeModule& UnsafeModule::operator=(UnsafeModule&& other) {
  swap(other);
  return *this;
}

void UnsafeModule::createTypeInfo(TypeIndex typeIndex, 
                                  std::pair<void*, void(*)(void*)> (*create)(UnsafeModule&, void*), 
                                  void* createArgument) {
  auto itr = typeRegistry.find(typeIndex);
  if (itr == typeRegistry.end()) {
    // This type wasn't registered yet, register it.
    TypeInfo& typeInfo = typeRegistry[typeIndex];
    typeInfo.create = create;
    typeInfo.createArgument = createArgument;
    typeInfo.storedSingleton = nullptr;
  } else {
    // This type was already registered.
    TypeInfo& ourInfo = itr->second;
    check(ourInfo.storedSingleton == nullptr, [=](){ return multipleBindingsError(typeIndex); });
    // At this point it's guaranteed that ourInfo.storedSingleton==nullptr.
    // If the create operations and arguments are equal ok, but if they aren't we
    // can't be sure that they're equivalent and we abort.
    bool equal = ourInfo.create == create && ourInfo.createArgument == createArgument;
    check(equal, [=](){ return multipleBindingsError(typeIndex); });
  }
}

void UnsafeModule::createTypeInfo(TypeIndex typeIndex, 
                                  void* storedSingleton, 
                                  void (*destroy)(void*)) {
  auto itr = typeRegistry.find(typeIndex);
  if (itr == typeRegistry.end()) {
    // This type wasn't registered yet, register it.
    TypeInfo& typeInfo = typeRegistry[typeIndex];
    typeInfo.storedSingleton = storedSingleton;
    typeInfo.destroy = destroy;
  } else {
    // This type was already registered.
    TypeInfo& ourInfo = itr->second;
    check(ourInfo.storedSingleton != nullptr, [=](){ return multipleBindingsError(typeIndex); });
    // At this point it's guaranteed that ourInfo.storedSingleton!=nullptr.
    // If the stored singletons and destroy operations are equal ok, but if they aren't we
    // can't be sure that they're equivalent and we abort.
    bool equal = ourInfo.storedSingleton == storedSingleton
      && ourInfo.destroy == destroy;
    check(equal, [=](){ return multipleBindingsError(typeIndex); });
  }
}

} // namespace impl
} // namespace fruit
