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

#ifndef FRUIT_INJECTOR_DEFN_H
#define FRUIT_INJECTOR_DEFN_H

#include "../component.h"

// Redundant, but makes KDevelop happy.
#include "../injector.h"

namespace fruit {

template <typename... P>
inline Injector<P...>::Injector(Component<P...> component)
  : storage(new fruit::impl::InjectorStorage(std::move(component.storage),
                                             std::initializer_list<fruit::impl::TypeId>{fruit::impl::getTypeId<P>()...})) {
};

template <typename... P>
template <typename... NormalizedComponentParams, typename... ComponentParams>
inline Injector<P...>::Injector(const NormalizedComponent<NormalizedComponentParams...>& normalized_component,
                                Component<ComponentParams...> component)
  : storage(new fruit::impl::InjectorStorage(normalized_component.storage,
                                             std::move(component.storage), 
                                             fruit::impl::getTypeIdsForList<fruit::impl::meta::Apply<
                                                 fruit::impl::meta::ConcatVectors,
                                                 typename fruit::impl::meta::Apply<fruit::impl::meta::ConstructComponentImpl, ComponentParams...>::Ps,
                                                 typename fruit::impl::meta::Apply<fruit::impl::meta::ConstructComponentImpl, NormalizedComponentParams...>::Ps
                                             >>())) {
    
  using namespace fruit::impl;
  using namespace fruit::impl::meta;
    
  using Comp = meta::Apply<meta::ConstructComponentImpl, ComponentParams...>;
  FruitDelegateCheck(CheckIfError<Comp>);
  using E1 = Eval<std::conditional<!meta::Apply<meta::IsEmptyVector, typename Comp::Rs>::value,
      Apply<ConstructErrorWithArgVector, ComponentWithRequirementsInInjectorErrorTag, typename Comp::Rs>,
      void
  >>;
  FruitDelegateCheck(CheckIfError<E1>);
  
  using NormalizedComp = meta::Apply<meta::ConstructComponentImpl, NormalizedComponentParams...>;
  FruitDelegateCheck(CheckIfError<NormalizedComp>);
  
  using Op = meta::Apply<InstallComponent<NormalizedComp>, Comp>;
  FruitDelegateCheck(CheckIfError<typename Op::Result>);
  
  // The calculation of MergedComp will also do some checks, e.g. multiple bindings for the same type.
  using MergedComp = typename Op::Result;
  
  using TypesNotProvided = meta::Apply<meta::SetDifference,
                                       meta::Vector<P...>,
                                       typename MergedComp::Ps>;
  using E2 = Eval<std::conditional<!Apply<IsEmptyVector, typename MergedComp::Rs>::value,
      Apply<ConstructErrorWithArgVector, UnsatisfiedRequirementsInNormalizedComponentErrorTag, typename MergedComp::Rs>,
      Eval<std::conditional<!Apply<IsEmptyVector, TypesNotProvided>::value,
        Apply<ConstructErrorWithArgVector, TypesInInjectorNotProvidedErrorTag, TypesNotProvided>,
        void
      >>
  >>;
  FruitDelegateCheck(CheckIfError<E2>);
}

template <typename... P>
template <typename T>
inline T Injector<P...>::get() {
  using namespace fruit::impl;
  using namespace fruit::impl::meta;

  using E = Eval<std::conditional<!meta::Apply<meta::IsInVector, meta::Apply<meta::GetClassForType, T>, typename Comp::Ps>::value,
      Error<TypeNotProvidedErrorTag, T>,
      void
  >>;
  FruitDelegateCheck(CheckIfError<E>);
  return storage->template get<T>();
}

template <typename... P>
template <typename C>
inline C* Injector<P...>::unsafeGet() {
  return storage->template unsafeGet<C>();
}

template <typename... P>
template <typename T>
inline Injector<P...>::operator T() {
  return get<T>();
}

template <typename... P>
template <typename C>
inline const std::vector<C*>& Injector<P...>::getMultibindings() {
  return storage->template getMultibindings<C>();
}

template <typename... P>
inline void Injector<P...>::eagerlyInjectAll() {
  // Eagerly inject normal bindings.
  void* unused[] = {reinterpret_cast<void*>(storage->template get<P*>())...};
  (void)unused;
  
  storage->eagerlyInjectMultibindings();
}

} // namespace fruit


#endif // FRUIT_INJECTOR_DEFN_H
