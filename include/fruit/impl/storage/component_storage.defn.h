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

template <typename I, typename C>
inline void ComponentStorage::bind() {
  addBindingData(InjectorStorage::createBindingDataForBind<I, C>());
}

template <typename C>
inline void ComponentStorage::bindInstance(C& instance) {
  addBindingData(InjectorStorage::createBindingDataForBindInstance<C>(instance));
}

template <typename Lambda, typename OptionalI>
struct RegisterProviderHelper {
  inline void operator()(ComponentStorage& component) {
    RegisterProviderHelper<Lambda, None>()(component);
    
    component.addCompressedBindingData(InjectorStorage::createBindingDataForCompressedProvider<Lambda, OptionalI>());
  }
};

template <typename Lambda>
struct RegisterProviderHelper<Lambda, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBindingData(InjectorStorage::createBindingDataForProvider<Lambda>());
  }
};

template <typename Lambda, typename OptionalI>
inline void ComponentStorage::registerProvider() {
  RegisterProviderHelper<Lambda, OptionalI>()(*this);
}

template <typename Signature, typename OptionalI>
struct RegisterConstructorHelper {
  inline void operator()(ComponentStorage& component) {
    RegisterConstructorHelper<Signature, None>()(component);
    
    component.addCompressedBindingData(InjectorStorage::createBindingDataForCompressedConstructor<Signature, OptionalI>());
  }
};

template <typename Signature>
struct RegisterConstructorHelper<Signature, None> {
  inline void operator()(ComponentStorage& component) {
    component.addBindingData(InjectorStorage::createBindingDataForConstructor<Signature>());
  }
};

template <typename Signature, typename OptionalI>
inline void ComponentStorage::registerConstructor() {
  RegisterConstructorHelper<Signature, OptionalI>()(*this);
}

template <typename I, typename C>
inline void ComponentStorage::addMultibinding() {
  addMultibindingData(InjectorStorage::createMultibindingDataForBinding<I, C>());
}

template <typename C>
inline void ComponentStorage::addInstanceMultibinding(C& instance) {
  addMultibindingData(InjectorStorage::createMultibindingDataForInstance<C>(instance));
}

template <typename Lambda>
inline void ComponentStorage::registerMultibindingProvider() {
  addMultibindingData(InjectorStorage::createMultibindingDataForProvider<Lambda>());
}

template <typename AnnotatedSignature, typename Lambda>
inline void ComponentStorage::registerFactory() {
  addBindingData(InjectorStorage::createBindingDataForFactory<AnnotatedSignature, Lambda>());
}

} // namespace fruit
} // namespace impl


#endif // FRUIT_COMPONENT_STORAGE_DEFN_H
