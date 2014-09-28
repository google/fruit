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

#ifndef FRUIT_INJECTOR_STORAGE_H
#define FRUIT_INJECTOR_STORAGE_H

#include "../fruit_forward_decls.h"
#include "normalized_component_storage.h"

#include <vector>

namespace fruit {
  
namespace impl {

template <typename T>
struct GetHelper;

/**
 * A component where all types have to be explicitly registered, and all checks are at runtime.
 * Used to implement Component<>, don't use directly.
 * 
 * This class handles the creation of types of the forms:
 * - shared_ptr<C>, [const] C*, [const] C&, C (where C is an atomic type)
 * - Injector<T1, ..., Tk> (with T1, ..., Tk of the above forms).
 */
class InjectorStorage {
private:
  using BindingData = NormalizedComponentStorage::BindingData;
  using BindingDataForMultibinding = NormalizedComponentStorage::BindingDataForMultibinding;
  using BindingDataVectorForMultibinding = NormalizedComponentStorage::BindingDataVectorForMultibinding;
  
  // A chunk of memory used to avoid multiple allocations, since we know all sizes when the injector is created, and the number of used bytes.
  char* singletonStorageBegin = nullptr;
  size_t singletonStorageNumUsedBytes = 0;
  
  // The list of types for which a singleton was created, in order of creation.
  // Allows destruction in the correct order.
  // NOTE: instances provided externally via bindInstance() are not in this vector
  // (the order of destruction for them doesn't matter since none of them depend on
  // other singletons).
  std::vector<const TypeInfo*> createdSingletons;
  
  NormalizedComponentStorage storage;
  
  // If not bound, returns nullptr.
  BindingDataVectorForMultibinding* getBindingDataVectorForMultibinding(const TypeInfo* typeInfo);
  
  template <typename C>
  C* getPtr();
  
  void* getPtr(const TypeInfo* typeInfo);
  
  void* getPtrForMultibinding(const TypeInfo* typeInfo);
  
  // Returns a std::vector<T*>*, or nullptr if there are no multibindings.
  void* getMultibindings(const TypeInfo* typeInfo);
  
  void clear();
  
  // Gets the instance from bindingData, and constructs it if necessary.
  void ensureConstructed(const TypeInfo* typeInfo, BindingData& bindingData);
  
  // Constructs any necessary instances, but NOT the instance set.
  void ensureConstructedMultibinding(fruit::impl::InjectorStorage::BindingDataVectorForMultibinding& bindingDataForMultibinding);
  
  template <typename T>
  friend struct GetHelper;
  
  friend class ComponentStorage;
  
public:
  InjectorStorage(NormalizedComponentStorage&& storage);
  
  InjectorStorage(InjectorStorage&&) = default;
  InjectorStorage& operator=(InjectorStorage&&) = default;
  
  InjectorStorage(const InjectorStorage& other) = delete;
  InjectorStorage& operator=(const InjectorStorage& other) = delete;
  
  ~InjectorStorage();
  
  // When this is called, T and all the types it (recursively) depends on must be bound/registered.
  template <typename T>
  auto get() -> decltype(GetHelper<T>()(*this)) {
    return GetHelper<T>()(*this);
  }
  
  template <typename C, typename... Args>
  C* constructSingleton(Args&&... args);
  
  template <typename C>
  const std::vector<C*>& getMultibindings();
  
  void eagerlyInjectMultibindings();
};

} // namespace impl
} // namespace fruit

#include "injector_storage.templates.h"


#endif // FRUIT_INJECTOR_STORAGE_H
