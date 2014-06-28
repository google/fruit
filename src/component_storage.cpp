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

void* ComponentStorage::getPtr(TypeIndex typeIndex) {
  for (ComponentStorage* storage = this; storage != nullptr; storage = storage->parent) {
    auto itr = storage->typeRegistry.find(typeIndex);
    if (itr == storage->typeRegistry.end()) {
      // Not registered here, try the parents (if any).
      continue;
    }
    TypeInfo& typeInfo = itr->second;
    if (typeInfo.storedSingleton == nullptr) {
      FruitCheck(bool(typeInfo.create), [=](){return "attempting to create an instance for the type " + demangleTypeName(typeIndex.name()) + " but there is no create operation";});
      typeInfo.storedSingleton = typeInfo.create(*this, typeInfo.createArgument);
      storage->createdSingletons.push_back(typeIndex);
    }
    void* p = typeInfo.storedSingleton;
    // This can happen if the user-supplied provider returns nullptr.
    check(p != nullptr, [=](){return "attempting to get an instance for the type " + demangleTypeName(typeIndex.name()) + " but got nullptr";});
    return p;
  }
  FruitCheck(false, [=](){return "attempting to getPtr() on a non-registered type: " + demangleTypeName(typeIndex.name());});
  // Never executed.
  return nullptr;
}

void ComponentStorage::printError(const std::string& message) {
  cout << "Fatal injection error: " << message << endl;
  cout << "Registered types:" << endl;
  for (auto typePair : typeRegistry) {
    std::cout << demangleTypeName(typePair.first.name()) << std::endl;
  }
  std::cout << std::endl;
  if (parent != nullptr) {
    for (ComponentStorage* storage = parent; storage != nullptr; storage = storage->parent) {
      cout << "Registered types in parent injector:" << endl;
      for (auto typePair : storage->typeRegistry) {
        std::cout << demangleTypeName(typePair.first.name()) << std::endl;
      }
      std::cout << std::endl;
    }
  }
}

void ComponentStorage::install(const ComponentStorage& other) {
  FruitCheck(other.createdSingletons.empty(), "Attempting to install a component that has already started creating instances");
  FruitCheck(other.parent == nullptr, "Attempting to install a component that has already started creating instances");
  FruitCheck(parent == nullptr, "Attempting to install a component after calling setParent().");
  for (const auto& typeInfoPair : other.typeRegistry) {
    TypeIndex typeIndex = typeInfoPair.first;
    const TypeInfo& theirInfo = typeInfoPair.second;
    if (theirInfo.storedSingleton == nullptr) {
      createTypeInfo(typeIndex, theirInfo.create, theirInfo.createArgument, theirInfo.destroy);
    } else {
      createTypeInfo(typeIndex, theirInfo.storedSingleton, theirInfo.destroy);
    }
  }
}

void ComponentStorage::clear() {
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
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  FruitCheck(parent == nullptr, "Attempting to add a binding after calling setParent().");
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
  FruitCheck(parent == nullptr, "Attempting to add a binding after calling setParent().");
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

void ComponentStorage::setParent(ComponentStorage* parent) {
  FruitCheck(createdSingletons.empty(), "Attempting to add a binding to a component that has already started creating instances");
  this->parent = parent;
  // If the same type is bound in a parent and the child, ensure that the bindings are equivalent and
  // remove the bound in the child.
  for (ComponentStorage* p = parent; p != nullptr; p = p->parent) {
    for (auto i = typeRegistry.cbegin(), i_end = typeRegistry.cend(); i != i_end; /* no increment */) {
      TypeIndex typeIndex = i->first;
      const TypeInfo& childInfo = i->second;
      auto itr = p->typeRegistry.find(typeIndex);
      if (itr == p->typeRegistry.end()) {
        // Type not bound in parent, ok.
        ++i;
        continue;
      }
      const TypeInfo& parentInfo = itr->second;
      
      // This type was already registered.
      
      bool equal;
      if (childInfo.storedSingleton != nullptr) {
        // Instance binding.
        equal = childInfo.storedSingleton == parentInfo.storedSingleton;
      } else {
        // Note that parentInfo.storedSingleton may or may not be nullptr.
        equal = childInfo.create == parentInfo.create
            && childInfo.createArgument == parentInfo.createArgument;
      }
      
      check(equal, [=](){ return multipleBindingsError(typeIndex); });
      
      i = typeRegistry.erase(i);
    }
  }
}

} // namespace impl
} // namespace fruit
