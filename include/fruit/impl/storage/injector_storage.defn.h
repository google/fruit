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

#include "../util/demangle_type_name.h"
#include "../util/type_info.h"
#include "../util/lambda_invoker.h"
#include "../fruit_assert.h"
#include "../meta/vector.h"
#include "../meta/component.h"

#include <cassert>

// Redundant, but makes KDevelop happy.
#include "injector_storage.h"

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


// General case, value
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

template <typename T>
inline T InjectorStorage::get() {
  return GetHelper<T>()(*this, lazyGetPtr<meta::Apply<meta::GetClassForType, T>>());
}

template <typename T>
inline T InjectorStorage::get(Graph::edge_iterator deps, std::size_t dep_index) {
  return GetHelper<T>()(*this, lazyGetPtr<meta::Apply<meta::GetClassForType, T>>(deps, dep_index));
}

template <typename C>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr() {
  return lazyGetPtr(getTypeId<C>());
}

template <typename C>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  Graph::node_iterator itr = lazyGetPtr(deps, dep_index);
  assert(bindings.find(getTypeId<C>()) == itr);
  assert(!(bindings.end() == itr));
  return itr;
}

template <typename C>
inline C* InjectorStorage::getPtr() {
  void* p = getPtr(getTypeId<C>());
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::getPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  void* p = getPtr(lazyGetPtr(deps, dep_index));
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::getPtr(Graph::node_iterator itr) {
  assert(bindings.find(getTypeId<C>()) == itr);
  assert(!(bindings.end() == itr));
  void* p = getPtr(itr);
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::unsafeGet() {
  void* p = unsafeGetPtr(getTypeId<C>());
  return reinterpret_cast<C*>(p);
}

inline void* InjectorStorage::getPtr(TypeId type) {
  return getPtr(lazyGetPtr(type));
}

inline void* InjectorStorage::getPtr(Graph::node_iterator itr) {
  ensureConstructed(itr);
  return itr.getNode().getObject();
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(TypeId type) {
  return bindings.at(type);
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  return deps.getNodeIterator(dep_index, bindings);
}

inline void* InjectorStorage::unsafeGetPtr(TypeId type) {
  Graph::node_iterator itr = bindings.find(type);
  if (itr == bindings.end()) {
    return nullptr;
  }
  ensureConstructed(itr);
  return itr.getNode().getObject();
}

template <typename C>
void InjectorStorage::destroyObject(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  cPtr->C::~C();
}

template <typename C>
void InjectorStorage::destroyExternalObject(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  delete cPtr;
}

template <typename C>
inline const std::vector<C*>& InjectorStorage::getMultibindings() {
  void* p = getMultibindings(getTypeId<C>());
  if (p == nullptr) {
    static std::vector<C*> empty_vector;
    return empty_vector;
  } else {
    return *reinterpret_cast<std::vector<C*>*>(p);
  }
}

inline void InjectorStorage::ensureConstructed(typename SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator node_itr) {
  NormalizedBindingData& bindingData = node_itr.getNode();
  if (!node_itr.isTerminal()) {
    bindingData.create(*this, node_itr.neighborsBegin());
    node_itr.setTerminal();
  }
}

template <typename T, typename... Args>
inline T* InjectorStorage::constructObject(Args&&... args) {
  T* x = allocator.constructObject<T>(std::forward<Args>(args)...);
  if (!std::is_trivially_destructible<T>::value) {
    executeOnDestruction(&InjectorStorage::destroyObject<T>, reinterpret_cast<void*>(x));
  }
  return x;
}

inline void InjectorStorage::executeOnDestruction(BindingData::destroy_t destroy, void* p) {
  on_destruction.emplace_back(destroy, p);
}

inline NormalizedMultibindingData* InjectorStorage::getNormalizedMultibindingData(TypeId type) {
  auto itr = multibindings.find(type);
  if (itr != multibindings.end())
    return &(itr->second);
  else
    return nullptr;
}

template <typename C>
inline std::shared_ptr<char> InjectorStorage::createMultibindingVector(InjectorStorage& storage) {
  TypeId type = getTypeId<C>();
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

// The inner operator() takes an InjectorStorage& and a Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of move-constructing a C into the injector's own allocator if needed.
template <typename Lambda,
          typename C         = meta::Apply<meta::SignatureType, meta::Apply<meta::FunctionSignature, Lambda>>,
          typename ArgVector = meta::Apply<meta::SignatureArgs, meta::Apply<meta::FunctionSignature, Lambda>>,
          typename Indexes = meta::GenerateIntSequence<
              meta::ApplyC<meta::VectorApparentSize,
                  meta::Apply<meta::SignatureArgs, meta::Apply<meta::FunctionSignature, Lambda>>
              >::value>
          >
struct InvokeLambdaWithInjectedArgVector;

template <typename Lambda, typename C, typename... Args, int... indexes>
struct InvokeLambdaWithInjectedArgVector<Lambda, C*, meta::Vector<Args...>, meta::IntVector<indexes...>> {
  C* operator()(InjectorStorage& injector) {
    C* cPtr = LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>()...);
    injector.executeOnDestruction(&InjectorStorage::destroyExternalObject<C>, static_cast<void*>(cPtr));
    
    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<C>()) + " but the provider returned nullptr");
    }
    
    return cPtr;
  }
  
  C* operator()(InjectorStorage& injector, InjectorStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;
    
    C* cPtr = LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>(deps, indexes)...);
    injector.executeOnDestruction(&InjectorStorage::destroyExternalObject<C>, static_cast<void*>(cPtr));
    
    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<C>()) + " but the provider returned nullptr");
    }
    
    return cPtr;
  }
};

template <typename Lambda, typename C, typename... Args, int... indexes>
struct InvokeLambdaWithInjectedArgVector<Lambda, C, meta::Vector<Args...>, meta::IntVector<indexes...>> {
  C* operator()(InjectorStorage& injector) {
    return injector.constructObject<C, C&&>(LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>()...));
  }
  
  C* operator()(InjectorStorage& injector, InjectorStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;
    
    return injector.constructObject<C, C&&>(LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>(deps, indexes)...));    
  }
};

// The inner operator() takes an InjectorStorage& and a Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of allocating the required space into the injector's allocator.
template <typename Lambda,
          typename Signature = meta::Apply<meta::FunctionSignature, Lambda>,
          typename Indexes = meta::GenerateIntSequence<
              meta::ApplyC<meta::VectorApparentSize,
                  meta::Apply<meta::SignatureArgs, Signature>
              >::value>
          >
struct InvokeConstructorWithInjectedArgVector;

template <typename Lambda, typename C, typename... Args, int... indexes>
struct InvokeConstructorWithInjectedArgVector<Lambda, C(Args...), meta::IntVector<indexes...>> {
  C* operator()(InjectorStorage& injector, InjectorStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;
    
    return injector.constructObject<C, Args...>(injector.get<Args>(deps, indexes)...);
  }
};

// I, C must not be pointers.
template <typename I, typename C>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForBind() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](InjectorStorage& m, Graph::edge_iterator deps) {
    C* cPtr = m.get<C*>(deps, 0);
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  return std::make_tuple(getTypeId<I>(), BindingData(create, getBindingDeps<meta::Vector<C>>()));
}

template <typename C>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForBindInstance(C& instance) {
  return std::make_tuple(getTypeId<C>(), BindingData(&instance));
}

template <typename Lambda>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForProvider() {
  // TODO: Move this check to injection_errors.h.
  static_assert(std::is_empty<Lambda>::value,
                "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
  using Signature = meta::Apply<meta::FunctionSignature, Lambda>;
  using C = typename std::remove_pointer<meta::Apply<meta::SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, Graph::edge_iterator deps) {
    C* cPtr = InvokeLambdaWithInjectedArgVector<Lambda>()(injector, deps);
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  const BindingDeps* deps = getBindingDeps<meta::Apply<meta::GetClassForTypeVector, meta::Apply<meta::SignatureArgs, Signature>>>();
  return std::make_tuple(getTypeId<C>(), BindingData(create, deps));
}

template <typename Lambda, typename I>
inline std::tuple<TypeId, TypeId, BindingData> InjectorStorage::createBindingDataForCompressedProvider() {
  using Signature = meta::Apply<meta::FunctionSignature, Lambda>;
  using C = typename std::remove_pointer<meta::Apply<meta::SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, Graph::edge_iterator deps) {
    C* cPtr = InvokeLambdaWithInjectedArgVector<Lambda>()(injector, deps);
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  const BindingDeps* deps = getBindingDeps<meta::Apply<meta::GetClassForTypeVector, meta::Apply<meta::SignatureArgs, Signature>>>();
  return std::make_tuple(getTypeId<I>(), getTypeId<C>(), BindingData(create, deps));
}

template <typename Signature>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForConstructor() {
  using C = typename std::remove_pointer<meta::Apply<meta::SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, Graph::edge_iterator deps) {
    C* cPtr = InvokeConstructorWithInjectedArgVector<Signature>()(injector, deps);
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  const BindingDeps* deps = getBindingDeps<meta::Apply<meta::GetClassForTypeVector, meta::Apply<meta::SignatureArgs, Signature>>>();
  return std::make_tuple(getTypeId<C>(), BindingData(create, deps));
}

template <typename Signature, typename I>
inline std::tuple<TypeId, TypeId, BindingData> InjectorStorage::createBindingDataForCompressedConstructor() {
  using C = typename std::remove_pointer<meta::Apply<meta::SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, Graph::edge_iterator deps) {
    C* cPtr = InvokeConstructorWithInjectedArgVector<Signature>()(injector, deps);
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  const BindingDeps* deps = getBindingDeps<meta::Apply<meta::GetClassForTypeVector, meta::Apply<meta::SignatureArgs, Signature>>>();
  return std::make_tuple(getTypeId<I>(), getTypeId<C>(), BindingData(create, deps));
}

template <typename I, typename C>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForBinding() {
  auto create = [](InjectorStorage& m) {
    C* cPtr = m.get<C*>();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<MultibindingData::object_t>(iPtr);
  };
  return std::make_tuple(getTypeId<I>(), MultibindingData(create, getBindingDeps<meta::Vector<C>>(), createMultibindingVector<I>));
}

template <typename C>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForInstance(C& instance) {
  return std::make_tuple(getTypeId<C>(), MultibindingData(&instance, createMultibindingVector<C>));
}

template <typename Lambda>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForProvider() {
  using Signature = meta::Apply<meta::FunctionSignature, Lambda>;
  using C = typename std::remove_pointer<meta::Apply<meta::SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector) {
    C* cPtr = InvokeLambdaWithInjectedArgVector<Lambda>()(injector);
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  using Deps = meta::Apply<meta::GetClassForTypeVector, meta::Apply<meta::SignatureArgs, Signature>>;
  return std::make_tuple(getTypeId<C>(),
                         MultibindingData(create, getBindingDeps<Deps>(), InjectorStorage::createMultibindingVector<C>));
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_INJECTOR_STORAGE_DEFN_H
