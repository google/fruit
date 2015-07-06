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
                                             fruit::impl::getTypeIdsForList<typename fruit::impl::meta::Eval<
                                                 fruit::impl::meta::ConcatVectors(
                                                 typename fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<ComponentParams>...)>::type::Ps,
                                                 typename fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<NormalizedComponentParams>...)>::type::Ps)
                                             >::type>())) {
    
  using namespace fruit::impl;
  using namespace fruit::impl::meta;
    
  using Comp = typename Eval<ConstructComponentImpl(Type<ComponentParams>...)>::type;
  // We don't check whether Comp is an error here; if it was, the instantiation of Component<Comp>
  // would have resulted in an error already.
  using E1 = typename Eval<If(Not(IsEmptyVector(typename Comp::Rs)),
                              ConstructErrorWithArgVector(ComponentWithRequirementsInInjectorErrorTag, typename Comp::Rs),
                              Type<int>)
                           >::type;
  (void)typename CheckIfError<E1>::type();
  
  using NormalizedComp = typename Eval<ConstructComponentImpl(Type<NormalizedComponentParams>...)>::type;
  // We don't check whether NormalizedComp is an error here; if it was, the instantiation of
  // NormalizedComponent<NormalizedComp> would have resulted in an error already.
  
  using Op = typename Eval<InstallComponent(Comp, NormalizedComp)>::type;
  (void)typename CheckIfError<Op>::type();
  
  // The calculation of MergedComp will also do some checks, e.g. multiple bindings for the same type.
  using MergedComp = typename Op::Result;
  
  using TypesNotProvided = typename Eval<SetDifference(
                                         Vector<Type<P>...>,
                                         typename MergedComp::Ps)
                                         >::type;
  using E2 = typename Eval<If(Not(IsEmptyVector(typename MergedComp::Rs)),
                              ConstructErrorWithArgVector(UnsatisfiedRequirementsInNormalizedComponentErrorTag, typename MergedComp::Rs),
                              If(Not(IsEmptyVector(TypesNotProvided)),
                                 ConstructErrorWithArgVector(TypesInInjectorNotProvidedErrorTag, TypesNotProvided),
                                 Type<int>))
                           >::type;
  (void)typename CheckIfError<E2>::type();
}

template <typename... P>
template <typename T>
inline fruit::impl::meta::EvalType<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<T>)> Injector<P...>::get() {
  using namespace fruit::impl;
  using namespace fruit::impl::meta;

  using E = typename Eval<If(Not(IsInVector(NormalizeType(Type<T>), typename Comp::Ps)),
                             ConstructError(TypeNotProvidedErrorTag, Type<T>),
                             Type<int>)
                             >::type;
  (void)typename CheckIfError<E>::type();
  return storage->template get<T>();
}

template <typename... P>
template <typename C>
inline fruit::impl::meta::EvalType<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<C>)>* Injector<P...>::unsafeGet() {
  return storage->template unsafeGet<C>();
}

template <typename... P>
template <typename T>
inline Injector<P...>::operator T() {
  return get<T>();
}

template <typename... P>
template <typename AnnotatedC>
inline const std::vector<fruit::impl::meta::EvalType<fruit::impl::meta::RemoveAnnotations(fruit::impl::meta::Type<AnnotatedC>)>*>& Injector<P...>::getMultibindings() {
  return storage->template getMultibindings<AnnotatedC>();
}

template <typename... P>
inline void Injector<P...>::eagerlyInjectAll() {
  using namespace fruit::impl::meta;
  // Eagerly inject normal bindings.
  void* unused[] = {reinterpret_cast<void*>(storage->template get<EvalType<AddPointerInAnnotatedType(Type<P>)>>())...};
  (void)unused;
  
  storage->eagerlyInjectMultibindings();
}

} // namespace fruit


#endif // FRUIT_INJECTOR_DEFN_H
