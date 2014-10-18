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

#ifndef FRUIT_COMPONENT_STORAGE_TEMPLATES_H
#define FRUIT_COMPONENT_STORAGE_TEMPLATES_H

#include "injector_storage.h"
#include "demangle_type_name.h"
#include "type_info.h"
#include "fruit_assert.h"

// Not necessary, just to make KDevelop happy.
#include "component_storage.h"

namespace fruit {
namespace impl {

template <typename AnnotatedSignature>
struct BindAssistedFactory;

// Non-assisted case.
template <int numAssistedBefore, typename Arg, typename ParamTuple>
struct GetAssistedArgHelper {
  auto operator()(InjectorStorage& m, ParamTuple) -> decltype(m.get<Arg>()) {
    return m.get<Arg>();
  }
};

// Assisted case.
template <int numAssistedBefore, typename Arg, typename ParamTuple>
struct GetAssistedArgHelper<numAssistedBefore, Assisted<Arg>, ParamTuple> {
  auto operator()(InjectorStorage&, ParamTuple paramTuple) -> decltype(std::get<numAssistedBefore>(paramTuple)) {
    return std::get<numAssistedBefore>(paramTuple);
  }
};

template <int index, typename AnnotatedArgs, typename ParamTuple>
struct GetAssistedArg : public GetAssistedArgHelper<NumAssistedBefore<index, AnnotatedArgs>::value, GetNthType<index, AnnotatedArgs>, ParamTuple> {};

template <typename AnnotatedSignature, typename InjectedFunctionType, typename Sequence>
class BindAssistedFactoryHelperForValue {};

template <typename AnnotatedSignature, typename C, typename... Params, int... indexes>
class BindAssistedFactoryHelperForValue<AnnotatedSignature, C(Params...), IntList<indexes...>> {
private:
  /* std::function<C(Params...)>, C(Args...) */
  using RequiredSignature = ConstructSignature<SignatureType<AnnotatedSignature>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  
  InjectorStorage& storage;
  RequiredSignature* factory;
  
public:
  BindAssistedFactoryHelperForValue(InjectorStorage& storage, RequiredSignature* factory) 
    :storage(storage), factory(factory) {}

  C operator()(Params... params) {
      return factory(GetAssistedArg<indexes, SignatureArgs<AnnotatedSignature>, decltype(std::tie(params...))>()(storage, std::tie(params...))...);
  }
};

template <typename AnnotatedSignature, typename InjectedFunctionType, typename Sequence>
class BindAssistedFactoryHelperForPointer {};

template <typename AnnotatedSignature, typename C, typename... Params, int... indexes>
class BindAssistedFactoryHelperForPointer<AnnotatedSignature, std::unique_ptr<C>(Params...), IntList<indexes...>> {
private:
  /* std::function<std::unique_ptr<C>(Params...)>, std::unique_ptr<C>(Args...) */
  using RequiredSignature = ConstructSignature<std::unique_ptr<C>, RequiredArgsForAssistedFactory<AnnotatedSignature>>;
  
  InjectorStorage& storage;
  RequiredSignature* factory;
  
public:
  BindAssistedFactoryHelperForPointer(InjectorStorage& storage, RequiredSignature* factory) 
    :storage(storage), factory(factory) {}

  std::unique_ptr<C> operator()(Params... params) {
      return factory(GetAssistedArg<indexes, SignatureArgs<AnnotatedSignature>, decltype(std::tie(params...))>()(storage, std::tie(params...))...);
  }
};

template <typename AnnotatedSignature>
struct BindAssistedFactoryForValue : public BindAssistedFactoryHelperForValue<
      AnnotatedSignature,
      ConstructSignature<SignatureType<AnnotatedSignature>, InjectedFunctionArgsForAssistedFactory<AnnotatedSignature>>,
      GenerateIntSequence<
        list_size<
          RequiredArgsForAssistedFactory<AnnotatedSignature>
        >::value
      >> {
  BindAssistedFactoryForValue(InjectorStorage& storage, ConstructSignature<SignatureType<AnnotatedSignature>, RequiredArgsForAssistedFactory<AnnotatedSignature>>* factory) 
    : BindAssistedFactoryHelperForValue<
      AnnotatedSignature,
      ConstructSignature<SignatureType<AnnotatedSignature>, InjectedFunctionArgsForAssistedFactory<AnnotatedSignature>>,
      GenerateIntSequence<
        list_size<
          RequiredArgsForAssistedFactory<AnnotatedSignature>
        >::value
      >>(storage, factory) {}
};

template <typename AnnotatedSignature>
struct BindAssistedFactoryForPointer : public BindAssistedFactoryHelperForPointer<
      AnnotatedSignature,
      ConstructSignature<std::unique_ptr<SignatureType<AnnotatedSignature>>, InjectedFunctionArgsForAssistedFactory<AnnotatedSignature>>,
      GenerateIntSequence<
        list_size<
          RequiredArgsForAssistedFactory<AnnotatedSignature>
        >::value
      >> {
  BindAssistedFactoryForPointer(ComponentStorage& storage, ConstructSignature<std::unique_ptr<SignatureType<AnnotatedSignature>>, RequiredArgsForAssistedFactory<AnnotatedSignature>>* factory) 
    : BindAssistedFactoryHelperForPointer<
      AnnotatedSignature,
      ConstructSignature<std::unique_ptr<SignatureType<AnnotatedSignature>>, InjectedFunctionArgsForAssistedFactory<AnnotatedSignature>>,
      GenerateIntSequence<
        list_size<
          RequiredArgsForAssistedFactory<AnnotatedSignature>
        >::value
      >>(storage, factory) {}
};

template <typename C>
inline std::shared_ptr<char> ComponentStorage::createSingletonsVector(InjectorStorage& storage) {
  const TypeInfo* typeInfo = getTypeInfo<C>();
  BindingDataVectorForMultibinding* bindingDataVector = storage.getBindingDataVectorForMultibinding(typeInfo);
  if (bindingDataVector == nullptr) {
    // No multibindings.
    // We don't cache this result to remain thread-safe (as long as eager injection was performed).
    return nullptr;
  }
  if (bindingDataVector->v.get() != nullptr) {
    // Result cached, return early.
    return bindingDataVector->v;
  }
  
  storage.ensureConstructedMultibinding(*bindingDataVector);
  
  std::vector<C*> s;
  s.reserve(bindingDataVector->bindingDatas.size());
  for (const BindingData& bindingData : bindingDataVector->bindingDatas) {
    s.push_back(reinterpret_cast<C*>(bindingData.getStoredSingleton()));
  }
  
  std::shared_ptr<std::vector<C*>> vPtr = std::make_shared<std::vector<C*>>(std::move(s));
  std::shared_ptr<char> result(vPtr, reinterpret_cast<char*>(vPtr.get()));
  
  bindingDataVector->v = result;
  
  return result;
}

// I, C must not be pointers.
template <typename I, typename C>
inline void ComponentStorage::bind() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](InjectorStorage& m, BindingData::createArgument_t) {
    C* cPtr = m.get<C*>();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return std::make_pair(reinterpret_cast<BindingData::object_t>(iPtr),
                          BindingData::destroy_t(nullptr));
  };
  createBindingData(getTypeInfo<I>(), create, BindingData::createArgument_t(0));
}

template <typename C>
inline void ComponentStorage::bindInstance(C& instance) {
  createBindingData(getTypeInfo<C>(), &instance, nullptr);
}

template <typename Signature, typename Function>
struct RegisterProviderHelper {}; // Not used.

// registerProvider() implementation for a lambda that returns an object by value.
template <typename C, typename... Args, typename Function>
struct RegisterProviderHelper<C(Args...), Function> {
  inline void operator()(ComponentStorage& storage, Function f) {
    static_assert(std::is_empty<Function>::value,
                  "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
    // This is a no-op since Function is empty.
    static Function fun = f;
    
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t) {
      // The value of `arg' is probably unused, since the type of the lambda should be enough to determine the function pointer.
      C* cPtr = m.constructSingleton<C, Args...>(fun(m.get<Args>()...));
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        cPtr->C::~C();
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            std::is_trivially_destructible<C>::value
                              ? nullptr
                              : NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingData(getTypeInfo<C>(), create,
                              NormalizedComponentStorage::BindingData::createArgument_t(0));
  }
};

// registerProvider() implementation for a plain function that returns an object by value.
template <typename C, typename... Args>
struct RegisterProviderHelper<C(Args...), C(*)(Args...)> {
  inline void operator()(ComponentStorage& storage, C(*f)(Args...)) {
    if (f == nullptr) {
      storage.fatal("attempting to register nullptr as provider.");
    }
    
    using provider_type = decltype(f);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t arg) {
      provider_type provider = reinterpret_cast<provider_type>(arg);
      C* cPtr = m.constructSingleton<C, Args...>(provider(m.get<Args>()...));
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        cPtr->C::~C();
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            std::is_trivially_destructible<C>::value
                              ? nullptr
                              : NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingData(getTypeInfo<C>(), create,
                              reinterpret_cast<NormalizedComponentStorage::BindingData::createArgument_t>(f));
  }
};

// registerProvider() implementation for a lambda that returns a pointer.
template <typename C, typename... Args, typename Function>
struct RegisterProviderHelper<C*(Args...), Function> {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  
  inline void operator()(ComponentStorage& storage, Function f) {
    static_assert(std::is_empty<Function>::value,
                  "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
    // This is a no-op since Function is empty.
    static Function fun = f;
    
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t) {
      // The value of `arg' is probably unused, since the type of the lambda should be enough to determine the function pointer.
      C* cPtr = fun(m.get<Args>()...);
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get an instance for the type " + getTypeInfo<C>()->name() + " but the provider returned nullptr");
      }
      
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        delete cPtr;
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            static_cast<NormalizedComponentStorage::BindingData::destroy_t>(destroy));
    };
    storage.createBindingData(getTypeInfo<C>(), create,
                              NormalizedComponentStorage::BindingData::createArgument_t(&f));
  }
};

// registerProvider() implementation for a plain function that returns a pointer.
template <typename C, typename... Args>
struct RegisterProviderHelper<C*(Args...), C*(*)(Args...)> {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  
  inline void operator()(ComponentStorage& storage, C*(*provider)(Args...)) {
    // This can happen if the user-supplied provider returns nullptr.
    if (provider == nullptr) {
      ComponentStorage::fatal("attempting to register nullptr as provider.");
    }
    
    using provider_type = decltype(provider);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t arg) {
      provider_type provider = reinterpret_cast<provider_type>(arg);
      C* cPtr = provider(m.get<Args>()...);
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get an instance for the type " + getTypeInfo<C>()->name() + " but the provider returned nullptr");
      }
      
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        delete cPtr;
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            static_cast<NormalizedComponentStorage::BindingData::destroy_t>(destroy));
    };
    storage.createBindingData(getTypeInfo<C>(), create,
                              reinterpret_cast<NormalizedComponentStorage::BindingData::createArgument_t>(provider));
  }
};

template <typename Function>
inline void ComponentStorage::registerProvider(Function provider) {
  RegisterProviderHelper<FunctionSignature<Function>, Function>()(*this, provider);
}

template <typename C, typename... Args>
inline void ComponentStorage::registerConstructor() {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](InjectorStorage& m, BindingData::createArgument_t arg) {
    (void)arg;
    C* cPtr = m.constructSingleton<C, Args...>(m.get<Args>()...);
    auto destroy = [](BindingData::object_t p) {
      C* cPtr = reinterpret_cast<C*>(p);
      cPtr->C::~C();
    };
    return std::make_pair(reinterpret_cast<BindingData::object_t>(cPtr),
                          std::is_trivially_destructible<C>::value ? nullptr : BindingData::destroy_t(destroy));
  };
  createBindingData(getTypeInfo<C>(), create, BindingData::createArgument_t(0));
}

// I, C must not be pointers.
template <typename I, typename C>
inline void ComponentStorage::addMultibinding() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t) {
    C* cPtr = m.get<C*>();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(iPtr),
                          BindingData::destroy_t(nullptr));
  };
  createBindingDataForMultibinding(getTypeInfo<I>(), create, BindingData::createArgument_t(0), createSingletonsVector<I>);
}

template <typename C>
inline void ComponentStorage::addInstanceMultibinding(C& instance) {
  createBindingDataForMultibinding(getTypeInfo<C>(), &instance, BindingData::createArgument_t(0), createSingletonsVector<C>);
}

template <typename Signature, typename Function>
struct RegisterMultibindingProviderHelper {}; // Not used.

// registerProvider() implementation for a lambda that returns an object by value.
template <typename C, typename... Args, typename Function>
struct RegisterMultibindingProviderHelper<C(Args...), Function> {
  inline void operator()(ComponentStorage& storage, Function f) {
    static_assert(std::is_empty<Function>::value,
                  "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
    // This is a no-op since Function is empty.
    static Function fun = f;
    
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t) {
      // The value of `arg' is probably unused, since the type of the lambda should be enough to determine the function pointer.
      C* cPtr = m.constructSingleton<C, Args...>(fun(m.get<std::forward<Args>>()...));
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get a multibinding instance for the type " + getTypeInfo<C>()->name() + " but the provider returned nullptr.");
      }
          
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        delete cPtr;
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingDataForMultibinding(getTypeInfo<C>(), create,
                                    reinterpret_cast<NormalizedComponentStorage::BindingData::createArgument_t>(&f),
                                    ComponentStorage::createSingletonsVector<C>);
  }
};

// registerProvider() implementation for a plain function that returns an object by value.
template <typename C, typename... Args>
struct RegisterMultibindingProviderHelper<C(Args...), C(*)(Args...)> {
  inline void operator()(ComponentStorage& storage, C(*f)(Args...)) {
    if (f == nullptr) {
      storage.fatal("attempting to register nullptr as provider.");
    }
    
    using provider_type = decltype(f);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t arg) {
      provider_type provider = reinterpret_cast<provider_type>(arg);
      C* cPtr = m.constructSingleton<C, Args...>(provider(m.get<std::forward<Args>>()...));
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get a multibinding instance for the type " + getTypeInfo<C>()->name() + " but the provider returned nullptr.");
      }
          
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        delete cPtr;
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingDataForMultibinding(getTypeInfo<C>(), create,
                                    reinterpret_cast<NormalizedComponentStorage::BindingData::createArgument_t>(f),
                                    ComponentStorage::createSingletonsVector<C>);
  }
};

// registerProvider() implementation for a lambda that returns a pointer.
template <typename C, typename... Args, typename Function>
struct RegisterMultibindingProviderHelper<C*(Args...), Function> {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  
  inline void operator()(ComponentStorage& storage, Function f) {
    static_assert(std::is_empty<Function>::value,
                  "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
    // This is a no-op since Function is empty.
    static Function fun = f;
    
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t) {
      // The value of `arg' is probably unused, since the type of the lambda should be enough to determine the function pointer.
      C* cPtr = fun(m.get<std::forward<Args>>()...);
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get a multibinding instance for the type " + getTypeInfo<C>()->name() + " but the provider returned nullptr.");
      }
          
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        delete cPtr;
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingDataForMultibinding(getTypeInfo<C>(), create,
                                    NormalizedComponentStorage::BindingData::createArgument_t(&f),
                                    ComponentStorage::createSingletonsVector<C>);
  }
};

// registerProvider() implementation for a plain function that returns a pointer.
template <typename C, typename... Args>
struct RegisterMultibindingProviderHelper<C*(Args...), C*(*)(Args...)> {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  
  inline void operator()(ComponentStorage& storage, C*(*provider)(Args...)) {
    // This can happen if the user-supplied provider returns nullptr.
    if (provider == nullptr) {
      ComponentStorage::fatal("attempting to register nullptr as provider.");
    }
    
    using provider_type = decltype(provider);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t arg) {
      provider_type provider = reinterpret_cast<provider_type>(arg);
      C* cPtr = provider(m.get<std::forward<Args>>()...);
      
      // This can happen if the user-supplied provider returns nullptr.
      if (cPtr == nullptr) {
        ComponentStorage::fatal("attempting to get a multibinding instance for the type " + getTypeInfo<C>()->name() + " but the provider returned nullptr.");
      }
          
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        C* cPtr = reinterpret_cast<C*>(p);
        delete cPtr;
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(cPtr),
                            NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingDataForMultibinding(getTypeInfo<C>(), create,
                                    reinterpret_cast<NormalizedComponentStorage::BindingData::createArgument_t>(provider),
                                    ComponentStorage::createSingletonsVector<C>);
  }
};

template <typename Function>
inline void ComponentStorage::registerMultibindingProvider(Function provider) {
  RegisterMultibindingProviderHelper<FunctionSignature<Function>, Function>()(*this, provider);
}

template <typename AnnotatedSignature, typename InjectedSignature, typename Function>
struct RegisterFactoryHelper {}; // Not used.

// registerProvider() implementation for a lambda.
template <typename AnnotatedSignature, typename C, typename... Argz, typename Function>
struct RegisterFactoryHelper<AnnotatedSignature, C(Argz...), Function> {
  inline void operator()(ComponentStorage& storage, Function f) {
    static_assert(std::is_empty<Function>::value,
                  "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
    // This is a no-op since Function is empty.
    static Function fun = f;
    
    using fun_t = std::function<InjectedSignatureForAssistedFactory<AnnotatedSignature>>;
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t) {
      fun_t* fPtr = 
        m.constructSingleton<fun_t>(BindAssistedFactoryForValue<AnnotatedSignature>(m, fun));
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        fun_t* fPtr = reinterpret_cast<fun_t*>(p);
        fPtr->~fun_t();
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(fPtr),
                            NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingData(getTypeInfo<fun_t>(), create,
                              NormalizedComponentStorage::BindingData::createArgument_t(&f));
  }
};

// registerProvider() implementation for a plain function.
template <typename AnnotatedSignature, typename C, typename... Argz>
struct RegisterFactoryHelper<AnnotatedSignature, C(Argz...), C(*)(Argz...)> {
  inline void operator()(ComponentStorage& storage, C(*f)(Argz...)) {
    if (f == nullptr) {
      storage.fatal("attempting to register nullptr as provider.");
    }
    
    using fun_t = std::function<InjectedSignatureForAssistedFactory<AnnotatedSignature>>;
    using provider_type = decltype(f);
    auto create = [](InjectorStorage& m, NormalizedComponentStorage::BindingData::createArgument_t arg) {
      provider_type factory = reinterpret_cast<provider_type>(arg);
      fun_t* fPtr = 
        m.constructSingleton<fun_t>(BindAssistedFactoryForValue<AnnotatedSignature>(m, factory));
      auto destroy = [](NormalizedComponentStorage::BindingData::object_t p) {
        fun_t* fPtr = reinterpret_cast<fun_t*>(p);
        fPtr->~fun_t();
      };
      return std::make_pair(reinterpret_cast<NormalizedComponentStorage::BindingData::object_t>(fPtr),
                            NormalizedComponentStorage::BindingData::destroy_t(destroy));
    };
    storage.createBindingData(getTypeInfo<fun_t>(), create,
                              NormalizedComponentStorage::BindingData::createArgument_t(f));
  }
};

template <typename AnnotatedSignature, typename Function>
inline void ComponentStorage::registerFactory(Function provider) {
  RegisterFactoryHelper<AnnotatedSignature, FunctionSignature<Function>, Function>()(*this, provider);
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_COMPONENT_STORAGE_TEMPLATES_H
