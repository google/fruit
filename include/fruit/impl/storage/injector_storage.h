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
#include "fixed_size_allocator.h"

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
  
  // Prints the specified error and calls exit(1).
  static void fatal(const std::string& error);
  
  // Returns a tuple (getTypeId<I>(), bindingData)
  template <typename I, typename C>
  static std::tuple<TypeId, BindingData> createBindingDataForBind();

  // Returns a tuple (getTypeId<C>(), bindingData)
  template <typename C>
  static std::tuple<TypeId, BindingData> createBindingDataForBindInstance(C& instance);

  // Returns a tuple (getTypeId<C>(), bindingData)
  template <typename Lambda>
  static std::tuple<TypeId, BindingData> createBindingDataForProvider();

  // Returns a tuple (getTypeId<I>(), getTypeId<C>(), bindingData)
  template <typename Lambda, typename I>
  static std::tuple<TypeId, TypeId, BindingData> createBindingDataForCompressedProvider();

  // Returns a tuple (getTypeId<C>(), bindingData)
  template <typename Signature>
  static std::tuple<TypeId, BindingData> createBindingDataForConstructor();

  // Returns a tuple (getTypeId<I>(), getTypeId<C>(), bindingData)
  template <typename Signature, typename I>
  static std::tuple<TypeId, TypeId, BindingData> createBindingDataForCompressedConstructor();

  // Returns a tuple (getTypeId<InjectedFunction>(), bindingData)
  template <typename AnnotatedSignature, typename Lambda>
  static std::tuple<TypeId, BindingData> createBindingDataForFactory();

  // Returns a tuple (getTypeId<I>(), bindingData)
  template <typename I, typename C>
  static std::tuple<TypeId, MultibindingData> createMultibindingDataForBinding();

  // Returns a tuple (getTypeId<C>(), bindingData)
  template <typename C>
  static std::tuple<TypeId, MultibindingData> createMultibindingDataForInstance(C& instance);

  // Returns a tuple (getTypeId<C>(), multibindingData)
  template <typename Lambda>
  static std::tuple<TypeId, MultibindingData> createMultibindingDataForProvider();

private:
  // The NormalizedComponentStorage owned by this object (if any).
  // Only used for the 1-argument constructor, otherwise it's nullptr.
  std::unique_ptr<NormalizedComponentStorage> normalized_component_storage_ptr;
  
  FixedSizeAllocator allocator;
  
  // The list of destroy operation for created objects, in order of creation, and the pointers that they must be invoked with.
  // Allows destruction in the correct order.
  // These must be called in reverse order.
  std::vector<std::pair<BindingData::destroy_t, void*>> on_destruction;
  
  // A graph with injected types as nodes (each node stores the NormalizedBindingData for the type) and dependencies as edges.
  // For types that have a constructed object already, the corresponding node is stored as terminal node.
  SemistaticGraph<TypeId, NormalizedBindingData> bindings;
  
  // Maps the type index of a type T to the corresponding NormalizedMultibindingData object (that stores all multibindings).
  std::unordered_map<TypeId, NormalizedMultibindingData> multibindings;
  
private:
  
  template <typename C>
  static std::shared_ptr<char> createMultibindingVector(InjectorStorage& storage);
  
  // If not bound, returns nullptr.
  NormalizedMultibindingData* getNormalizedMultibindingData(TypeId type);
  
  // Looks up the location where the type is (or will be) stored, but does not construct the class.
  template <typename C>
  Graph::node_iterator lazyGetPtr();
  
  // Looks up the location where the type is (or will be) stored, but does not construct the class.
  template <typename C>
  Graph::node_iterator lazyGetPtr(NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index);
  
  template <typename C>
  C* getPtr();
  
  // Similar to the previous, but takes an dep vector + index. Use this when the node_iterator is known, it's faster.
  template <typename C>
  C* getPtr(NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index);
  
  // getPtr() is equivalent to getPtr(lazyGetPtr())
  // getPtr(deps, index) is equivalent to getPtr(lazyGetPtr(deps, index))
  template <typename C>
  C* getPtr(Graph::node_iterator itr);
  
  void* getPtr(TypeId typeInfo);
  // Similar to the previous, but takes a node_iterator. Use this when the node_iterator is known, it's faster.
  void* getPtr(Graph::node_iterator itr);
  
  // getPtr(typeInfo) is equivalent to getPtr(lazyGetPtr(typeInfo)).
  Graph::node_iterator lazyGetPtr(TypeId type);
  
  // getPtr(deps, index) is equivalent to getPtr(lazyGetPtr(deps, index)).
  Graph::node_iterator lazyGetPtr(NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index);
  
  // Similar to getPtr, but the binding might not exist. Returns nullptr if it doesn't.
  void* unsafeGetPtr(TypeId type);
  
  void* getPtrForMultibinding(TypeId type);
  
  // Returns a std::vector<T*>*, or nullptr if there are no multibindings.
  void* getMultibindings(TypeId type);
  
  // Gets the instance from BindingData, and constructs it if necessary.
  void ensureConstructed(typename SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator node_itr);
  
  // Constructs any necessary instances, but NOT the instance set.
  void ensureConstructedMultibinding(NormalizedMultibindingData& multibinding_data);
  
  template <typename T>
  friend struct GetHelper;
  
  template <typename T>
  friend class fruit::Provider;
  
public:
  InjectorStorage(ComponentStorage&& storage, std::initializer_list<TypeId> exposed_types);
  
  InjectorStorage(const NormalizedComponentStorage& normalized_storage, 
                  ComponentStorage&& storage,
                  std::initializer_list<TypeId> exposed_types);
  
  InjectorStorage(InjectorStorage&&) = delete;
  InjectorStorage& operator=(InjectorStorage&&) = delete;
  
  InjectorStorage(const InjectorStorage& other) = delete;
  InjectorStorage& operator=(const InjectorStorage& other) = delete;
  
  ~InjectorStorage();
  
  static void normalizeBindings(std::vector<std::pair<TypeId, BindingData>>& bindings_vectoor,
                                std::size_t& total_size,
                                std::vector<CompressedBinding>&& compressed_bindings_vector,
                                const std::vector<std::pair<TypeId, MultibindingData>>& multibindings,
                                std::initializer_list<TypeId> exposed_types);

  static void addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingData>& multibindings,
                               std::size_t& total_size,
                               std::vector<std::pair<TypeId, MultibindingData>>&& multibindings_vector);
  
  template <typename T>
  T get();
  
  // Similar to the above, but specifying the node_iterator of the type. Use this when the node_iterator is known, it's faster.
  // dep_index is the index of the dep in `deps'.
  template <typename T>
  T get(NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index);
   
  // Returns nullptr if C was not bound.
  template <typename C>
  C* unsafeGet();
  
  // Allocates an object of type T, constructing it with the specified arguments. Similar to:
  // new C(args...)
  // Also calls executeOnDestruction(), no need to do it explicitly.
  template <typename T, typename... Args>
  T* constructObject(Args&&... args);
  
  void executeOnDestruction(BindingData::destroy_t destroy, void* p);
  
  // Destroys an object previously created using constructObject().
  // Can only be used on destruction, in particular no further calls to constructObject are allowed after calling this.
  template <typename C>
  static void destroyObject(void* p);
  
  // Calls delete on a object previously allocated using new.
  // Can only be used on destruction, in particular no further calls to constructObject are allowed after calling this.
  template <typename C>
  static void destroyExternalObject(void* p);
  
  template <typename C>
  const std::vector<C*>& getMultibindings();
  
  void eagerlyInjectMultibindings();
};

} // namespace impl
} // namespace fruit

#include "injector_storage.defn.h"


#endif // FRUIT_INJECTOR_STORAGE_H
