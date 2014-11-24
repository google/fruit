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
#include "../metaprogramming/list.h"
#include "../metaprogramming/component.h"

#include <cassert>

// Redundant, but makes KDevelop happy.
#include "injector_storage.h"

namespace fruit {
namespace impl {

// General case, value
template <typename C>
struct GetHelper {
  C operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  C operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<const C> {
  const C operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  const C operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<std::shared_ptr<C>> {
  std::shared_ptr<C> operator()(InjectorStorage& injector) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), injector.getPtr<C>());
  }
  std::shared_ptr<C> operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<C*> {
  C* operator()(InjectorStorage& injector) {
    return injector.getPtr<C>();
  }
  C* operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return injector.getPtr<C>(deps, dep_index);
  }
};

template <typename C>
struct GetHelper<const C*> {
  const C* operator()(InjectorStorage& injector) {
    return injector.getPtr<C>();
  }
  const C* operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return injector.getPtr<C>(deps, dep_index);
  }
};

template <typename C>
struct GetHelper<C&> {
  C& operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  C& operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<const C&> {
  const C& operator()(InjectorStorage& injector) {
    return *(injector.getPtr<C>());
  }
  const C& operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return *(injector.getPtr<C>(deps, dep_index));
  }
};

template <typename C>
struct GetHelper<Provider<C>> {
  Provider<C> operator()(InjectorStorage& injector) {
    return Provider<C>(&injector, injector.lazyGetPtr<C>());
  }
  Provider<C> operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps, std::size_t dep_index) {
    return Provider<C>(&injector, injector.lazyGetPtr<C>(deps, dep_index));
  }
};

template <typename C>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr() {
  return lazyGetPtr(getTypeId<C>());
}

template <typename C>
inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  Graph::node_iterator itr = lazyGetPtr(deps, dep_index);
  assert(typeRegistry.find(getTypeId<C>()) == itr);
  assert(!(typeRegistry.end() == itr));
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
  assert(typeRegistry.find(getTypeId<C>()) == itr);
  assert(!(typeRegistry.end() == itr));
  void* p = getPtr(itr);
  return reinterpret_cast<C*>(p);
}

template <typename C>
inline C* InjectorStorage::unsafeGet() {
  void* p = unsafeGetPtr(getTypeId<C>());
  return reinterpret_cast<C*>(p);
}

inline void* InjectorStorage::getPtr(TypeId typeInfo) {
  return getPtr(lazyGetPtr(typeInfo));
}

inline void* InjectorStorage::getPtr(Graph::node_iterator itr) {
  ensureConstructed(itr);
  return itr.getNode().getStoredSingleton();
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(TypeId typeInfo) {
  return typeRegistry.at(typeInfo);
}

inline InjectorStorage::Graph::node_iterator InjectorStorage::lazyGetPtr(Graph::edge_iterator deps, std::size_t dep_index) {
  return deps.getNodeIterator(dep_index, typeRegistry);
}

inline void* InjectorStorage::unsafeGetPtr(TypeId typeInfo) {
  Graph::node_iterator itr = typeRegistry.find(typeInfo);
  if (itr == typeRegistry.end()) {
    return nullptr;
  }
  ensureConstructed(itr);
  return itr.getNode().getStoredSingleton();
}

template <typename C>
void InjectorStorage::destroySingleton(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  cPtr->C::~C();
}

template <typename C>
void InjectorStorage::destroyExternalSingleton(void* p) {
  C* cPtr = reinterpret_cast<C*>(p);
  cPtr->C::~C();
  operator delete(cPtr);
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

inline void InjectorStorage::ensureConstructed(typename SemistaticGraph<TypeId, NormalizedBindingData>::node_iterator nodeItr) {
  NormalizedBindingData& bindingData = nodeItr.getNode();
  if (!nodeItr.isTerminal()) {
    bindingData.create(*this, nodeItr.neighborsBegin());
    nodeItr.setTerminal();
  }
}

template <typename T, typename... Args>
inline T* InjectorStorage::constructObject(Args&&... args) {
  T* x = allocator.constructObject<T>(std::forward<Args>(args)...);
  if (!std::is_trivially_destructible<T>::value) {
    executeOnDestruction(&InjectorStorage::destroySingleton<T>, reinterpret_cast<void*>(x));
  }
  return x;
}

inline void InjectorStorage::executeOnDestruction(BindingData::destroy_t destroy, void* p) {
  onDestruction.emplace_back(destroy, p);
}

inline NormalizedMultibindingData* InjectorStorage::getNormalizedMultibindingData(TypeId typeInfo) {
  auto itr = typeRegistryForMultibindings.find(typeInfo);
  if (itr != typeRegistryForMultibindings.end())
    return &(itr->second);
  else
    return nullptr;
}

template <typename C>
inline std::shared_ptr<char> InjectorStorage::createSingletonsVector(InjectorStorage& storage) {
  TypeId typeInfo = getTypeId<C>();
  NormalizedMultibindingData* multibindingData = storage.getNormalizedMultibindingData(typeInfo);
  
  // This method is only called if there was at least 1 multibinding (otherwise the would-be caller would have returned nullptr
  // instead of calling this).
  assert(multibindingData != nullptr);
  
  if (multibindingData->v.get() != nullptr) {
    // Result cached, return early.
    return multibindingData->v;
  }
  
  storage.ensureConstructedMultibinding(*multibindingData);
  
  std::vector<C*> s;
  s.reserve(multibindingData->elems.size());
  for (const NormalizedMultibindingData::Elem& elem : multibindingData->elems) {
    s.push_back(reinterpret_cast<C*>(elem.object));
  }
  
  std::shared_ptr<std::vector<C*>> vPtr = std::make_shared<std::vector<C*>>(std::move(s));
  std::shared_ptr<char> result(vPtr, reinterpret_cast<char*>(vPtr.get()));
  
  multibindingData->v = result;
  
  return result;
}

template <typename Lambda, typename C, typename ArgList, typename Indexes>
struct InvokeLambdaWithInjectedArgListHelper;

template <typename Lambda, typename C, typename... Args, int... indexes>
struct InvokeLambdaWithInjectedArgListHelper<Lambda, C*, List<Args...>, IntList<indexes...>> {
  C* operator()(InjectorStorage& injector) {
    C* cPtr = LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>()...);
    injector.executeOnDestruction(&InjectorStorage::destroyExternalSingleton<C>, static_cast<void*>(cPtr));
    
    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<C>()) + " but the provider returned nullptr");
    }
    
    return cPtr;
  }
  
  C* operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;
    
    C* cPtr = LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>(deps, indexes)...);
    injector.executeOnDestruction(&InjectorStorage::destroyExternalSingleton<C>, static_cast<void*>(cPtr));
    
    // This can happen if the user-supplied provider returns nullptr.
    if (cPtr == nullptr) {
      InjectorStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<C>()) + " but the provider returned nullptr");
    }
    
    return cPtr;
  }
};

template <typename Lambda, typename C, typename... Args, int... indexes>
struct InvokeLambdaWithInjectedArgListHelper<Lambda, C, List<Args...>, IntList<indexes...>> {
  C* operator()(InjectorStorage& injector) {
    C* cPtr = injector.constructObject<C, C&&>(LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>()...));
    return cPtr;
  }
  
  C* operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;
    
    C* cPtr = injector.constructObject<C, C&&>(LambdaInvoker::invoke<Lambda, Args...>(injector.get<Args>(deps, indexes)...));    
    return cPtr;
  }
};

// The inner operator() takes an InjectorStorage& and a NormalizedComponentStorage::Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of move-constructing a C into the injector's own allocator if needed.
template <typename Lambda>
struct InvokeLambdaWithInjectedArgList 
: public InvokeLambdaWithInjectedArgListHelper<
      Lambda,
      Apply<SignatureType, Apply<FunctionSignature, Lambda>>,
      Apply<SignatureArgs, Apply<FunctionSignature, Lambda>>,
      GenerateIntSequence<
          ApplyC<
              ListApparentSize,
              Apply<SignatureArgs, Apply<FunctionSignature, Lambda>>
          >::value
      >> {};

template <typename Lambda, typename C, typename ArgList, typename Indexes>
struct InvokeConstructorWithInjectedArgListHelper;

template <typename Lambda, typename C, typename... Args, int... indexes>
struct InvokeConstructorWithInjectedArgListHelper<Lambda, C, List<Args...>, IntList<indexes...>> {
  C* operator()(InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
    // `deps' *is* used below, but when there are no Args some compilers report it as unused.
    (void)deps;
    
    C* cPtr = injector.constructObject<C, Args...>(injector.get<Args>(deps, indexes)...);
    return cPtr;
  }
};

// The inner operator() takes an InjectorStorage& and a NormalizedComponentStorage::Graph::edge_iterator (the type's deps) and
// returns the injected object as a C*.
// This takes care of allocating the required space into the injector's allocator.
template <typename Signature>
struct InvokeConstructorWithInjectedArgList
: public InvokeConstructorWithInjectedArgListHelper<
      Signature,
      Apply<SignatureType, Signature>,
      Apply<SignatureArgs, Signature>,
      GenerateIntSequence<
          ApplyC<
              ListApparentSize,
              Apply<SignatureArgs, Signature>
          >::value
      >> {};

// I, C must not be pointers.
template <typename I, typename C>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForBind() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
    C* cPtr = m.get<C*>(deps, 0);
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  return std::make_tuple(getTypeId<I>(), BindingData(create, getBindingDeps<List<C>>()));
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
  using Signature = Apply<FunctionSignature, Lambda>;
  using C = typename std::remove_pointer<Apply<SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
    C* cPtr = InvokeLambdaWithInjectedArgList<Lambda>()(injector, deps);
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  const BindingDeps* deps = getBindingDeps<Apply<GetClassForTypeList, Apply<SignatureArgs, Signature>>>();
  return std::make_tuple(getTypeId<C>(), BindingData(create, deps));
}

template <typename Lambda, typename I>
inline std::tuple<TypeId, TypeId, BindingData> InjectorStorage::createBindingDataForCompressedProvider() {
  using Signature = Apply<FunctionSignature, Lambda>;
  using C = typename std::remove_pointer<Apply<SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
    C* cPtr = InvokeLambdaWithInjectedArgList<Lambda>()(injector, deps);
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  const BindingDeps* deps = getBindingDeps<Apply<GetClassForTypeList, Apply<SignatureArgs, Signature>>>();
  return std::make_tuple(getTypeId<I>(), getTypeId<C>(), BindingData(create, deps));
}

template <typename Signature>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForConstructor() {
  using C = typename std::remove_pointer<Apply<SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
    C* cPtr = InvokeConstructorWithInjectedArgList<Signature>()(injector, deps);
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  const BindingDeps* deps = getBindingDeps<Apply<GetClassForTypeList, Apply<SignatureArgs, Signature>>>();
  return std::make_tuple(getTypeId<C>(), BindingData(create, deps));
}

template <typename Signature, typename I>
inline std::tuple<TypeId, TypeId, BindingData> InjectorStorage::createBindingDataForCompressedConstructor() {
  using C = typename std::remove_pointer<Apply<SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
    C* cPtr = InvokeConstructorWithInjectedArgList<Signature>()(injector, deps);
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  const BindingDeps* deps = getBindingDeps<Apply<GetClassForTypeList, Apply<SignatureArgs, Signature>>>();
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
  return std::make_tuple(getTypeId<I>(), MultibindingData(create, getBindingDeps<List<C>>(), createSingletonsVector<C>));
}

template <typename C>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForInstance(C& instance) {
  return std::make_tuple(getTypeId<C>(), MultibindingData(&instance, createSingletonsVector<C>));
}

template <typename Lambda>
inline std::tuple<TypeId, MultibindingData> InjectorStorage::createMultibindingDataForProvider() {
  using Signature = Apply<FunctionSignature, Lambda>;
  using C = typename std::remove_pointer<Apply<SignatureType, Signature>>::type;
  auto create = [](InjectorStorage& injector) {
    C* cPtr = InvokeLambdaWithInjectedArgList<Lambda>()(injector);
    return reinterpret_cast<BindingData::object_t>(cPtr);
  };
  using Deps = Apply<GetClassForTypeList, Apply<SignatureArgs, Signature>>;
  return std::make_tuple(getTypeId<C>(), 
                         MultibindingData(create, getBindingDeps<Deps>(), InjectorStorage::createSingletonsVector<C>));
}

// Non-assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg, typename ParamTuple>
struct GetAssistedArgHelper {
  inline auto operator()(InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps, ParamTuple)
      -> decltype(m.get<Arg>()) {
    return m.get<Arg>(deps, numNonAssistedBefore);
  }
};

// Assisted case.
template <int numAssistedBefore, int numNonAssistedBefore, typename Arg, typename ParamTuple>
struct GetAssistedArgHelper<numAssistedBefore, numNonAssistedBefore, Assisted<Arg>, ParamTuple> {
  inline auto operator()(InjectorStorage&, NormalizedComponentStorage::Graph::edge_iterator deps, ParamTuple paramTuple)
      -> decltype(std::get<numAssistedBefore>(paramTuple)) {
    (void) deps;
    return std::get<numAssistedBefore>(paramTuple);
  }
};

template <int index, typename AnnotatedArgs, typename ParamTuple>
struct GetAssistedArg : public GetAssistedArgHelper<NumAssistedBefore<index, AnnotatedArgs>::value,
                                                    index - NumAssistedBefore<index, AnnotatedArgs>::value,
                                                    GetNthType<index, AnnotatedArgs>,
                                                    ParamTuple> {};

template <typename AnnotatedSignature, typename InjectedFunctionType, typename Sequence>
class BindAssistedFactoryHelper {};

template <typename AnnotatedSignature, typename C, typename... Params, int... indexes>
class BindAssistedFactoryHelper<AnnotatedSignature, C(Params...), IntList<indexes...>> {
private:
  /* std::function<C(Params...)>, C(Args...) */
  using RequiredSignature = Apply<ConstructSignature,
                                  Apply<SignatureType, AnnotatedSignature>,
                                  Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>>;
  
  InjectorStorage& storage;
  RequiredSignature* factory;
  NormalizedComponentStorage::Graph::edge_iterator deps;
  
public:
  BindAssistedFactoryHelper(InjectorStorage& storage, RequiredSignature* factory,
                            NormalizedComponentStorage::Graph::edge_iterator deps) 
    :storage(storage), factory(factory), deps(deps) {}

  inline C operator()(Params... params) {
      return factory(GetAssistedArg<indexes,
                                    Apply<SignatureArgs, AnnotatedSignature>,
                                    decltype(std::tie(params...))
                                   >()(storage, deps, std::tie(params...))
                     ...);
  }
};

// TODO: Rename to a more meaningful name, or consider inlining.
template <typename AnnotatedSignature>
struct BindAssistedFactory : public BindAssistedFactoryHelper<
      AnnotatedSignature,
      Apply<ConstructSignature,
            Apply<SignatureType, AnnotatedSignature>,
            Apply<InjectedFunctionArgsForAssistedFactory, AnnotatedSignature>>,
      GenerateIntSequence<
        ApplyC<ListSize,
               Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>
        >::value
      >> {
  inline BindAssistedFactory(InjectorStorage& storage,
                             Apply<ConstructSignature,
                                   Apply<SignatureType, AnnotatedSignature>,
                                   Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>
                                   >* factory,
                      NormalizedComponentStorage::Graph::edge_iterator deps) 
    : BindAssistedFactoryHelper<
      AnnotatedSignature,
      Apply<ConstructSignature,
            Apply<SignatureType, AnnotatedSignature>,
            Apply<InjectedFunctionArgsForAssistedFactory, AnnotatedSignature>>,
      GenerateIntSequence<
        ApplyC<ListSize,
          Apply<RequiredArgsForAssistedFactory, AnnotatedSignature>
        >::value
      >>(storage, factory, deps) {}
};

template <typename AnnotatedSignature, typename InjectedSignature, typename Lambda>
struct CreateBindingDataForFactoryHelper;

// registerProvider() implementation for a lambda.
template <typename AnnotatedSignature, typename C, typename... Argz, typename Lambda>
struct CreateBindingDataForFactoryHelper<AnnotatedSignature, C(Argz...), Lambda> {
  inline std::tuple<TypeId, BindingData> operator()() {
    using fun_t = std::function<Apply<InjectedSignatureForAssistedFactory, AnnotatedSignature>>;
    auto create = [](InjectorStorage& injector, NormalizedComponentStorage::Graph::edge_iterator deps) {
      fun_t* fPtr =
        injector.constructObject<fun_t>(BindAssistedFactory<AnnotatedSignature>(injector, LambdaInvoker::invoke<Lambda, Argz...>, deps));
      return reinterpret_cast<BindingData::object_t>(fPtr);
    };
    const BindingDeps* deps = getBindingDeps<Apply<RemoveAssisted, Apply<SignatureArgs, AnnotatedSignature>>>();
    return std::make_tuple(getTypeId<fun_t>(), BindingData(create, deps));
  }
};

template <typename AnnotatedSignature, typename Lambda>
inline std::tuple<TypeId, BindingData> InjectorStorage::createBindingDataForFactory() {
  return CreateBindingDataForFactoryHelper<AnnotatedSignature, Apply<FunctionSignature, Lambda>, Lambda>()();
}


} // namespace fruit
} // namespace impl


#endif // FRUIT_INJECTOR_STORAGE_DEFN_H
