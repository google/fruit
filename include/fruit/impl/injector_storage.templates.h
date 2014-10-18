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

#ifndef FRUIT_INJECTOR_STORAGE_TEMPLATES_H
#define FRUIT_INJECTOR_STORAGE_TEMPLATES_H

#include "demangle_type_name.h"
#include "type_info.h"
#include "fruit_assert.h"

// Redundant, but makes KDevelop happy.
#include "injector_storage.h"

namespace fruit {
namespace impl {

template <typename AnnotatedSignature>
struct BindAssistedFactory;

// General case, value
template <typename C>
struct GetHelper {
  C operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<const C> {
  const C operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<std::shared_ptr<C>> {
  std::shared_ptr<C> operator()(InjectorStorage& injector) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), injector.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<C*> {
  C* operator()(InjectorStorage& injector) {
    return injector.getPtr<C>();
  }
};

template <typename C>
struct GetHelper<const C*> {
  const C* operator()(InjectorStorage& injector) {
    return injector.getPtr<C>();
  }
};

template <typename C>
struct GetHelper<C&> {
  C& operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<const C&> {
  const C& operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
};

template <typename... Ps>
struct GetHelper<Provider<Ps...>> {
  Provider<Ps...> operator()(InjectorStorage& storage) {
    return Provider<Ps...>(&storage);
  }
};

template <typename C>
inline C* InjectorStorage::getPtr() {
  void* p = getPtr(getTypeInfo<C>());
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::unsafeGet() {
  void* p = unsafeGetPtr(getTypeInfo<C>());
  return reinterpret_cast<C*>(p);
}

inline void* InjectorStorage::getPtr(const TypeInfo* typeInfo) {
  BindingData& bindingData = storage.typeRegistry.at(typeInfo);
  ensureConstructed(bindingData);
  return bindingData.getStoredSingleton();
}

inline void* InjectorStorage::unsafeGetPtr(const TypeInfo* typeInfo) {
  BindingData* bindingData = storage.typeRegistry.find(typeInfo);
  if (bindingData == nullptr) {
    return nullptr;
  }
  ensureConstructed(*bindingData);
  return bindingData->getStoredSingleton();
}

template <typename C, typename... Args>
inline C* InjectorStorage::constructSingleton(Args&&... args) {
  size_t misalignment = (singletonStorageNumUsedBytes % alignof(C));
  if (misalignment != 0) {
    singletonStorageNumUsedBytes += alignof(C) - misalignment;
  }
  C* c = reinterpret_cast<C*>(singletonStorageBegin + singletonStorageNumUsedBytes);
  new (c) C(std::forward<Args>(args)...);
  singletonStorageNumUsedBytes += sizeof(C);
  return c;
}

template <typename C>
void InjectorStorage::destroySingleton(InjectorStorage& storage) {
  C* cPtr = storage.getPtr<C>();
  cPtr->C::~C();
}

template <typename C>
void InjectorStorage::destroyExternalSingleton(InjectorStorage& storage) {
  C* cPtr = storage.getPtr<C>();
  delete cPtr;
}

template <typename C>
inline const std::vector<C*>& InjectorStorage::getMultibindings() {
  void* p = getMultibindings(getTypeInfo<C>());
  if (p == nullptr) {
    static std::vector<C*> empty_vector;
    return empty_vector;
  } else {
    return *reinterpret_cast<std::vector<C*>*>(p);
  }
}

inline void InjectorStorage::ensureConstructed(BindingData& bindingData) {
  if (!bindingData.isCreated()) {
    BindingData::destroy_t destroy = bindingData.create(*this);
    if (destroy != nullptr) {
      onDestruction.push_back(destroy);
    }
  }
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_INJECTOR_STORAGE_TEMPLATES_H
