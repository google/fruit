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

#ifndef FRUIT_INJECTOR_TEMPLATES_H
#define FRUIT_INJECTOR_TEMPLATES_H

#include "../component.h"

// Redundant, but makes KDevelop happy.
#include "../injector.h"

namespace fruit {

template <typename... P>
inline Injector<P...>::Injector(Component<P...> component)
  : storage(std::move(component.storage)) {
};

template <typename... P>
template <typename... NormalizedComponentParams, typename... ComponentParams>
inline Injector<P...>::Injector(NormalizedComponent<NormalizedComponentParams...> normalizedComponent,
                                Component<ComponentParams...> component)
  : storage(fruit::impl::NormalizedComponentStorage::mergeComponentStorages(std::move(normalizedComponent.storage),
                                                                            std::move(component.storage))) {
    
  using Comp = fruit::impl::ConstructComponentImpl<ComponentParams...>;
  FruitDelegateCheck(fruit::impl::ComponentWithRequirementsInInjectorErrorHelper<typename Comp::Rs>);
  
  using NormalizedComp = fruit::impl::ConstructComponentImpl<NormalizedComponentParams...>;
  
  // The calculation of MergedComp will also do some checks, e.g. multiple bindings for the same type.
  using MergedComp = typename fruit::impl::InstallComponent<NormalizedComp, Comp>::Result;
  
  FruitDelegateCheck(fruit::impl::UnsatisfiedRequirementsInNormalizedComponentHelper<typename MergedComp::Rs>);
  FruitDelegateCheck(fruit::impl::TypesInInjectorNotProvidedHelper<fruit::impl::set_difference<fruit::impl::List<P...>,
                                                                   typename MergedComp::Ps>>);
}

template <typename... P>
template <typename T>
inline T Injector<P...>::get() {
  FruitDelegateCheck(fruit::impl::TypeNotProvidedError<T, fruit::impl::is_in_list<impl::GetClassForType<T>, typename Comp::Ps>::value>);
  return storage.get<T>();
}

template <typename... P>
template <typename T>
inline Injector<P...>::operator T() {
  return get<T>();
}

template <typename... P>
template <typename C>
inline const std::vector<C*>& Injector<P...>::getMultibindings() {
  return storage.getMultibindings<C>();
}

template <typename... P>
inline void Injector<P...>::eagerlyInjectAll() {
  // Eagerly inject normal bindings.
  void* unused[] = {reinterpret_cast<void*>(storage.get<P*>())...};
  (void)unused;
  
  storage.eagerlyInjectMultibindings();
}

} // namespace fruit


#endif // FRUIT_INJECTOR_TEMPLATES_H
