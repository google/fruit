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

#include <fruit/impl/util/demangle_type_name.h>
#include <fruit/impl/util/type_info.h>
#include <fruit/impl/util/lambda_invoker.h>
#include <fruit/impl/fruit_assert.h>
#include <fruit/impl/meta/vector.h>
#include <fruit/impl/meta/component.h>

#include <cassert>

// Redundant, but makes KDevelop happy.
#include <fruit/impl/storage/injector_storage.h>

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

inline TypeId InjectorStorage::BindingDataNodeIter::getId() {
  return itr->first;
}
    
inline NormalizedBindingData InjectorStorage::BindingDataNodeIter::getValue() {
  return NormalizedBindingData(itr->second);
}

inline bool InjectorStorage::BindingDataNodeIter::isTerminal() {
  return itr->second.isCreated();
}

inline const TypeId* InjectorStorage::BindingDataNodeIter::getEdgesBegin() {
  const BindingDeps* deps = itr->second.getDeps();
  return deps->deps;
}

inline const TypeId* InjectorStorage::BindingDataNodeIter::getEdgesEnd() {
  const BindingDeps* deps = itr->second.getDeps();
  return deps->deps + deps->num_deps;
}

template <typename AnnotatedT>
struct GetHelper;

// General case, value.
template <typename C>
struct GetHelper {
  C operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return *(injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetHelper<const C> {
  const C operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return *(injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetHelper<std::shared_ptr<C>> {
  // This method is covered by tests, even though lcov doesn't detect that.
  std::shared_ptr<C> operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetHelper<C*> {
  C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return injector.getPtr<C>(node_itr);
  }
};

template <typename C>
struct GetHelper<const C*> {
  // This method is covered by tests, even though lcov doesn't detect that.
  const C* operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return injector.getPtr<C>(node_itr);
  }
};

template <typename C>
struct GetHelper<C&> {
  C& operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return *(injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetHelper<const C&> {
  // This method is covered by tests, even though lcov doesn't detect that.
  const C& operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return *(injector.getPtr<C>(node_itr));
  }
};

template <typename C>
struct GetHelper<Provider<C>> {
  Provider<C> operator()(InjectorStorage& injector, InjectorStorage::Graph::node_iterator node_itr) {
    return Provider<C>(&injector, node_itr);
  }
};

template <typename Annotation, typename T>
struct GetHelper<fruit::Annotated<Annotation, T>> : public GetHelper<T> {
};

template <typename AnnotatedT>
inline InjectorStorage::RemoveAnnotations<AnnotatedT> InjectorStorage::get() {
  return GetHelper<AnnotatedT>()(*this, lazyGetPtr<NormalizeType<AnnotatedT>>());
}

template <typename T>
inline T InjectorStorage::get(InjectorStorage::Graph::node_iterator node_iterator) {
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<T>, fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<T>)));
  return GetHelper<T>()(*this, node_iterator);
}

template <typename AnnotatedC>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr() {
  return lazyGetPtr(getTypeId<AnnotatedC>());
}

template <typename AnnotatedC>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index, Graph::node_iterator bindings_begin) {
  Graph::node_iterator itr = deps.getNodeIterator(dep_index, bindings_begin);
  assert(bindings.find(getTypeId<AnnotatedC>()) == itr);
  assert(!(bindings.end() == itr));
  return itr;
}

template <typename C>
inline C* InjectorStorage::getPtr(Graph::node_iterator itr) {
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<C>, fruit::impl::meta::NormalizeType(fruit::impl::meta::Type<C>)));
  void* p = getPtrInternal(itr);
  return reinterpret_cast<C*>(p);
}

template <typename AnnotatedC>
inline InjectorStorage::RemoveAnnotations<AnnotatedC>* InjectorStorage::unsafeGet() {
  using C = RemoveAnnotations<AnnotatedC>;
  void* p = unsafeGetPtr(getTypeId<AnnotatedC>());
  return reinterpret_cast<C*>(p);
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(TypeId type) {
  return bindings.at(type);
}

inline void* InjectorStorage::unsafeGetPtr(TypeId type) {
  Graph::node_iterator itr = bindings.find(type);
  if (itr == bindings.end()) {
    return nullptr;
  }
  return getPtrInternal(itr);
}

template <typename AnnotatedC>
inline const std::vector<InjectorStorage::RemoveAnnotations<AnnotatedC>*>& InjectorStorage::getMultibindings() {
  using C = RemoveAnnotations<AnnotatedC>;
  void* p = getMultibindings(getTypeId<AnnotatedC>());
  if (p == nullptr) {
    static std::vector<C*> empty_vector;
    return empty_vector;
  } else {
    return *reinterpret_cast<std::vector<C*>*>(p);
  }
}

inline void* InjectorStorage::getPtrInternal(Graph::node_iterator node_itr) {
  NormalizedBindingData& bindingData = node_itr.getNode();
  if (!node_itr.isTerminal()) {
    bindingData.create(*this, node_itr);
    assert(node_itr.isTerminal());
  }
  return bindingData.getObject();
}

inline NormalizedMultibindingData* InjectorStorage::getNormalizedMultibindingData(TypeId type) {
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
  NormalizedMultibindingData* multibinding = storage.getNormalizedMultibindingData(type);
  
  // This method is only called if there was at least 1 multibinding (otherwise the would-be caller would have returned nullptr
  // instead of calling this).
  assert(multibinding != nullptr);
  
  if (multibinding->v.get() != nullptr) {
    // Result cached, return early.
    return multibinding->v;
  }
  
  storage.ensureConstructedMultibinding(*multibinding);
  
  std::vector<C*> s;
  s.reserve(multibinding->elems.size());
  for (const NormalizedMultibindingData::Elem& elem : multibinding->elems) {
    s.push_back(reinterpret_cast<C*>(elem.object));
  }
  
  std::shared_ptr<std::vector<C*>> vector_ptr = std::make_shared<std::vector<C*>>(std::move(s));
  std::shared_ptr<char> result(vector_ptr, reinterpret_cast<char*>(vector_ptr.get()));
  
  multibinding->v = result;
  
  return result;
}

// I, C must not be pointers.
template <typename AnnotatedI, typename AnnotatedC>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForBind() {
  using I = RemoveAnnotations<AnnotatedI>;
  using C = RemoveAnnotations<AnnotatedC>;
  FruitStaticAssert(fruit::impl::meta::Not(fruit::impl::meta::IsPointer(fruit::impl::meta::Type<I>)));
  FruitStaticAssert(fruit::impl::meta::Not(fruit::impl::meta::IsPointer(fruit::impl::meta::Type<C>)));
  auto create = [](InjectorStorage& injector, Graph::node_iterator node_itr) {
    InjectorStorage::Graph::node_iterator bindings_begin = injector.bindings.begin();
    C* cPtr = injector.get<C*>(injector.lazyGetPtr<AnnotatedC>(node_itr.neighborsBegin(), 0, bindings_begin));
    node_itr.setTerminal();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  return std::make_tuple(getTypeId<AnnotatedI>(), BindingData(create, getBindingDeps<fruit::impl::meta::Vector<fruit::impl::meta::Type<AnnotatedC>>>(), false /* needs_allocation */));
}

template <typename AnnotatedC, typename C>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForBindInstance(C& instance) {
  return std::make_tuple(getTypeId<AnnotatedC>(), BindingData(&instance));
}

// The inner operator() takes an InjectorStorage& and a Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of move-constructing a C into the injector's own allocator if needed.
template <typename AnnotatedSignature,
          typename Lambda,
          bool lambda_returns_pointer,
          typename AnnotatedT         = InjectorStorage::SignatureType<AnnotatedSignature>,
          typename AnnotatedArgVector = fruit::impl::meta::Eval<fruit::impl::meta::SignatureArgs(fruit::impl::meta::Type<AnnotatedSignature>)>,
          typename Indexes = fruit::impl::meta::Eval<
              fruit::impl::meta::GenerateIntSequence(fruit::impl::meta::VectorSize(
                  fruit::impl::meta::SignatureArgs(fruit::impl::meta::Type<AnnotatedSignature>)))
              >>
struct InvokeLambdaWithInjectedArgVector;

// AnnotatedT is of the form C* or Annotated<Annotation, C*>
template <typename AnnotatedSignature, typename Lambda, typename AnnotatedT, typename... AnnotatedArgs, int... indexes>
struct InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, true /* lambda_returns_pointer */, AnnotatedT, fruit::impl::meta::Vector<AnnotatedArgs...>, fruit::impl::meta::Vector<fruit::impl::meta::Int<indexes>...>> {
  using CPtr = InjectorStorage::RemoveAnnotations<AnnotatedT>;
  using AnnotatedC = InjectorStorage::NormalizeType<AnnotatedT>;
  
  CPtr operator()(InjectorStorage& injector, FixedSizeAllocator& allocator) {
    CPtr cPtr = LambdaInvoker::invoke<Lambda, InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>...>(
        injector.get<fruit::impl::meta::UnwrapType<AnnotatedArgs>>()...);
    allocator.registerExternallyAllocatedObject(cPtr);
    
    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<AnnotatedC>()) + " but the provider returned nullptr");
    }
    
    return cPtr;
  }
  
  CPtr operator()(InjectorStorage& injector, SemistaticGraph<TypeId, NormalizedBindingData>& bindings,
                  FixedSizeAllocator& allocator, InjectorStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)deps;
    
    InjectorStorage::Graph::node_iterator bindings_begin = bindings.begin();
    // `bindings_begin' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void) bindings_begin;
    CPtr cPtr = LambdaInvoker::invoke<Lambda, InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>...>(
        injector.get<InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>(
            injector.lazyGetPtr<InjectorStorage::NormalizeType<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>(deps, indexes, bindings_begin))
        ...);
    allocator.registerExternallyAllocatedObject(cPtr);
    
    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<AnnotatedC>()) + " but the provider returned nullptr");
    }
    
    return cPtr;
  }
};

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedC, typename... AnnotatedArgs, int... indexes>
struct InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, false /* lambda_returns_pointer */, AnnotatedC, fruit::impl::meta::Vector<AnnotatedArgs...>, fruit::impl::meta::Vector<fruit::impl::meta::Int<indexes>...>> {
  using C = InjectorStorage::RemoveAnnotations<AnnotatedC>;
  
  C* operator()(InjectorStorage& injector, FixedSizeAllocator& allocator) {
    return allocator.constructObject<AnnotatedC, C&&>(
        LambdaInvoker::invoke<Lambda, InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>...>(
            injector.get<fruit::impl::meta::UnwrapType<AnnotatedArgs>>()...));
  }
  
  C* operator()(InjectorStorage& injector, SemistaticGraph<TypeId, NormalizedBindingData>& bindings,
                FixedSizeAllocator& allocator, InjectorStorage::Graph::edge_iterator deps) {
    InjectorStorage::Graph::node_iterator bindings_begin = bindings.begin();
    // `bindings_begin' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void) bindings_begin;
    
    // `deps' *is* used below, but when there are no AnnotatedArgs some compilers report it as unused.
    (void)deps;
    
    C* p = allocator.constructObject<AnnotatedC, C&&>(LambdaInvoker::invoke<Lambda, InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>...>(
        injector.get<InjectorStorage::RemoveAnnotations<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>(
            injector.lazyGetPtr<InjectorStorage::NormalizeType<fruit::impl::meta::UnwrapType<AnnotatedArgs>>>(deps, indexes, bindings_begin))
        ...));
    return p;
  }
};

template <typename AnnotatedSignature, typename Lambda>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForProvider() {
#ifdef FRUIT_EXTRA_DEBUG
  using Signature = fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotationsFromSignature(fruit::impl::meta::Type<AnnotatedSignature>)>>;
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<Signature>, fruit::impl::meta::FunctionSignature(fruit::impl::meta::Type<Lambda>)));
#endif
  using AnnotatedT = SignatureType<AnnotatedSignature>;
  using AnnotatedC = NormalizeType<AnnotatedT>;
  // T is either C or C*.
  using T          = RemoveAnnotations<AnnotatedT>;
  using C          = NormalizeType<T>;
  auto create = [](InjectorStorage& injector, Graph::node_iterator node_itr) {
    C* cPtr = InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, std::is_pointer<T>::value>()(
        injector, injector.bindings, injector.allocator, node_itr.neighborsBegin());
    node_itr.setTerminal();
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  const BindingDeps* deps = getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>();
  bool needs_allocation = !std::is_pointer<T>::value;
  return std::make_tuple(getTypeId<AnnotatedC>(), BindingData(create, deps, needs_allocation));
}

template <typename AnnotatedSignature, typename Lambda, typename AnnotatedI>
inline std::tuple<TypeId, TypeId, BindingData> InjectorStorage::createBindingDataForCompressedProvider() {
#ifdef FRUIT_EXTRA_DEBUG
  using Signature = fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotationsFromSignature(fruit::impl::meta::Type<AnnotatedSignature>)>>;
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<Signature>, fruit::impl::meta::FunctionSignature(fruit::impl::meta::Type<Lambda>)));
#endif
  using AnnotatedT = SignatureType<AnnotatedSignature>;
  using AnnotatedC = NormalizeType<AnnotatedT>;
  // T is either C or C*.
  using T          = RemoveAnnotations<AnnotatedT>;
  using C          = NormalizeType<T>;
  using I          = RemoveAnnotations<AnnotatedI>;
  auto create = [](InjectorStorage& injector, Graph::node_iterator node_itr) {
    C* cPtr = InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, std::is_pointer<T>::value>()(
        injector, injector.bindings, injector.allocator, node_itr.neighborsBegin());
    node_itr.setTerminal();
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  const BindingDeps* deps = getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>();
  bool needs_allocation = !std::is_pointer<T>::value;
  return std::make_tuple(getTypeId<AnnotatedI>(), getTypeId<AnnotatedC>(), BindingData(create, deps, needs_allocation));
}

// The inner operator() takes an InjectorStorage& and a Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of allocating the required space into the injector's allocator.
template <typename AnnotatedSignature,
          typename Indexes = fruit::impl::meta::Eval<
              fruit::impl::meta::GenerateIntSequence(fruit::impl::meta::VectorSize(
                  fruit::impl::meta::SignatureArgs(fruit::impl::meta::Type<AnnotatedSignature>)))
              >>
struct InvokeConstructorWithInjectedArgVector;

template <typename AnnotatedC, typename... AnnotatedArgs, int... indexes>
struct InvokeConstructorWithInjectedArgVector<AnnotatedC(AnnotatedArgs...), fruit::impl::meta::Vector<fruit::impl::meta::Int<indexes>...>> {
  using C          = InjectorStorage::RemoveAnnotations<AnnotatedC>;
  
  C* operator()(InjectorStorage& injector, SemistaticGraph<TypeId, NormalizedBindingData>& bindings,
                FixedSizeAllocator& allocator, InjectorStorage::Graph::edge_iterator deps) {
    
    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;
    
    InjectorStorage::Graph::node_iterator bindings_begin = bindings.begin();
    // `bindings_begin' *is* used below, but when there are no Args some compilers report it as unused.
    (void) bindings_begin;
    C* p = allocator.constructObject<AnnotatedC, InjectorStorage::RemoveAnnotations<AnnotatedArgs>...>(
        injector.get<InjectorStorage::RemoveAnnotations<AnnotatedArgs>>(
            injector.lazyGetPtr<InjectorStorage::NormalizeType<AnnotatedArgs>>(deps, indexes, bindings_begin))
        ...);
    return p;
  }
};

template <typename AnnotatedSignature>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForConstructor() {
  using AnnotatedC = SignatureType<AnnotatedSignature>;
  using C          = RemoveAnnotations<AnnotatedC>;
  auto create = [](InjectorStorage& injector, Graph::node_iterator node_itr) {
    C* cPtr = InvokeConstructorWithInjectedArgVector<AnnotatedSignature>()(injector, 
                  injector.bindings, injector.allocator, node_itr.neighborsBegin());
    node_itr.setTerminal();
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  const BindingDeps* deps = getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>();
  return std::make_tuple(getTypeId<AnnotatedC>(), BindingData(create, deps, true /* needs_allocation */));
}

template <typename AnnotatedSignature, typename AnnotatedI>
inline std::tuple<TypeId, TypeId, BindingData> InjectorStorage::createBindingDataForCompressedConstructor() {
  using AnnotatedC = SignatureType<AnnotatedSignature>;
  using C          = RemoveAnnotations<AnnotatedC>;
  using I          = RemoveAnnotations<AnnotatedI>;
  auto create = [](InjectorStorage& injector, Graph::node_iterator node_itr) {
    C* cPtr = InvokeConstructorWithInjectedArgVector<AnnotatedSignature>()(injector, 
                  injector.bindings, injector.allocator, node_itr.neighborsBegin());
    node_itr.setTerminal();
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  const BindingDeps* deps = getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>();
  return std::make_tuple(getTypeId<AnnotatedI>(), getTypeId<AnnotatedC>(), BindingData(create, deps, true /* needs_allocation */));
}

template <typename AnnotatedI, typename AnnotatedC>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForBinding() {
  using AnnotatedCPtr = fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<fruit::impl::meta::AddPointerInAnnotatedType(fruit::impl::meta::Type<AnnotatedC>)>>;
  using I             = RemoveAnnotations<AnnotatedI>;
  using C             = RemoveAnnotations<AnnotatedC>;
  auto create = [](InjectorStorage& m) {
    C* cPtr = m.get<AnnotatedCPtr>();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<MultibindingData::object_t>(iPtr);
  };
  return std::make_tuple(getTypeId<AnnotatedI>(), MultibindingData(create, getBindingDeps<fruit::impl::meta::Vector<fruit::impl::meta::Type<AnnotatedC>>>(), createMultibindingVector<AnnotatedI>,
                                                                   false /* needs_allocation */));
}

template <typename AnnotatedC, typename C>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForInstance(C& instance) {
  return std::make_tuple(getTypeId<AnnotatedC>(), MultibindingData(&instance, createMultibindingVector<AnnotatedC>));
}

template <typename AnnotatedSignature, typename Lambda>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForProvider() {
#ifdef FRUIT_EXTRA_DEBUG
  using Signature = fruit::impl::meta::UnwrapType<fruit::impl::meta::Eval<fruit::impl::meta::RemoveAnnotationsFromSignature(fruit::impl::meta::Type<AnnotatedSignature>)>>;
  FruitStaticAssert(fruit::impl::meta::IsSame(fruit::impl::meta::Type<Signature>, fruit::impl::meta::FunctionSignature(fruit::impl::meta::Type<Lambda>)));
#endif

  using AnnotatedT = SignatureType<AnnotatedSignature>;
  using AnnotatedC = NormalizeType<AnnotatedT>;
  using T          = RemoveAnnotations<AnnotatedT>;
  using C          = NormalizeType<T>;
  auto create = [](InjectorStorage& injector) {
    C* cPtr = InvokeLambdaWithInjectedArgVector<AnnotatedSignature, Lambda, std::is_pointer<T>::value>()(
        injector, injector.allocator);
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  bool needs_allocation = !std::is_pointer<T>::value;
  return std::make_tuple(getTypeId<AnnotatedC>(),
                         MultibindingData(create, getBindingDeps<NormalizedSignatureArgs<AnnotatedSignature>>(), InjectorStorage::createMultibindingVector<AnnotatedC>,
                                          needs_allocation));
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_INJECTOR_STORAGE_DEFN_H
