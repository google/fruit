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
                                             fruit::impl::getTypeIdsForList<fruit::impl::meta::Eval<
                                                 fruit::impl::meta::ConcatVectors(
                                                    fruit::impl::meta::SetToVector(typename fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<ComponentParams>...)>::Ps),
                                                    fruit::impl::meta::SetToVector(typename fruit::impl::meta::Eval<fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<NormalizedComponentParams>...)>::Ps))
                                             >>())) {
    
  using namespace fruit::impl;
  using namespace fruit::impl::meta;
    
  using Comp = Eval<ConstructComponentImpl(Type<ComponentParams>...)>;
  // We don't check whether Comp is an error here; if it was, the instantiation of Component<Comp>
  // would have resulted in an error already.
  using E1 = Eval<If(Not(IsEmptySet(typename Comp::RsSuperset)),
                     ConstructErrorWithArgVector(ComponentWithRequirementsInInjectorErrorTag, SetToVector(typename Comp::RsSuperset)),
                  Type<int>)>;
  (void)typename CheckIfError<E1>::type();
  
  using NormalizedComp = ConstructComponentImpl(Type<NormalizedComponentParams>...);
  // We don't check whether NormalizedComp is an error here; if it was, the instantiation of
  // NormalizedComponent<NormalizedComp> would have resulted in an error already.
  
  using Op = Eval<InstallComponent(Comp, NormalizedComp)>;
  (void)typename CheckIfError<Op>::type();
  
  // The calculation of MergedComp will also do some checks, e.g. multiple bindings for the same type.
  using MergedComp = typename Op::Result;
  
  using TypesNotProvided = SetDifference(Vector<Type<P>...>,
                                         typename MergedComp::Ps);
  using MergedCompRs = SetDifference(typename MergedComp::RsSuperset,
                                     typename MergedComp::Ps);
  using E2 = Eval<If(Not(IsEmptySet(MergedCompRs)),
                     ConstructErrorWithArgVector(UnsatisfiedRequirementsInNormalizedComponentErrorTag, SetToVector(MergedCompRs)),
                  If(Not(IsContained(Vector<Type<P>...>, typename MergedComp::Ps)),
                     ConstructErrorWithArgVector(TypesInInjectorNotProvidedErrorTag, SetToVector(TypesNotProvided)),
                  Type<int>))>;
  (void)typename CheckIfError<E2>::type();
}

template <typename... P>
template <typename T>
inline Injector<P...>::RemoveAnnotations<T> Injector<P...>::get() {
  using namespace fruit::impl;
  using namespace fruit::impl::meta;

  using E = Eval<If(Not(IsInSet(NormalizeType(Type<T>), typename Comp::Ps)),
                    ConstructError(TypeNotProvidedErrorTag, Type<T>),
                    Type<int>)>;
  (void)typename CheckIfError<E>::type();
  return storage->template get<T>();
}

template <typename... P>
template <typename C>
inline Injector<P...>::RemoveAnnotations<C>* Injector<P...>::unsafeGet() {
  return storage->template unsafeGet<C>();
}

template <typename... P>
template <typename T>
inline Injector<P...>::operator T() {
  return get<T>();
}

template <typename... P>
template <typename AnnotatedC>
inline const std::vector<Injector<P...>::RemoveAnnotations<AnnotatedC>*>& Injector<P...>::getMultibindings() {
  return storage->template getMultibindings<AnnotatedC>();
}

template <typename... P>
inline void Injector<P...>::eagerlyInjectAll() {
  using namespace fruit::impl::meta;
  // Eagerly inject normal bindings.
  void* unused[] = {reinterpret_cast<void*>(storage->template get<UnwrapType<Eval<AddPointerInAnnotatedType(Type<P>)>>>())...};
  (void)unused;
  
  storage->eagerlyInjectMultibindings();
}

} // namespace fruit


#endif // FRUIT_INJECTOR_DEFN_H
