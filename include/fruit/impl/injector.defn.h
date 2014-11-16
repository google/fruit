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
inline Injector<P...>::Injector(const NormalizedComponent<NormalizedComponentParams...>& normalizedComponent,
                                Component<ComponentParams...> component)
  : storage(new fruit::impl::InjectorStorage(normalizedComponent.storage,
                                             std::move(component.storage), 
                                             // TODO: Remove requirements from here. It still works but they shouldn't be here.
                                             std::initializer_list<fruit::impl::TypeId>{fruit::impl::getTypeId<ComponentParams>()...})) {
    
  using namespace fruit::impl;
    
  using Comp = Apply<ConstructComponentImpl, ComponentParams...>;
  FruitDelegateCheck(ComponentWithRequirementsInInjectorErrorHelper<typename Comp::Rs>);
  
  using NormalizedComp = Apply<ConstructComponentImpl, NormalizedComponentParams...>;
  
  // The calculation of MergedComp will also do some checks, e.g. multiple bindings for the same type.
  using MergedComp = typename InstallComponent<NormalizedComp, Comp>::Result;
  
  FruitDelegateCheck(UnsatisfiedRequirementsInNormalizedComponentHelper<typename MergedComp::Rs>);
  FruitDelegateCheck(TypesInInjectorNotProvidedHelper<Apply<SetDifference,
                                                            List<P...>,
                                                            typename MergedComp::Ps>>);
}

template <typename... P>
template <typename T>
inline T Injector<P...>::get() {
  using namespace fruit::impl;

  FruitDelegateCheck(TypeNotProvidedError<T, ApplyC<IsInList, Apply<GetClassForType, T>, typename Comp::Ps>::value>);
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
