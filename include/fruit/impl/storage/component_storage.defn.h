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

#ifndef FRUIT_COMPONENT_STORAGE_DEFN_H
#define FRUIT_COMPONENT_STORAGE_DEFN_H

#include "injector_storage.h"
#include "../util/demangle_type_name.h"
#include "../util/type_info.h"
#include "../fruit_assert.h"
#include "../metaprogramming/list.h"
#include "../util/lambda_invoker.h"
#include "../../component.h"

// Not necessary, just to make KDevelop happy.
#include "component_storage.h"

namespace fruit {
namespace impl {

template <typename Types>
struct GetBindingDepsForList;

template <typename... Types>
struct GetBindingDepsForList<List<Types...>> {
  inline const BindingDeps* operator()() {
    return getBindingDeps<Apply<GetClassForType, Types>...>();
  }
};

template <typename AnnotatedSignature>
struct BindAssistedFactory;

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

template <typename C>
inline std::shared_ptr<char> ComponentStorage::createSingletonsVector(InjectorStorage& storage) {
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

// I, C must not be pointers.
template <typename I, typename C>
inline void ComponentStorage::bind() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
    C* cPtr = m.get<C*>(deps, 0);
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<BindingData::object_t>(iPtr);
  };
  createBindingData(getTypeId<I>(), BindingData(create, getBindingDeps<C>()));
}

template <typename C>
inline void ComponentStorage::bindInstance(C& instance) {
  createBindingData(getTypeId<C>(), BindingData(&instance));
}

template <typename Signature, typename Indexes, typename Function, typename OptionalI>
struct RegisterProviderHelper;

// registerProvider() implementation for a lambda that returns an object by value.
template <typename C, typename... Args, int... indexes, typename Function>
struct RegisterProviderHelper<C(Args...), IntList<indexes...>, Function, None> {
  inline void operator()(ComponentStorage& storage) {
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
      // `deps' *is* used below, but when there are no Args some compilers report it as unused.
      (void)deps;
      // The value of `arg' is probably unused, since the type of the lambda should be enough to determine the function pointer.
      C* cPtr = m.constructSingleton<C, Args...>(LambdaInvoker::invoke<Function, Args...>(m.get<Args>(deps, indexes)...));
      if (!std::is_trivially_destructible<C>::value) {
        m.executeOnDestruction(&InjectorStorage::destroySingleton<C>, static_cast<void*>(cPtr));
      }
      return reinterpret_cast<BindingData::object_t>(cPtr);
    };
    storage.createBindingData(getTypeId<C>(), BindingData(create, GetBindingDepsForList<List<Args...>>()()));
  }
};

template <typename C, typename... Args, int... indexes, typename Function, typename I>
struct RegisterProviderHelper<C(Args...), IntList<indexes...>, Function, I> {
  inline void operator()(ComponentStorage& storage) {
    RegisterProviderHelper<C(Args...), IntList<indexes...>, Function, None>()(storage);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
      // `deps' *is* used below, but when there are no Args some compilers report it as unused.
      (void)deps;
      C* cPtr = m.constructSingleton<C, Args...>(LambdaInvoker::invoke<Function, Args...>(m.get<Args>(deps, indexes)...));
      I* iPtr = static_cast<I*>(cPtr);
      if (!std::is_trivially_destructible<C>::value) {
        m.executeOnDestruction(&InjectorStorage::destroySingleton<C>, static_cast<void*>(cPtr));
      }
      return reinterpret_cast<BindingData::object_t>(iPtr);
    };
    BindingData bindingData(create, GetBindingDepsForList<List<Args...>>()());
    storage.compressedBindings.push_back(CompressedBinding{getTypeId<I>(), getTypeId<C>(), bindingData});
  }
};

// registerProvider() implementation for a lambda that returns a pointer.
template <typename C, typename... Args, int... indexes, typename Function>
struct RegisterProviderHelper<C*(Args...), IntList<indexes...>, Function, None> {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  
  inline void operator()(ComponentStorage& storage) {
    static_assert(std::is_empty<Function>::value,
                  "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
    
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
      // `deps' *is* used below, but when there are no Args some compilers report it as unused.
      (void)deps;
      
      C* cPtr = LambdaInvoker::invoke<Function, Args...>(m.get<Args>(deps, indexes)...);
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<C>()) + " but the provider returned nullptr");
      }
      
      m.executeOnDestruction(&InjectorStorage::destroyExternalSingleton<C>, static_cast<void*>(cPtr));
      return reinterpret_cast<BindingData::object_t>(cPtr);
    };
    storage.createBindingData(getTypeId<C>(), BindingData(create, GetBindingDepsForList<List<Args...>>()()));
  }
};

// registerProvider() implementation for a lambda that returns a pointer.
template <typename C, typename... Args, int... indexes, typename Function, typename I>
struct RegisterProviderHelper<C*(Args...), IntList<indexes...>, Function, I> {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  
  inline void operator()(ComponentStorage& storage) {
    RegisterProviderHelper<C*(Args...), IntList<indexes...>, Function, None>()(storage);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
      // `deps' *is* used below, but when there are no Args some compilers report it as unused.
      (void)deps;
      
      C* cPtr = LambdaInvoker::invoke<Function, Args...>(m.get<Args>(deps, indexes)...);
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get an instance for the type " + std::string(getTypeId<C>()) + " but the provider returned nullptr");
      }
      
      I* iPtr = static_cast<I*>(cPtr);
      
      m.executeOnDestruction(&InjectorStorage::destroyExternalSingleton<C>, static_cast<void*>(cPtr));
      return reinterpret_cast<BindingData::object_t>(iPtr);
    };
    storage.createBindingData(getTypeId<C>(), BindingData(create, GetBindingDepsForList<List<Args...>>()()));
  }
};

template <typename Function, typename OptionalI>
inline void ComponentStorage::registerProvider() {
  RegisterProviderHelper<Apply<FunctionSignature, Function>,
                         GenerateIntSequence<ApplyC<ListApparentSize, 
                                                    Apply<SignatureArgs, Apply<FunctionSignature, Function>>>::value>,
                         Function,
                         OptionalI>()(*this);
}

template <typename Indexes, typename C, typename OptionalI, typename... Args>
struct RegisterConstructorHelper;

template <int... indexes, typename C, typename... Args>
struct RegisterConstructorHelper<IntList<indexes...>, C, None, Args...> {
  inline void operator()(ComponentStorage& storage) {
    FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
      // To avoid an `unused variable' warning if there are no Args.
      (void) deps;
      C* cPtr = m.constructSingleton<C, Args...>(m.get<Args>(deps, indexes)...);
      if (!std::is_trivially_destructible<C>::value) {
        m.executeOnDestruction(&InjectorStorage::destroySingleton<C>, static_cast<void*>(cPtr));
      }
      return reinterpret_cast<BindingData::object_t>(cPtr);
    };
    storage.createBindingData(getTypeId<C>(), BindingData(create, GetBindingDepsForList<List<Apply<GetClassForType, Args>...>>()()));
  }
};

template <int... indexes, typename C, typename I, typename... Args>
struct RegisterConstructorHelper<IntList<indexes...>, C, I, Args...> {
  inline void operator()(ComponentStorage& storage) {
    RegisterConstructorHelper<IntList<indexes...>, C, None, Args...>()(storage);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
      // To avoid an `unused variable' warning if there are no Args.
      (void) deps;
      C* cPtr = m.constructSingleton<C, Args...>(m.get<Args>(deps, indexes)...);
      I* iPtr = static_cast<I*>(cPtr);
      return reinterpret_cast<BindingData::object_t>(iPtr);
      if (!std::is_trivially_destructible<C>::value) {
        m.executeOnDestruction(&InjectorStorage::destroySingleton<C>, reinterpret_cast<void*>(cPtr));
      }
    };
    BindingData bindingData(create, GetBindingDepsForList<List<Apply<GetClassForType, Args>...>>()());
    storage.compressedBindings.push_back(CompressedBinding{getTypeId<I>(), getTypeId<C>(), bindingData});
  }
};

template <typename C, typename OptionalI, typename... Args>
inline void ComponentStorage::registerConstructor() {
  RegisterConstructorHelper<GenerateIntSequence<sizeof...(Args)>,
                            C,
                            OptionalI,
                            Args...>()(*this);
}

// I, C must not be pointers.
template <typename I, typename C>
inline void ComponentStorage::addMultibinding() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](InjectorStorage& m) {
    C* cPtr = m.get<C*>();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return std::make_pair(reinterpret_cast<MultibindingData::object_t>(iPtr), MultibindingData::destroy_t(nullptr));
  };
  createMultibindingData(getTypeId<I>(), create, createSingletonsVector<C>);
  multibindingDeps.push_back(getTypeId<C>());
}

template <typename C>
inline void ComponentStorage::addInstanceMultibinding(C& instance) {
  createMultibindingData(getTypeId<C>(), &instance, MultibindingData::destroy_t(nullptr),
                                   createSingletonsVector<C>);
}

template <typename Signature, typename Function>
struct RegisterMultibindingProviderHelper {}; // Not used.

// registerProvider() implementation for a lambda that returns an object by value.
template <typename C, typename... Args, typename Function>
struct RegisterMultibindingProviderHelper<C(Args...), Function> {
  inline void operator()(ComponentStorage& storage) {
    auto create = [](InjectorStorage& m) {
      C* cPtr = m.constructSingleton<C, Args...>(LambdaInvoker::invoke<Function, Args...>(m.get<std::forward<Args>>()...));
      
      auto destroy = [](BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        cPtr->C::~C();
      };
      return std::make_pair(reinterpret_cast<BindingData::object_t>(cPtr), MultibindingData::destroy_t(destroy));
    };
    storage.createMultibindingData(getTypeId<C>(), create,
                                    ComponentStorage::createSingletonsVector<C>);
    for (TypeId typeId : std::initializer_list<TypeId>{getTypeId<Apply<GetClassForType, Args>>()...}) {
      storage.multibindingDeps.push_back(typeId);
    }
  }
};

// registerProvider() implementation for a lambda that returns a pointer.
template <typename C, typename... Args, typename Function>
struct RegisterMultibindingProviderHelper<C*(Args...), Function> {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  
  inline void operator()(ComponentStorage& storage) {
    auto create = [](InjectorStorage& m) {
      C* cPtr = LambdaInvoker::invoke<Function, Args...>(m.get<std::forward<Args>>()...);
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get a multibinding instance for the type " + std::string(getTypeId<C>()) + " but the provider returned nullptr.");
      }
          
      auto destroy = [](MultibindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        delete cPtr;
      };
      return std::make_pair(reinterpret_cast<BindingData::object_t>(cPtr), MultibindingData::destroy_t(destroy));
    };
    storage.createMultibindingData(getTypeId<C>(), create, ComponentStorage::createSingletonsVector<C>);
    for (TypeId typeId : std::initializer_list<TypeId>{getTypeId<Apply<GetClassForType, Args>>()...}) {
      storage.multibindingDeps.push_back(typeId);
    }
  }
};

template <typename Function>
inline void ComponentStorage::registerMultibindingProvider() {
  RegisterMultibindingProviderHelper<Apply<FunctionSignature, Function>, Function>()(*this);
}

template <typename AnnotatedSignature, typename InjectedSignature, typename Function>
struct RegisterFactoryHelper {}; // Not used.

// registerProvider() implementation for a lambda.
template <typename AnnotatedSignature, typename C, typename... Argz, typename Function>
struct RegisterFactoryHelper<AnnotatedSignature, C(Argz...), Function> {
  inline void operator()(ComponentStorage& storage) {    
    using fun_t = std::function<Apply<InjectedSignatureForAssistedFactory, AnnotatedSignature>>;
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::Graph::edge_iterator deps) {
      fun_t* fPtr = 
        m.constructSingleton<fun_t>(BindAssistedFactory<AnnotatedSignature>(m, LambdaInvoker::invoke<Function, Argz...>, deps));
      m.executeOnDestruction(&InjectorStorage::destroySingleton<fun_t>, static_cast<void*>(fPtr));
      return reinterpret_cast<BindingData::object_t>(fPtr);
    };
    storage.createBindingData(getTypeId<fun_t>(), 
                              BindingData(create, 
                                          GetBindingDepsForList<
                                              Apply<RemoveAssisted, Apply<SignatureArgs, AnnotatedSignature>>>()()));
  }
};

template <typename AnnotatedSignature, typename Function>
inline void ComponentStorage::registerFactory() {
  RegisterFactoryHelper<AnnotatedSignature, Apply<FunctionSignature, Function>, Function>()(*this);
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_COMPONENT_STORAGE_DEFN_H
