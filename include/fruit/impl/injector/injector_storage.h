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

#include <fruit/fruit_forward_decls.h>
#include <fruit/impl/data_structures/fixed_size_allocator.h>
#include <fruit/impl/meta/component.h>
#include <fruit/impl/normalized_component_storage/normalized_bindings.h>

#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>

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
  // TODO: remove.
  //  using BindingVectors = std::pair<std::vector<std::pair<TypeId, BindingData>>,
  //                                   std::vector<std::pair<TypeId, MultibindingData>>>;
  using Graph = SemistaticGraph<TypeId, NormalizedBinding>;

  template <typename AnnotatedT>
  using RemoveAnnotations = fruit::impl::meta::UnwrapType<
      fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<AnnotatedT>)>>;

  // MSVC 14 has trouble specializing alias templates using expanded pack elements.
  // This is a known issue:
  // https://stackoverflow.com/questions/43411542/metaprogramming-failed-to-specialize-alias-template
  // The workaround is just to use a struct directly.
  template <typename AnnotatedT>
  struct AnnotationRemover {
    using type = RemoveAnnotations<AnnotatedT>;
  };

  template <typename T>
  using NormalizeType = fruit::impl::meta::UnwrapType<
      fruit::impl::meta::Eval<fruit::impl::meta::NormalizeType(fruit::impl::meta::Type<T>)>>;

  template <typename T>
  struct TypeNormalizer {
    using type = NormalizeType<T>;
  };

  template <typename Signature>
  using SignatureType = fruit::impl::meta::UnwrapType<
      fruit::impl::meta::Eval<fruit::impl::meta::SignatureType(fruit::impl::meta::Type<Signature>)>>;

  template <typename Signature>
  using NormalizedSignatureArgs = fruit::impl::meta::Eval<fruit::impl::meta::NormalizeTypeVector(
      fruit::impl::meta::SignatureArgs(fruit::impl::meta::Type<Signature>))>;

  // Prints the specified error and calls exit(1).
  static void fatal(const std::string& error);

  template <typename AnnotatedI, typename AnnotatedC>
  static ComponentStorageEntry createComponentStorageEntryForBind();

  template <typename AnnotatedI, typename AnnotatedC>
  static ComponentStorageEntry createComponentStorageEntryForConstBind();

  template <typename AnnotatedC, typename C>
  static ComponentStorageEntry createComponentStorageEntryForBindInstance(C& instance);

  template <typename AnnotatedC, typename C>
  static ComponentStorageEntry createComponentStorageEntryForBindConstInstance(const C& instance);

  template <typename AnnotatedSignature, typename Lambda>
  static ComponentStorageEntry createComponentStorageEntryForProvider();

  template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
  static ComponentStorageEntry createComponentStorageEntryForCompressedProvider();

  template <typename AnnotatedSignature>
  static ComponentStorageEntry createComponentStorageEntryForConstructor();

  template <typename AnnotatedSignature, typename AnnotatedI>
  static ComponentStorageEntry createComponentStorageEntryForCompressedConstructor();

  template <typename AnnotatedT>
  static ComponentStorageEntry createComponentStorageEntryForMultibindingVectorCreator();

  template <typename AnnotatedI, typename AnnotatedC>
  static ComponentStorageEntry createComponentStorageEntryForMultibinding();

  template <typename AnnotatedC, typename C>
  static ComponentStorageEntry createComponentStorageEntryForInstanceMultibinding(C& instance);

  template <typename AnnotatedSignature, typename Lambda>
  static ComponentStorageEntry createComponentStorageEntryForMultibindingProvider();

private:
  // The NormalizedComponentStorage owned by this object (if any).
  // Only used for the 1-argument constructor, otherwise it's nullptr.
  std::unique_ptr<NormalizedComponentStorage> normalized_component_storage_ptr;

  FixedSizeAllocator allocator;

  // A graph with injected types as nodes (each node stores the NormalizedBindingData for the type) and dependencies as
  // edges.
  // For types that have a constructed object already, the corresponding node is stored as terminal node.
  SemistaticGraph<TypeId, NormalizedBinding> bindings;

  // Maps the type index of a type T to the corresponding NormalizedMultibindingSet object (that stores all
  // multibindings).
  std::unordered_map<TypeId, NormalizedMultibindingSet> multibindings;

  // This mutex is used to synchronize concurrent accesses to this InjectorStorage object.
  std::recursive_mutex mutex;

private:
  template <typename AnnotatedC>
  static std::shared_ptr<char> createMultibindingVector(InjectorStorage& storage);

  // If not bound, returns nullptr.
  NormalizedMultibindingSet* getNormalizedMultibindingSet(TypeId type);

  // Looks up the location where the type is (or will be) stored, but does not construct the class.
  template <typename AnnotatedC>
  Graph::node_iterator lazyGetPtr();

  // getPtr() is equivalent to getPtrInternal(lazyGetPtr())
  template <typename C>
  const C* getPtr(Graph::node_iterator itr);

  // Similar to the previous, but takes a node_iterator. Use this when the node_iterator is known, it's faster.
  const void* getPtrInternal(Graph::node_iterator itr);

  // getPtr(typeInfo) is equivalent to getPtr(lazyGetPtr(typeInfo)).
  Graph::node_iterator lazyGetPtr(TypeId type);

  // getPtr(deps, index) is equivalent to getPtr(lazyGetPtr(deps, index)).
  Graph::node_iterator lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index);

  // Similar to getPtr, but the binding might not exist. Returns nullptr if it doesn't.
  const void* unsafeGetPtr(TypeId type);

  void* getPtrForMultibinding(TypeId type);

  // Returns a std::vector<T*>*, or nullptr if there are no multibindings.
  void* getMultibindings(TypeId type);

  // Constructs any necessary instances, but NOT the instance set.
  void ensureConstructedMultibinding(NormalizedMultibindingSet& multibinding_set);

  template <typename T>
  friend struct GetFirstStage;

  template <typename T>
  friend class fruit::Provider;

  using object_ptr_t = void*;
  using const_object_ptr_t = const void*;

  template <typename I, typename C, typename AnnotatedC>
  static const_object_ptr_t createInjectedObjectForBind(InjectorStorage& injector,
                                                        InjectorStorage::Graph::node_iterator node_itr);

  template <typename C, typename T, typename AnnotatedSignature, typename Lambda>
  static const_object_ptr_t createInjectedObjectForProvider(InjectorStorage& injector, Graph::node_iterator node_itr);

  template <typename I, typename C, typename T, typename AnnotatedSignature, typename Lambda>
  static const_object_ptr_t createInjectedObjectForCompressedProvider(InjectorStorage& injector,
                                                                      Graph::node_iterator node_itr);

  template <typename C, typename AnnotatedSignature>
  static const_object_ptr_t createInjectedObjectForConstructor(InjectorStorage& injector,
                                                               Graph::node_iterator node_itr);

  template <typename I, typename C, typename AnnotatedSignature>
  static const_object_ptr_t createInjectedObjectForCompressedConstructor(InjectorStorage& injector,
                                                                         Graph::node_iterator node_itr);

  template <typename I, typename C, typename AnnotatedCPtr>
  static object_ptr_t createInjectedObjectForMultibinding(InjectorStorage& m);

  template <typename C, typename T, typename AnnotatedSignature, typename Lambda>
  static object_ptr_t createInjectedObjectForMultibindingProvider(InjectorStorage& injector);

public:
  // Wraps a std::vector<ComponentStorageEntry>::iterator as an iterator on tuples
  // (typeId, normalizedBindingData, isTerminal, edgesBegin, edgesEnd)
  struct BindingDataNodeIter {
    std::vector<ComponentStorageEntry, ArenaAllocator<ComponentStorageEntry>>::iterator itr;

    BindingDataNodeIter* operator->();

    void operator++();

    bool operator==(const BindingDataNodeIter& other) const;
    bool operator!=(const BindingDataNodeIter& other) const;

    std::ptrdiff_t operator-(BindingDataNodeIter other) const;

    TypeId getId();
    NormalizedBinding getValue();
    bool isTerminal();
    const TypeId* getEdgesBegin();
    const TypeId* getEdgesEnd();
  };

  /**
   * The MemoryPool is only used during construction, the constructed object *can* outlive the memory pool.
   */
  InjectorStorage(ComponentStorage&& storage, const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
                  MemoryPool& memory_pool);

  /**
   * The MemoryPool is only used during construction, the constructed object *can* outlive the memory pool.
   */
  InjectorStorage(const NormalizedComponentStorage& normalized_storage, ComponentStorage&& storage,
                  MemoryPool& memory_pool);

  // This is just the default destructor, but we declare it here to avoid including
  // normalized_component_storage.h in fruit.h.
  ~InjectorStorage();

  InjectorStorage(InjectorStorage&&) = delete;
  InjectorStorage& operator=(InjectorStorage&&) = delete;

  InjectorStorage(const InjectorStorage& other) = delete;
  InjectorStorage& operator=(const InjectorStorage& other) = delete;

  // Usually get<T>() returns a T.
  // However, get<Annotated<Annotation1, T>>() returns a T, not an Annotated<Annotation1, T>.
  template <typename AnnotatedT>
  RemoveAnnotations<AnnotatedT> get();

  // Similar to the above, but specifying the node_iterator of the type. Use this together with lazyGetPtr when the
  // node_iterator is known, it's faster.
  // Note that T should *not* be annotated.
  template <typename T>
  T get(InjectorStorage::Graph::node_iterator node_iterator);

  // Looks up the location where the type is (or will be) stored, but does not construct the class.
  // get<AnnotatedT>() is equivalent to get<AnnotatedT>(lazyGetPtr<Apply<NormalizeType, AnnotatedT>>(deps, dep_index))
  // and also                        to get<T>         (lazyGetPtr<Apply<NormalizeType, AnnotatedT>>(deps, dep_index))
  // dep_index is the index of the dep in `deps'.
  template <typename AnnotatedC>
  Graph::node_iterator lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index,
                                  Graph::node_iterator bindings_begin) const;

  // Returns nullptr if AnnotatedC was not bound.
  template <typename AnnotatedC>
  const RemoveAnnotations<AnnotatedC>* unsafeGet();

  template <typename AnnotatedC>
  const std::vector<RemoveAnnotations<AnnotatedC>*>& getMultibindings();

  void eagerlyInjectMultibindings();
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/injector/injector_storage.defn.h>

#endif // FRUIT_INJECTOR_STORAGE_H
