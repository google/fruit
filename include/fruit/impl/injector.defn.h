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
    
  // TODO: Avoid using declarations in headers.
  using namespace fruit::impl;
  using namespace fruit::impl::meta;
    
  using Comp = fruit::impl::meta::Apply<meta::ConstructComponentImpl, ComponentParams...>;
  // We don't check whether Comp is an error here; if it was, the instantiation of Component<Comp>
  // would have resulted in an error already.
  using E1 = Eval<std::conditional<!meta::Apply<meta::IsEmptyVector, typename Comp::Rs>::value,
      Apply<ConstructErrorWithArgVector, ComponentWithRequirementsInInjectorErrorTag, typename Comp::Rs>,
      int
  >>;
  (void)typename CheckIfError<E1>::type();
  
  using NormalizedComp = meta::Apply<meta::ConstructComponentImpl, NormalizedComponentParams...>;
  // We don't check whether NormalizedComp is an error here; if it was, the instantiation of
  // NormalizedComponent<NormalizedComp> would have resulted in an error already.
  
  using Op = meta::Apply<InstallComponent<NormalizedComp>, Comp>;
  (void)typename CheckIfError<typename Op::Result>::type();
  
  // The calculation of MergedComp will also do some checks, e.g. multiple bindings for the same type.
  using MergedComp = typename Op::Result;
  
  using TypesNotProvided = meta::Apply<meta::SetDifference,
                                       meta::Vector<P...>,
                                       typename MergedComp::Ps>;
  using E2 = Eval<std::conditional<!Apply<IsEmptyVector, typename MergedComp::Rs>::value,
      Apply<ConstructErrorWithArgVector, UnsatisfiedRequirementsInNormalizedComponentErrorTag, typename MergedComp::Rs>,
      Eval<std::conditional<!Apply<IsEmptyVector, TypesNotProvided>::value,
        Apply<ConstructErrorWithArgVector, TypesInInjectorNotProvidedErrorTag, TypesNotProvided>,
        int
      >>
  >>;
  (void)typename CheckIfError<E2>::type();
}

template <typename... P>
template <typename T>
inline T Injector<P...>::get() {
  using namespace fruit::impl;
  using namespace fruit::impl::meta;

  using E = Eval<std::conditional<!meta::Apply<meta::IsInVector, meta::Apply<meta::GetClassForType, T>, typename Comp::Ps>::value,
      Error<TypeNotProvidedErrorTag, T>,
      int
  >>;
  (void)typename CheckIfError<E>::type();
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
