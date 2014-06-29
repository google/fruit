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

#ifndef FRUIT_UNSAFE_MODULE_TEMPLATES_H
#define FRUIT_UNSAFE_MODULE_TEMPLATES_H

#include "metaprogramming.h"
#include "demangle_type_name.h"
#include "type_info.h"
#include "fruit_assert.h"

// Redundant, but makes KDevelop happy.
#include "component_storage.h"

namespace fruit {
namespace impl {

template <typename AnnotatedSignature>
struct BindAssistedFactory;

// General case, value
template <typename C>
struct GetHelper {
  C operator()(ComponentStorage& component) {
    return *(component.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<const C> {
  const C operator()(ComponentStorage& component) {
    return *(component.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<std::shared_ptr<C>> {
  std::shared_ptr<C> operator()(ComponentStorage& component) {
    return std::shared_ptr<C>(std::shared_ptr<char>(), component.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<C*> {
  C* operator()(ComponentStorage& component) {
    return component.getPtr<C>();
  }
};

template <typename C>
struct GetHelper<const C*> {
  const C* operator()(ComponentStorage& component) {
    return component.getPtr<C>();
  }
};

template <typename C>
struct GetHelper<C&> {
  C& operator()(ComponentStorage& component) {
    return *(component.getPtr<C>());
  }
};

template <typename C>
struct GetHelper<const C&> {
  const C& operator()(ComponentStorage& component) {
    return *(component.getPtr<C>());
  }
};

template <typename... Ps>
struct GetHelper<Provider<Ps...>> {
  Provider<Ps...> operator()(ComponentStorage& storage) {
    return Provider<Ps...>(&storage);
  }
};

// Non-assisted case.
template <int numAssistedBefore, typename Arg, typename ParamTuple>
struct GetAssistedArgHelper {
  auto operator()(ComponentStorage& m, ParamTuple) -> decltype(m.get<Arg>()) {
    return m.get<Arg>();
  }
};

// Assisted case.
template <int numAssistedBefore, typename Arg, typename ParamTuple>
struct GetAssistedArgHelper<numAssistedBefore, Assisted<Arg>, ParamTuple> {
  auto operator()(ComponentStorage&, ParamTuple paramTuple) -> decltype(std::get<numAssistedBefore>(paramTuple)) {
    return std::get<numAssistedBefore>(paramTuple);
  }
};

template <int index, typename AnnotatedArgs, typename ParamTuple>
struct GetAssistedArg : public GetAssistedArgHelper<NumAssistedBefore<index, AnnotatedArgs>::value, GetNthType<index, AnnotatedArgs>, ParamTuple> {};

template <typename AnnotatedSignature, typename InjectedFunctionType, typename Sequence>
class BindAssistedFactoryHelper {};

template <typename AnnotatedSignature, typename C, typename... Params, int... indexes>
class BindAssistedFactoryHelper<AnnotatedSignature, C(Params...), IntList<indexes...>> {
private:
  /* std::function<C(Params...)>, C(Args...) */
  using RequiredSignature = RequiredSignatureForAssistedFactory<AnnotatedSignature>;
  
  ComponentStorage& storage;
  RequiredSignature* factory;
  
public:
  BindAssistedFactoryHelper(ComponentStorage& storage, RequiredSignature* factory) 
    :storage(storage), factory(factory) {}

  C operator()(Params... params) {
      return factory(GetAssistedArg<indexes, SignatureArgs<AnnotatedSignature>, decltype(std::tie(params...))>()(storage, std::tie(params...))...);
  }
};

template <typename AnnotatedSignature>
struct BindAssistedFactory : public BindAssistedFactoryHelper<
      AnnotatedSignature,
      InjectedFunctionTypeForAssistedFactory<AnnotatedSignature>,
      GenerateIntSequence<
        list_size<
          SignatureArgs<RequiredSignatureForAssistedFactory<AnnotatedSignature>>
        >::value
      >> {
  BindAssistedFactory(ComponentStorage& storage, RequiredSignatureForAssistedFactory<AnnotatedSignature>* factory) 
    : BindAssistedFactoryHelper<
      AnnotatedSignature,
      InjectedFunctionTypeForAssistedFactory<AnnotatedSignature>,
      GenerateIntSequence<
        list_size<
          SignatureArgs<RequiredSignatureForAssistedFactory<AnnotatedSignature>>
        >::value
      >>(storage, factory) {}
};


template <typename MessageGenerator>
inline void ComponentStorage::check(bool b, MessageGenerator messageGenerator) {
  if (!b) {
    printError(messageGenerator());
    abort();
  }
}

template <typename C>
inline void ComponentStorage::createTypeInfo(void* (*create)(ComponentStorage&, void*),
                                             void* createArgument,
                                             void (*deleteOperation)(void*)) {
  createTypeInfo(getTypeIndex<C>(), create, createArgument, deleteOperation);
}

template <typename C>
inline void ComponentStorage::createTypeInfo(void* instance,
                                             void (*destroy)(void*)) {
  createTypeInfo(getTypeIndex<C>(), instance, destroy);
}

template <typename C>
inline void ComponentStorage::createTypeInfoForMultibinding(void* (*create)(ComponentStorage&, void*),
                                                         void* createArgument,
                                                         void (*deleteOperation)(void*)) {
  createTypeInfoForMultibinding(getTypeIndex<C>(), create, createArgument, deleteOperation);
}

template <typename C>
inline void ComponentStorage::createTypeInfoForMultibinding(void* instance,
                                                         void (*destroy)(void*)) {
  createTypeInfoForMultibinding(getTypeIndex<C>(), instance, destroy);
}

template <typename C>
inline C* ComponentStorage::getPtr() {
  void* p = getPtr(getTypeIndex<C>());
  return reinterpret_cast<C*>(p);
}

// I, C must not be pointers.
template <typename I, typename C>
inline void ComponentStorage::bind() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](ComponentStorage& m, void*) {
    C* cPtr = m.getPtr<C>();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<void*>(iPtr);
  };
  createTypeInfo<I>(create, nullptr, nopDeleter);
}

template <typename C>
inline void ComponentStorage::bindInstance(C& instance) {
  createTypeInfo<C>(&instance, nopDeleter);
}

template <typename C, typename... Args>
inline void ComponentStorage::registerProvider(C* (*provider)(Args...), void (*deleter)(void*)) {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  check(provider != nullptr, "attempting to register nullptr as provider");
  using provider_type = decltype(provider);
  auto create = [](ComponentStorage& m, void* arg) {
    provider_type provider = reinterpret_cast<provider_type>(arg);
    C* cPtr = provider(m.get<Args>()...);
    return reinterpret_cast<void*>(cPtr);
  };
  createTypeInfo<C>(create, reinterpret_cast<void*>(provider), deleter);
}

template <typename C, typename... Args>
inline void ComponentStorage::registerProvider(C (*provider)(Args...), void (*deleter)(void*)) {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  // TODO: Move this check into ComponentImpl.
  static_assert(std::is_move_constructible<C>::value, "C should be movable");
  check(provider != nullptr, "attempting to register nullptr as provider");
  using provider_type = decltype(provider);
  auto create = [](ComponentStorage& m, void* arg) {
    provider_type provider = reinterpret_cast<provider_type>(arg);
    C* cPtr = new C(provider(m.get<Args>()...));
    return reinterpret_cast<void*>(cPtr);
  };
  createTypeInfo<C>(create, reinterpret_cast<void*>(provider), deleter);
}

// I, C must not be pointers.
template <typename I, typename C>
inline void ComponentStorage::addMultibinding() {
  FruitStaticAssert(!std::is_pointer<I>::value, "I should not be a pointer");
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  auto create = [](ComponentStorage& m, void*) {
    C* cPtr = m.getPtr<C>();
    // This step is needed when the cast C->I changes the pointer
    // (e.g. for multiple inheritance).
    I* iPtr = static_cast<I*>(cPtr);
    return reinterpret_cast<void*>(iPtr);
  };
  createTypeInfoForMultibinding<I>(create, nullptr, nopDeleter);
}

template <typename C>
inline void ComponentStorage::addInstanceMultibinding(C& instance) {
  createTypeInfoForMultibinding<C>(&instance, nopDeleter);
}

template <typename C, typename... Args>
inline void ComponentStorage::registerMultibindingProvider(C* (*provider)(Args...), void (*deleter)(void*)) {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  check(provider != nullptr, "attempting to register nullptr as provider");
  using provider_type = decltype(provider);
  auto create = [](ComponentStorage& m, void* arg) {
    provider_type provider = reinterpret_cast<provider_type>(arg);
    C* cPtr = provider(m.get<Args>()...);
    return reinterpret_cast<void*>(cPtr);
  };
  createTypeInfoForMultibinding<C>(create, reinterpret_cast<void*>(provider), deleter);
}

template <typename C, typename... Args>
inline void ComponentStorage::registerMultibindingProvider(C (*provider)(Args...), void (*deleter)(void*)) {
  FruitStaticAssert(!std::is_pointer<C>::value, "C should not be a pointer");
  // TODO: Move this check into ComponentImpl.
  static_assert(std::is_move_constructible<C>::value, "C should be movable");
  check(provider != nullptr, "attempting to register nullptr as provider");
  using provider_type = decltype(provider);
  auto create = [](ComponentStorage& m, void* arg) {
    provider_type provider = reinterpret_cast<provider_type>(arg);
    C* cPtr = new C(provider(m.get<Args>()...));
    return reinterpret_cast<void*>(cPtr);
  };
  createTypeInfoForMultibinding<C>(create, reinterpret_cast<void*>(provider), deleter);
}

template <typename AnnotatedSignature>
inline void ComponentStorage::registerFactory(RequiredSignatureForAssistedFactory<AnnotatedSignature>* factory) {
  check(factory != nullptr, "attempting to register nullptr as factory");
  using Signature = RequiredSignatureForAssistedFactory<AnnotatedSignature>;
  using InjectedFunctionType = InjectedFunctionTypeForAssistedFactory<AnnotatedSignature>;
  auto create = [](ComponentStorage& m, void* arg) {
    Signature* factory = reinterpret_cast<Signature*>(arg);
    std::function<InjectedFunctionType>* fPtr = 
        new std::function<InjectedFunctionType>(BindAssistedFactory<AnnotatedSignature>(m, factory));
    return reinterpret_cast<void*>(fPtr);
  };
  createTypeInfo<std::function<InjectedFunctionType>>(create, reinterpret_cast<void*>(factory), SimpleDeleter<std::function<InjectedFunctionType>>::f);
}

template <typename C>
std::set<C*> ComponentStorage::getMultibindings() {
  TypeIndex typeIndex = getTypeIndex<C>();
  std::set<C*> bindings;
  for (ComponentStorage* storage = this; storage != nullptr; storage = storage->parent) {
    std::unordered_map<TypeIndex, std::set<TypeInfo>>::iterator itr = storage->typeRegistryForMultibindings.find(typeIndex);
    if (itr == storage->typeRegistryForMultibindings.end()) {
      // Not registered here, try the parents (if any).
      continue;
    }
    storage->ensureConstructedMultibinding(typeIndex, itr->second);
    for (const TypeInfo& typeInfo : itr->second) {
      bindings.insert(reinterpret_cast<C*>(typeInfo.storedSingleton));
    }
  }
  return bindings;
}

template <typename C>
inline ComponentStorage::TypeInfo& ComponentStorage::getTypeInfo() {
  TypeIndex typeIndex = getTypeIndex<C>();
  auto itr = typeRegistry.find(typeIndex);
  FruitCheck(itr != typeRegistry.end(), [=](){return "attempting to getTypeInfo() on a non-registered type: " + demangleTypeName(typeIndex.name());});
  return itr->second;
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_UNSAFE_MODULE_TEMPLATES_H
