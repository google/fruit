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

#ifndef FRUIT_INJECTOR_STORAGE_DEFN_H
#define FRUIT_INJECTOR_STORAGE_DEFN_H

#include <fruit/impl/component_storage/component_storage_entry.h>
#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/meta/component.h>
#include <fruit/impl/meta/vector.h>
#include <fruit/impl/util/demangle_type_name.h>
#include <fruit/impl/util/lambda_invoker.h>
#include <fruit/impl/util/type_info.h>

#include <cassert>

// Redundant, but makes KDevelop happy.
#include <fruit/impl/injector/injector_storage.h>

namespace fruit {
namespace impl {

inline InjectorStorage::BindingDataNodeIter* InjectorStorage::BindingDataNodeIter::operator->() {
  return this;
}

inline void InjectorStorage::BindingDataNodeIter::operator++() {
  ++itr;
}

inline bool InjectorStorage::BindingDataNodeIter::operator==(const BindingDataNodeIter& other) const {
  return itr == other.itr;
}

inline bool InjectorStorage::BindingDataNodeIter::operator!=(const BindingDataNodeIter& other) const {
  return itr != other.itr;
}

inline std::ptrdiff_t InjectorStorage::BindingDataNodeIter::operator-(BindingDataNodeIter other) const {
  return itr - other.itr;
}

inline TypeId InjectorStorage::BindingDataNodeIter::getId() {
  // For these kinds the type_id has a different meaning, but we never need to call this method for those.
  FruitAssert(itr->kind != ComponentStorageEntry::Kind::COMPRESSED_BINDING);
  FruitAssert(itr->kind != ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS);
  FruitAssert(itr->kind != ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_NO_ARGS);
  FruitAssert(itr->kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS);
  FruitAssert(itr->kind != ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS);
  FruitAssert(itr->kind != ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS);
  FruitAssert(itr->kind != ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS);
  return itr->type_id;
}

inline NormalizedBinding InjectorStorage::BindingDataNodeIter::getValue() {
  return NormalizedBinding(*itr);
}

inline bool InjectorStorage::BindingDataNodeIter::isTerminal() {
#if FRUIT_EXTRA_DEBUG
  if (itr->kind != ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT &&
      itr->kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION &&
      itr->kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION &&
      itr->kind != ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION) {
    std::cerr << "Unexpected binding kind: " << (std::size_t)itr->kind << std::endl;
    FruitAssert(false);
  }
#endif
  return itr->kind == ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT;
}

inline const TypeId* InjectorStorage::BindingDataNodeIter::getEdgesBegin() {
  FruitAssert(itr->kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION ||
              itr->kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION ||
              itr->kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION);
  return itr->binding_for_object_to_construct.deps->deps;
}

inline const TypeId* InjectorStorage::BindingDataNodeIter::getEdgesEnd() {
  FruitAssert(itr->kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION ||
              itr->kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION ||
              itr->kind == ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION);
  return itr->binding_for_object_to_construct.deps->deps + itr->binding_for_object_to_construct.deps->num_deps;
}

template <typename AnnotatedT>
struct GetFirstStage;

// General case, value.
template <typename C>
struct GetFirstStage {
  const C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return injector.getPtr<C>(node_itr);
  }
};

template <typename C>
struct GetFirstStage<const C> {
  const C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return injector.getPtr<C>(node_itr);
  }
};

template <typename C>
struct GetFirstStage<std::shared_ptr<C>> {
  // This method is covered by tests, even though lcov doesn't detect that.
  C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    FruitAssert(node_itr.getNode().is_nonconst);
    return const_cast<C*>(injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetFirstStage<C*> {
  C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    FruitAssert(node_itr.getNode().is_nonconst);
    return const_cast<C*>(injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetFirstStage<const C*> {
  const C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return injector.getPtr<C>(node_itr);
  }
};

template <typename C>
struct GetFirstStage<C&> {
  C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    FruitAssert(node_itr.getNode().is_nonconst);
    return const_cast<C*>(injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetFirstStage<const C&> {
  // This method is covered by tests, even though lcov doesn't detect that.
  const C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return injector.getPtr<C>(node_itr);
  }
};

template <typename C>
struct GetFirstStage<Provider<C>> {
  Provider<C> operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return Provider<C>(&injector, node_itr);
  }
};

template <typename Annotation, typename T>
struct GetFirstStage<fruit::Annotated<Annotation, T>> : public GetFirstStage<T> {};

template <typename AnnotatedT>
struct GetSecondStage;

// General case, value.
template <typename C>
struct GetSecondStage {
  C operator()(const C* p) {
    return *p;
  }
};

template <typename C>
struct GetSecondStage<const C> {
  const C operator()(const C* p) {
    return *p;
  }
};

template <typename C>
struct GetSecondStage<std::shared_ptr<C>> {
  // This method is covered by tests, even though lcov doesn't detect that.
  std::shared_ptr<C> operator()(C* p) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), p);
  }
};

template <typename C>
struct GetSecondStage<C*> {
  C* operator()(C* p) {
    return p;
  }
};

template <typename C>
struct GetSecondStage<const C*> {
  // This method is covered by tests, even though lcov doesn't detect that.
  const C* operator()(const C* p) {
    return p;
  }
};

template <typename C>
struct GetSecondStage<C&> {
  C& operator()(C* p) {
    return *p;
  }
};

template <typename C>
struct GetSecondStage<const C&> {
  const C& operator()(const C* p) {
    return *p;
  }
};

template <typename C>
struct GetSecondStage<Provider<C>> {
  Provider<C> operator()(Provider<C> p) {
    return p;
  }
};

template <typename Annotation, typename T>
struct GetSecondStage<fruit::Annotated<Annotation, T>> : public GetSecondStage<T> {};

template <typename AnnotatedT>
inline InjectorStorage::RemoveAnnotations<AnnotatedT> InjectorStorage::get() {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return GetSecondStage<AnnotatedT>()(GetFirstStage<AnnotatedT>()(*this, lazyGetPtr<NormalizeType<AnnotatedT>>()));
}

template <typename T>
inline T InjectorStorage::get(InjectorStorage::Graph::node_iterator node_iterator) {
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<T>,
                                              fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<T>)));
  std::lock_guard<std::recursive_mutex> lock(mutex);
  return GetSecondStage<T>()(GetFirstStage<T>()(*this, node_iterator));
}

template <typename AnnotatedC>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr() {
  return lazyGetPtr(getTypeId<AnnotatedC>());
}

template <typename AnnotatedC>
inline InjectorStorage::Graph::node_iterator
InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index, Graph::node_iterator bindings_begin)
    const {
  // Here we (intentionally) do not lock `mutex', since this is a read-only method that only accesses immutable data.
  Graph::node_iterator itr = deps.getNodeIterator(dep_index, bindings_begin);
  FruitAssert(bindings.find(getTypeId<AnnotatedC>()) == Graph::const_node_iterator(itr));
  FruitAssert(!(bindings.end() == Graph::const_node_iterator(itr)));
  return itr;
}

template <typename C>
inline const C* InjectorStorage::getPtr(Graph::node_iterator itr) {
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<C>,
                                              fruit::impl::meta::NormalizeType(fruit::impl::meta::Type<C>)));
  const void* p = getPtrInternal(itr);
  return reinterpret_cast<const C*>(p);
}

template <typename AnnotatedC>
inline const InjectorStorage::RemoveAnnotations<AnnotatedC>* InjectorStorage::unsafeGet() {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  using C = RemoveAnnotations<AnnotatedC>;
  const void* p = unsafeGetPtr(getTypeId<AnnotatedC>());
  return reinterpret_cast<const C*>(p);
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(TypeId type) {
  return bindings.at(type);
}

inline const void* InjectorStorage::unsafeGetPtr(TypeId type) {
  Graph::node_iterator itr = bindings.find(type);
  if (itr == bindings.end()) {
    return nullptr;
  }
  return getPtrInternal(itr);
}

template <typename AnnotatedC>
inline const std::vector<InjectorStorage::RemoveAnnotations<AnnotatedC>*>& InjectorStorage::getMultibindings() {
  std::lock_guard<std::recursive_mutex> lock(mutex);
  using C = RemoveAnnotations<AnnotatedC>;
  void* p = getMultibindings(getTypeId<AnnotatedC>());
  if (p == nullptr) {
    static std::vector<C*> empty_vector;
    return empty_vector;
  } else {
    return *reinterpret_cast<std::vector<C*>*>(p);
  }
}

inline const void* InjectorStorage::getPtrInternal(Graph::node_iterator node_itr) {
  NormalizedBinding& normalized_binding = node_itr.getNode();
  if (!node_itr.isTerminal()) {
    normalized_binding.object = normalized_binding.create(*this, node_itr);
    FruitAssert(node_itr.isTerminal());
  }
  return normalized_binding.object;
}

inline NormalizedMultibindingSet* InjectorStorage::getNormalizedMultibindingSet(TypeId type) {
  auto itr = multibindings.find(type);
  if (itr != multibindings.end())
    return &(itr->second);
  else
    return nullptr;
}

template <typename AnnotatedC>
inline std::shared_ptr<char> InjectorStorage::createMultibindingVector(InjectorStorage& storage) {
  using C = RemoveAnnotations<AnnotatedC>;
  TypeId type = getTypeId<AnnotatedC>();
  NormalizedMultibindingSet* multibinding_set = storage.getNormalizedMultibindingSet(type);

  // This method is only called if there was at least 1 multibinding (otherwise the would-be caller would have returned
  // nullptr
  // instead of calling this).
  FruitAssert(multibinding_set != nullptr);

  if (multibinding_set->v.get() != nullptr) {
    // Result cached, return early.
    return multibinding_set->v;
  }

  storage.ensureConstructedMultibinding(*multibinding_set);

  std::vector<C*> s;
  s.reserve(multibinding_set->elems.size());
  for (const NormalizedMultibinding& multibinding : multibinding_set->elems) {
    FruitAssert(multibinding.is_constructed);
    s.push_back(reinterpret_cast<C*>(multibinding.object));
  }

  std::shared_ptr<std::vector<C*>> vector_ptr = std::make_shared<std::vector<C*>>(std::move(s));
  std::shared_ptr<char> result(vector_ptr, reinterpret_cast<char*>(vector_ptr.get()));

  multibinding_set->v = result;

  return result;
}

template <typename I, typename C, typename AnnotatedC>
InjectorStorage::const_object_ptr_t
InjectorStorage::createInjectedObjectForBind(InjectorStorage& injector,
                                             InjectorStorage::Graph::node_iterator node_itr) {

  InjectorStorage::Graph::node_iterator bindings_begin = injector.bindings.begin();
  const C* cPtr = injector.get<const C*>(injector.lazyGetPtr<AnnotatedC>(node_itr.neighborsBegin(), 0, bindings_begin));
  node_itr.setTerminal();
  // This step is needed when the cast C->I changes the pointer
  // (e.g. for multiple inheritance).
  const I* iPtr = static_cast<const I*>(cPtr);
  return reinterpret_cast<const_object_ptr_t>(iPtr);
}

// I, C must not be pointers.
template <typename AnnotatedI, typename AnnotatedC>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForBind() {
  using I = RemoveAnnotations<AnnotatedI>;
  using C = RemoveAnnotations<AnnotatedC>;
  FruitStaticAssert(fruit::impl::meta::Not(fruit::impl::meta::IsPointer(fruit::impl::meta::Type<I>)));
  FruitStaticAssert(fruit::impl::meta::Not(fruit::impl::meta::IsPointer(fruit::impl::meta::Type<C>)));
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION;
  result.type_id = getTypeId<AnnotatedI>();
  ComponentStorageEntry::BindingForObjectToConstruct& binding = result.binding_for_object_to_construct;
  binding.create = createInjectedObjectForBind<I, C, AnnotatedC>;
  binding.deps = getBindingDeps<fruit::impl::meta::Vector<fruit::impl::meta::Type<AnnotatedC>>>();
#if FRUIT_EXTRA_DEBUG
  binding.is_nonconst = true;
#endif
  return result;
}

// I, C must not be pointers.
template <typename AnnotatedI, typename AnnotatedC>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForConstBind() {
  using I = RemoveAnnotations<AnnotatedI>;
  using C = RemoveAnnotations<AnnotatedC>;
  FruitStaticAssert(fruit::impl::meta::Not(fruit::impl::meta::IsPointer(fruit::impl::meta::Type<I>)));
  FruitStaticAssert(fruit::impl::meta::Not(fruit::impl::meta::IsPointer(fruit::impl::meta::Type<C>)));
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION;
  result.type_id = getTypeId<AnnotatedI>();
  ComponentStorageEntry::BindingForObjectToConstruct& binding = result.binding_for_object_to_construct;
  binding.create = createInjectedObjectForBind<I, C, AnnotatedC>;
  binding.deps = getBindingDeps<fruit::impl::meta::Vector<fruit::impl::meta::Type<AnnotatedC>>>();
#if FRUIT_EXTRA_DEBUG
  binding.is_nonconst = false;
#endif
  return result;
}

template <typename AnnotatedC, typename C>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForBindInstance(C& instance) {
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT;
  result.type_id = getTypeId<AnnotatedC>();
  ComponentStorageEntry::BindingForConstructedObject& binding = result.binding_for_constructed_object;
  binding.object_ptr = &instance;
#if FRUIT_EXTRA_DEBUG
  binding.is_nonconst = true;
#endif
  return result;
}

template <typename AnnotatedC, typename C>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForBindConstInstance(const C& instance) {
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT;
  result.type_id = getTypeId<AnnotatedC>();
  ComponentStorageEntry::BindingForConstructedObject& binding = result.binding_for_constructed_object;
  binding.object_ptr = &instance;
#if FRUIT_EXTRA_DEBUG
  binding.is_nonconst = false;
#endif
  return result;
}

// The inner operator() takes an InjectorStorage& and a Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of move-constructing a C into the injector's own allocator if needed.
template <typename AnnotatedSignature, typename Lambda, bool lambda_returns_pointer,
          typename AnnotatedT = InjectorStorage::SignatureType<AnnotatedSignature>,
          typename AnnotatedArgVector =
              fruit::impl::meta::Eval<fruit::impl::meta::SignatureArgs(fruit::impl::meta::Type<AnnotatedSignature>)>,
          typename Indexes =
              fruit::impl::meta::Eval<fruit::impl::meta::GenerateIntSequence(fruit::impl::meta::VectorSize(
                  fruit::impl::meta::SignatureArgs(fruit::impl::meta::Type<AnnotatedSignature>)))>>
struct InvokeLambdaWithInjectedArgVector;

// AnnotatedT is of the form C* or Annotated<Annotation, C*>
template <typename AnnotatedSignature, typename Lambda, typename AnnotatedT, typename... AnnotatedArgs,
          typename... Indexes>
struct InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, true /* lambda_returns_pointer */, AnnotatedT,
                                         fruit::impl::meta::Vector<AnnotatedArgs...>,
                                         fruit::impl::meta::Vector<Indexes...>> {
  using CPtr = InjectorStorage::RemoveAnnotations<AnnotatedT>;
  using AnnotatedC = InjectorStorage::NormalizeType<AnnotatedT>;

  FRUIT_ALWAYS_INLINE
  CPtr operator()(InjectorStorage& injector, FixedSizeAllocator& allocator) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    CPtr cPtr =
        LambdaInvoker::invoke<Lambda, typename InjectorStorage::AnnotationRemover<
                                          typename fruit::impl::meta::TypeUnwrapper<AnnotatedArgs>::type>::type...>(
            injector.get<fruit::impl::meta::UnwrapType<AnnotatedArgs>>()...);

    allocator.registerExternallyAllocatedObject(cPtr);

    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<AnnotatedC>()) +
                             " but the provider returned nullptr");
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }

    return cPtr;
  }

  // This is not inlined in outerConstructHelper so that when get<> needs to construct an object more complex than a
  // pointer
  // (e.g. a shared_ptr), that happens as late as possible so that it's easier for the optimizer to optimize out some
  // operations (e.g. the increment/decrement/check of shared_ptr's reference count).
  template <typename... GetFirstStageResults>
  FRUIT_ALWAYS_INLINE CPtr innerConstructHelper(InjectorStorage& injector,
                                                GetFirstStageResults... getFirstStageResults) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    return LambdaInvoker::invoke<Lambda, typename InjectorStorage::AnnotationRemover<
                                             typename fruit::impl::meta::TypeUnwrapper<AnnotatedArgs>::type>::type...>(
        GetSecondStage<InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>()(
            getFirstStageResults)...);
  }

  // This is not inlined in operator() so that all the lazyGetPtr() calls happen first (instead of being interleaved
  // with the get() calls). The lazyGetPtr() calls don't branch, while the get() calls branch on the result of the
  // lazyGetPtr()s, so it's faster to execute them in this order.
  template <typename... NodeItrs>
  FRUIT_ALWAYS_INLINE CPtr outerConstructHelper(InjectorStorage& injector, NodeItrs... nodeItrs) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    return innerConstructHelper(
        injector, GetFirstStage<InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>()(
                      injector, nodeItrs)...);
  }

  FRUIT_ALWAYS_INLINE
  CPtr operator()(InjectorStorage& injector, SemistaticGraph<TypeId, NormalizedBinding>& bindings,
                  FixedSizeAllocator& allocator, InjectorStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)deps;

    InjectorStorage::Graph::node_iterator bindings_begin = bindings.begin();
    // `bindings_begin' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)bindings_begin;
    CPtr cPtr = outerConstructHelper(
        injector,
        injector.lazyGetPtr<InjectorStorage::NormalizeType<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>(
            deps, fruit::impl::meta::getIntValue<Indexes>(), bindings_begin)...);
    allocator.registerExternallyAllocatedObject(cPtr);

    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<AnnotatedC>()) +
                             " but the provider returned nullptr");
      FRUIT_UNREACHABLE; // LCOV_EXCL_LINE
    }

    return cPtr;
  }
};

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedC, typename... AnnotatedArgs,
          typename... Indexes>
struct InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, false /* lambda_returns_pointer */, AnnotatedC,
                                         fruit::impl::meta::Vector<AnnotatedArgs...>,
                                         fruit::impl::meta::Vector<Indexes...>> {
  using C = InjectorStorage::RemoveAnnotations<AnnotatedC>;

  FRUIT_ALWAYS_INLINE
  C* operator()(InjectorStorage& injector, FixedSizeAllocator& allocator) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    return allocator.constructObject<AnnotatedC, C&&>(
        LambdaInvoker::invoke<Lambda, typename InjectorStorage::AnnotationRemover<
                                          typename fruit::impl::meta::TypeUnwrapper<AnnotatedArgs>::type>::type&&...>(
            injector.get<typename fruit::impl::meta::TypeUnwrapper<AnnotatedArgs>::type>()...));
  }

  // This is not inlined in outerConstructHelper so that when get<> needs to construct an object more complex than a
  // pointer
  // (e.g. a shared_ptr), that happens as late as possible so that it's easier for the optimizer to optimize out some
  // operations (e.g. the increment/decrement/check of shared_ptr's reference count).
  template <typename... GetFirstStageResults>
  FRUIT_ALWAYS_INLINE C* innerConstructHelper(InjectorStorage& injector, FixedSizeAllocator& allocator,
                                              GetFirstStageResults... getFirstStageResults) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    return allocator.constructObject<AnnotatedC, C&&>(
        LambdaInvoker::invoke<Lambda, typename InjectorStorage::AnnotationRemover<
                                          typename fruit::impl::meta::TypeUnwrapper<AnnotatedArgs>::type>::type...>(
            GetSecondStage<typename InjectorStorage::AnnotationRemover<
                typename fruit::impl::meta::TypeUnwrapper<AnnotatedArgs>::type>::type>()(getFirstStageResults)...));
  }

  // This is not inlined in operator() so that all the lazyGetPtr() calls happen first (instead of being interleaved
  // with the get() calls). The lazyGetPtr() calls don't branch, while the get() calls branch on the result of the
  // lazyGetPtr()s, so it's faster to execute them in this order.
  template <typename... NodeItrs>
  FRUIT_ALWAYS_INLINE C* outerConstructHelper(InjectorStorage& injector, FixedSizeAllocator& allocator,
                                              NodeItrs... nodeItrs) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    return innerConstructHelper(
        injector, allocator,
        GetFirstStage<InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>()(injector,
                                                                                                          nodeItrs)...);
  }

  C* operator()(InjectorStorage& injector, SemistaticGraph<TypeId, NormalizedBinding>& bindings,
                FixedSizeAllocator& allocator, InjectorStorage::Graph::edge_iterator deps) {
    InjectorStorage::Graph::node_iterator bindings_begin = bindings.begin();
    // `bindings_begin' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)bindings_begin;

    // `deps' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)deps;

    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    C* p = outerConstructHelper(
        injector, allocator,
        injector.lazyGetPtr<InjectorStorage::NormalizeType<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>(
            deps, fruit::impl::meta::getIntValue<Indexes>(), bindings_begin)...);
    return p;
  }
};

template <typename C, typename T, typename AnnotatedSignature, typename Lambda>
InjectorStorage::const_object_ptr_t InjectorStorage::createInjectedObjectForProvider(InjectorStorage& injector,
                                                                                     Graph::node_iterator node_itr) {
  C* cPtr = InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, std::is_pointer<T>::value>()(
      injector, injector.bindings, injector.allocator, node_itr.neighborsBegin());
  node_itr.setTerminal();
  return reinterpret_cast<const_object_ptr_t>(cPtr);
}

template <typename AnnotatedSignature, typename Lambda>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForProvider() {
#if FRUIT_EXTRA_DEBUG
  using Signature =
      fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotationsFromSignature(
          fruit::impl::meta::Type<AnnotatedSignature>)>>;
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<Signature>,
                                              fruit::impl::meta::FunctionSignature(fruit::impl::meta::Type<Lambda>)));
#endif
  using AnnotatedT = SignatureType<AnnotatedSignature>;
  using AnnotatedC = NormalizeType<AnnotatedT>;
  // T is either C or C*.
  using T = RemoveAnnotations<AnnotatedT>;
  using C = NormalizeType<T>;
  ComponentStorageEntry result;
  constexpr bool needs_allocation = !std::is_pointer<T>::value;
  result.kind = needs_allocation
                    ? ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION
                    : ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION;
  result.type_id = getTypeId<AnnotatedC>();
  ComponentStorageEntry::BindingForObjectToConstruct& binding = result.binding_for_object_to_construct;
  binding.create = createInjectedObjectForProvider<C, T, AnnotatedSignature, Lambda>;
  binding.deps = getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>();
#if FRUIT_EXTRA_DEBUG
  binding.is_nonconst = true;
#endif
  return result;
}

template <typename I, typename C, typename T, typename AnnotatedSignature, typename Lambda>
InjectorStorage::const_object_ptr_t
InjectorStorage::createInjectedObjectForCompressedProvider(InjectorStorage& injector, Graph::node_iterator node_itr) {
  C* cPtr = InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, std::is_pointer<T>::value>()(
      injector, injector.bindings, injector.allocator, node_itr.neighborsBegin());
  node_itr.setTerminal();
  I* iPtr = static_cast<I*>(cPtr);
  return reinterpret_cast<object_ptr_t>(iPtr);
}

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForCompressedProvider() {
#if FRUIT_EXTRA_DEBUG
  using Signature =
      fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotationsFromSignature(
          fruit::impl::meta::Type<AnnotatedSignature>)>>;
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<Signature>,
                                              fruit::impl::meta::FunctionSignature(fruit::impl::meta::Type<Lambda>)));
#endif
  using AnnotatedT = SignatureType<AnnotatedSignature>;
  using AnnotatedC = NormalizeType<AnnotatedT>;
  // T is either C or C*.
  using T = RemoveAnnotations<AnnotatedT>;
  using C = NormalizeType<T>;
  using I = RemoveAnnotations<AnnotatedI>;
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::COMPRESSED_BINDING;
  result.type_id = getTypeId<AnnotatedI>();
  ComponentStorageEntry::CompressedBinding& binding = result.compressed_binding;
  binding.c_type_id = getTypeId<AnnotatedC>();
  binding.create = createInjectedObjectForCompressedProvider<I, C, T, AnnotatedSignature, Lambda>;
  return result;
}

// The inner operator() takes an InjectorStorage& and a Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of allocating the required space into the injector's allocator.
template <typename AnnotatedSignature,
          typename Indexes =
              fruit::impl::meta::Eval<fruit::impl::meta::GenerateIntSequence(fruit::impl::meta::VectorSize(
                  fruit::impl::meta::SignatureArgs(fruit::impl::meta::Type<AnnotatedSignature>)))>>
struct InvokeConstructorWithInjectedArgVector;

template <typename AnnotatedC, typename... AnnotatedArgs, typename... Indexes>
struct InvokeConstructorWithInjectedArgVector<AnnotatedC(AnnotatedArgs...), fruit::impl::meta::Vector<Indexes...>> {
  using C = InjectorStorage::RemoveAnnotations<AnnotatedC>;

  // This is not inlined in outerConstructHelper so that when get<> needs to construct an object more complex than a
  // pointer
  // (e.g. a shared_ptr), that happens as late as possible so that it's easier for the optimizer to optimize out some
  // operations (e.g. the increment/decrement/check of shared_ptr's reference count).
  template <typename... GetFirstStageResults>
  FRUIT_ALWAYS_INLINE C* innerConstructHelper(InjectorStorage& injector, FixedSizeAllocator& allocator,
                                              GetFirstStageResults... getFirstStageResults) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    return allocator.constructObject<AnnotatedC, typename InjectorStorage::AnnotationRemover<AnnotatedArgs>::type&&...>(
        GetSecondStage<InjectorStorage::RemoveAnnotations<AnnotatedArgs>>()(getFirstStageResults)...);
  }

  // This is not inlined in operator() so that all the lazyGetPtr() calls happen first (instead of being interleaved
  // with the get() calls). The lazyGetPtr() calls don't branch, while the get() calls branch on the result of the
  // lazyGetPtr()s, so it's faster to execute them in this order.
  template <typename... NodeItrs>
  FRUIT_ALWAYS_INLINE C* outerConstructHelper(InjectorStorage& injector, FixedSizeAllocator& allocator,
                                              NodeItrs... nodeItrs) {
    // `injector' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)injector;
    return innerConstructHelper(
        injector, allocator, GetFirstStage<InjectorStorage::RemoveAnnotations<AnnotatedArgs>>()(injector, nodeItrs)...);
  }

  FRUIT_ALWAYS_INLINE
  C* operator()(InjectorStorage& injector, SemistaticGraph<TypeId, NormalizedBinding>& bindings,
                FixedSizeAllocator& allocator, InjectorStorage::Graph::edge_iterator deps) {

    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;

    InjectorStorage::Graph::node_iterator bindings_begin = bindings.begin();
    // `bindings_begin' *is* used below, but when there are no Args some compilers report it as unused.
    (void)bindings_begin;
    C* p = outerConstructHelper(injector, allocator,
                                injector.lazyGetPtr<typename InjectorStorage::TypeNormalizer<AnnotatedArgs>::type>(
                                    deps, fruit::impl::meta::getIntValue<Indexes>(), bindings_begin)...);
    return p;
  }
};

template <typename C, typename AnnotatedSignature>
InjectorStorage::const_object_ptr_t InjectorStorage::createInjectedObjectForConstructor(InjectorStorage& injector,
                                                                                        Graph::node_iterator node_itr) {
  C* cPtr = InvokeConstructorWithInjectedArgVector<AnnotatedSignature>()(injector, injector.bindings,
                                                                         injector.allocator, node_itr.neighborsBegin());
  node_itr.setTerminal();
  return reinterpret_cast<InjectorStorage::object_ptr_t>(cPtr);
}

template <typename AnnotatedSignature>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForConstructor() {
  using AnnotatedC = SignatureType<AnnotatedSignature>;
  using C = RemoveAnnotations<AnnotatedC>;
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION;
  result.type_id = getTypeId<AnnotatedC>();
  ComponentStorageEntry::BindingForObjectToConstruct& binding = result.binding_for_object_to_construct;
  binding.create = createInjectedObjectForConstructor<C, AnnotatedSignature>;
  binding.deps = getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>();
#if FRUIT_EXTRA_DEBUG
  binding.is_nonconst = true;
#endif
  return result;
}

template <typename I, typename C, typename AnnotatedSignature>
InjectorStorage::const_object_ptr_t
InjectorStorage::createInjectedObjectForCompressedConstructor(InjectorStorage& injector,
                                                              Graph::node_iterator node_itr) {
  C* cPtr = InvokeConstructorWithInjectedArgVector<AnnotatedSignature>()(injector, injector.bindings,
                                                                         injector.allocator, node_itr.neighborsBegin());
  node_itr.setTerminal();
  I* iPtr = static_cast<I*>(cPtr);
  return reinterpret_cast<object_ptr_t>(iPtr);
}

template <typename AnnotatedSignature, typename AnnotatedI>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForCompressedConstructor() {
  using AnnotatedC = SignatureType<AnnotatedSignature>;
  using C = RemoveAnnotations<AnnotatedC>;
  using I = RemoveAnnotations<AnnotatedI>;
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::COMPRESSED_BINDING;
  result.type_id = getTypeId<AnnotatedI>();
  ComponentStorageEntry::CompressedBinding& binding = result.compressed_binding;
  binding.c_type_id = getTypeId<AnnotatedC>();
  binding.create = createInjectedObjectForCompressedConstructor<I, C, AnnotatedSignature>;
  return result;
}

template <typename AnnotatedT>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForMultibindingVectorCreator() {
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::MULTIBINDING_VECTOR_CREATOR;
  result.type_id = getTypeId<AnnotatedT>();
  ComponentStorageEntry::MultibindingVectorCreator& binding = result.multibinding_vector_creator;
  binding.get_multibindings_vector = createMultibindingVector<AnnotatedT>;
  return result;
}

template <typename I, typename C, typename AnnotatedCPtr>
InjectorStorage::object_ptr_t InjectorStorage::createInjectedObjectForMultibinding(InjectorStorage& m) {
  C* cPtr = m.get<AnnotatedCPtr>();
  // This step is needed when the cast C->I changes the pointer
  // (e.g. for multiple inheritance).
  I* iPtr = static_cast<I*>(cPtr);
  return reinterpret_cast<InjectorStorage::object_ptr_t>(iPtr);
}

template <typename AnnotatedI, typename AnnotatedC>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForMultibinding() {
  using AnnotatedCPtr = fruit::impl::meta::UnwrapType<
      fruit::impl::meta::Eval<fruit::impl::meta::AddPointerInAnnotatedType(fruit::impl::meta::Type<AnnotatedC>)>>;
  using I = RemoveAnnotations<AnnotatedI>;
  using C = RemoveAnnotations<AnnotatedC>;
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION;
  result.type_id = getTypeId<AnnotatedI>();
  ComponentStorageEntry::MultibindingForObjectToConstruct& binding = result.multibinding_for_object_to_construct;
  binding.create = createInjectedObjectForMultibinding<I, C, AnnotatedCPtr>;
  binding.deps = getBindingDeps<fruit::impl::meta::Vector<fruit::impl::meta::Type<AnnotatedC>>>();
  return result;
}

template <typename AnnotatedC, typename C>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForInstanceMultibinding(C& instance) {
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::MULTIBINDING_FOR_CONSTRUCTED_OBJECT;
  result.type_id = getTypeId<AnnotatedC>();
  ComponentStorageEntry::MultibindingForConstructedObject& binding = result.multibinding_for_constructed_object;
  binding.object_ptr = &instance;
  return result;
}

template <typename C, typename T, typename AnnotatedSignature, typename Lambda>
InjectorStorage::object_ptr_t InjectorStorage::createInjectedObjectForMultibindingProvider(InjectorStorage& injector) {
  C* cPtr = InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, std::is_pointer<T>::value>()(
      injector, injector.allocator);
  return reinterpret_cast<object_ptr_t>(cPtr);
}

template <typename AnnotatedSignature, typename Lambda>
inline ComponentStorageEntry InjectorStorage::createComponentStorageEntryForMultibindingProvider() {
#if FRUIT_EXTRA_DEBUG
  using Signature =
      fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotationsFromSignature(
          fruit::impl::meta::Type<AnnotatedSignature>)>>;
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<Signature>,
                                              fruit::impl::meta::FunctionSignature(fruit::impl::meta::Type<Lambda>)));
#endif

  using AnnotatedT = SignatureType<AnnotatedSignature>;
  using AnnotatedC = NormalizeType<AnnotatedT>;
  using T = RemoveAnnotations<AnnotatedT>;
  using C = NormalizeType<T>;
  ComponentStorageEntry result;
  bool needs_allocation = !std::is_pointer<T>::value;
  if (needs_allocation)
    result.kind = ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION;
  else
    result.kind = ComponentStorageEntry::Kind::MULTIBINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION;
  result.type_id = getTypeId<AnnotatedC>();
  ComponentStorageEntry::MultibindingForObjectToConstruct& binding = result.multibinding_for_object_to_construct;
  binding.create = createInjectedObjectForMultibindingProvider<C, T, AnnotatedSignature, Lambda>;
  binding.deps = getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>();
  return result;
}

} // namespace fruit
} // namespace impl

#endif // FRUIT_INJECTOR_STORAGE_DEFN_H
