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

#include "../../fruit_forward_decls.h"
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
public:  
  using BindingVectors = std::pair<std::vector<std::pair<TypeId, BindingData>>,
                                   std::vector<std::pair<TypeId, MultibindingData>>>;
  using Graph = SemistaticGraph<TypeId, NormalizedBindingData>;
  
private:
  // A chunk of memory used to avoid multiple allocations, since we know all sizes when the injector is created, and the number of used bytes.
  char* singletonStorageBegin = nullptr;
  // A pointer to the last used byte in the allocated memory chunk starting at singletonStorageBegin.
  char* singletonStorageLastUsed = nullptr;
  
  // The list of destroy operation for created singletons, in order of creation.
  // Allows destruction in the correct order.
  // These must be called in reverse order.
  std::vector<BindingData::destroy_t> onDestruction;
  
  // A graph with types as nodes (each node stores the BindingData for the type) and dependencies as edges.
  // For types that have a constructed object already, the corresponding node is stored as terminal node.
  SemistaticGraph<TypeId, NormalizedBindingData> typeRegistry;
  
  // Maps the type index of a type T to a set of the corresponding BindingData objects (for multibindings).
  std::unordered_map<TypeId, NormalizedMultibindingData> typeRegistryForMultibindings;
  
private:
  
  // If not bound, returns nullptr.
  NormalizedMultibindingData* getNormalizedMultibindingData(TypeId typeInfo);
  
  template <typename C>
  C* getPtr();
  
  // Similar to the previous, but takes an dep vector + index. Use this when the node_iterator is known, it's faster.
  template <typename C>
  C* getPtr(NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index);
  
  void* getPtr(TypeId typeInfo);
  // Similar to the previous, but takes a node_iterator. Use this when the node_iterator is known, it's faster.
  void* getPtr(NormalizedComponentStorage::Graph::node_iterator itr);
  
  // Similar to getPtr, but the binding might not exist. Returns nullptr if it doesn't.
  void* unsafeGetPtr(TypeId typeInfo);
  
  void* getPtrForMultibinding(TypeId typeInfo);
  
  // Returns a std::vector<T*>*, or nullptr if there are no multibindings.
  void* getMultibindings(TypeId typeInfo);
  
  void clear();
  
  // Gets the instance from BindingData, and constructs it if necessary.
  void ensureConstructed(typename SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator nodeItr);
  
  // Constructs any necessary instances, but NOT the instance set.
  void ensureConstructedMultibinding(NormalizedMultibindingData& multibindingData);
  
  template <typename T>
  friend struct GetHelper;
  
  friend class ComponentStorage;
  
public:
  InjectorStorage(NormalizedComponentStorage&& storage);
  
  InjectorStorage(const NormalizedComponentStorage& normalizedStorage, ComponentStorage&& storage);
  
  InjectorStorage(InjectorStorage&&) = default;
  InjectorStorage& operator=(InjectorStorage&&) = default;
  
  InjectorStorage(const InjectorStorage& other) = delete;
  InjectorStorage& operator=(const InjectorStorage& other) = delete;
  
  ~InjectorStorage();
  
  static std::size_t maximumRequiredSpace(fruit::impl::TypeId typeId);
  
  static void normalizeTypeRegistryVector(std::vector<std::pair<TypeId, BindingData>>& typeRegistryVector);
  
  static void addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingData>& typeRegistryForMultibindings,
                               std::size_t& total_size,
                               std::vector<std::pair<TypeId, MultibindingData>>&& typeRegistryVectorForMultibindings);

  
  // When this is called, T and all the types it (recursively) depends on must be bound/registered.
  template <typename T>
  auto get() -> decltype(GetHelper<T>()(*this)) {
    return GetHelper<T>()(*this);
  }
  
  // Similar to the above, but specifying the node_iterator of the type. Use this when the node_iterator is known, it's faster.
  // dep_index is the index of the dep in `deps'.
  template <typename T>
  auto get(NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) -> decltype(GetHelper<T>()(*this, deps, dep_index)) {
    return GetHelper<T>()(*this, deps, dep_index);
  }
  
  // Returns nullptr if C was not bound.
  template <typename C>
  C* unsafeGet();
  
  template <typename C, typename... Args>
  C* constructSingleton(Args&&... args);
  
  // Destroys a singleton previously created using constructSingleton().
  // Can only be used on destruction, in particular no further calls to constructSingleton are allowed after calling this.
  template <typename C>
  static void destroySingleton(InjectorStorage& storage);
  
  // Calls delete on a singleton previously allocated using new.
  // Can only be used on destruction, in particular no further calls to constructSingleton are allowed after calling this.
  template <typename C>
  static void destroyExternalSingleton(InjectorStorage& storage);
  
  template <typename C>
  const std::vector<C*>& getMultibindings();
  
  void eagerlyInjectMultibindings();
};

} // namespace impl
} // namespace fruit

#include "injector_storage.defn.h"


#endif // FRUIT_INJECTOR_STORAGE_H
