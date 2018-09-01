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

#include <fruit/component.h>

// Redundant, but makes KDevelop happy.
#include <fruit/injector.h>

namespace fruit {

template <typename... P>
template <typename... FormalArgs, typename... Args>
inline Injector<P...>::Injector(Component<P...> (*getComponent)(FormalArgs...), Args&&... args) {
  Component<P...> component = fruit::createComponent().install(getComponent, std::forward<Args>(args)...);

  fruit::impl::MemoryPool memory_pool;
  using exposed_types_t = std::vector<fruit::impl::TypeId, fruit::impl::ArenaAllocator<fruit::impl::TypeId>>;
  exposed_types_t exposed_types =
      exposed_types_t(std::initializer_list<fruit::impl::TypeId>{fruit::impl::getTypeId<P>()...},
                      fruit::impl::ArenaAllocator<fruit::impl::TypeId>(memory_pool));
  storage = std::unique_ptr<fruit::impl::InjectorStorage>(
      new fruit::impl::InjectorStorage(std::move(component.storage), exposed_types, memory_pool));
}

namespace impl {
namespace meta {

template <typename... P>
struct InjectorImplHelper {

  // This performs all checks needed in the constructor of Injector that takes NormalizedComponent.
  template <typename NormalizedComp, typename Comp>
  struct CheckConstructionFromNormalizedComponent {
    using Op = InstallComponent(Comp, NormalizedComp);

    // The calculation of MergedComp will also do some checks, e.g. multiple bindings for the same type.
    using MergedComp = GetResult(Op);

    using TypesNotProvided = SetDifference(RemoveConstFromTypes(Vector<Type<P>...>), GetComponentPs(MergedComp));
    using MergedCompRs = SetDifference(GetComponentRsSuperset(MergedComp), GetComponentPs(MergedComp));

    using type = Eval<If(
        Not(IsEmptySet(GetComponentRsSuperset(Comp))),
        ConstructErrorWithArgVector(ComponentWithRequirementsInInjectorErrorTag,
                                    SetToVector(GetComponentRsSuperset(Comp))),
        If(Not(IsEmptySet(MergedCompRs)),
           ConstructErrorWithArgVector(UnsatisfiedRequirementsInNormalizedComponentErrorTag, SetToVector(MergedCompRs)),
           If(Not(IsContained(VectorToSetUnchecked(RemoveConstFromTypes(Vector<Type<P>...>)),
                              GetComponentPs(MergedComp))),
              ConstructErrorWithArgVector(TypesInInjectorNotProvidedErrorTag, SetToVector(TypesNotProvided)),
              If(Not(IsContained(VectorToSetUnchecked(RemoveConstTypes(Vector<Type<P>...>)),
                                 GetComponentNonConstRsPs(MergedComp))),
                 ConstructErrorWithArgVector(
                     TypesInInjectorProvidedAsConstOnlyErrorTag,
                     SetToVector(SetDifference(VectorToSetUnchecked(RemoveConstTypes(Vector<Type<P>...>)),
                                               GetComponentNonConstRsPs(MergedComp)))),
                 None))))>;
  };

  template <typename T>
  struct CheckGet {
    using Comp = ConstructComponentImpl(Type<P>...);

    using type = Eval<PropagateError(CheckInjectableType(RemoveAnnotations(Type<T>)),
                                     If(Not(IsInSet(NormalizeType(Type<T>), GetComponentPs(Comp))),
                                        ConstructError(TypeNotProvidedErrorTag, Type<T>),
                                        If(And(TypeInjectionRequiresNonConstBinding(Type<T>),
                                               Not(IsInSet(NormalizeType(Type<T>), GetComponentNonConstRsPs(Comp)))),
                                           ConstructError(TypeProvidedAsConstOnlyErrorTag, Type<T>), None)))>;
  };
};

} // namespace meta
} // namespace impl

template <typename... P>
template <typename... NormalizedComponentParams, typename... ComponentParams, typename... FormalArgs, typename... Args>
inline Injector<P...>::Injector(const NormalizedComponent<NormalizedComponentParams...>& normalized_component,
                                Component<ComponentParams...> (*getComponent)(FormalArgs...), Args&&... args) {
  Component<ComponentParams...> component = fruit::createComponent().install(getComponent, std::forward<Args>(args)...);

  fruit::impl::MemoryPool memory_pool;
  storage = std::unique_ptr<fruit::impl::InjectorStorage>(new fruit::impl::InjectorStorage(
      *(normalized_component.storage.storage), std::move(component.storage), memory_pool));

  using NormalizedComp =
      fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<NormalizedComponentParams>...);
  using Comp1 = fruit::impl::meta::ConstructComponentImpl(fruit::impl::meta::Type<ComponentParams>...);
  // We don't check whether the construction of NormalizedComp or Comp resulted in errors here; if they did, the
  // instantiation
  // of NormalizedComponent<NormalizedComponentParams...> or Component<ComponentParams...> would have resulted in an
  // error already.

  using E = typename fruit::impl::meta::InjectorImplHelper<P...>::template CheckConstructionFromNormalizedComponent<
      NormalizedComp, Comp1>::type;
  (void)typename fruit::impl::meta::CheckIfError<E>::type();
}

template <typename... P>
template <typename T>
inline fruit::impl::RemoveAnnotations<T> Injector<P...>::get() {
  using E = typename fruit::impl::meta::InjectorImplHelper<P...>::template CheckGet<T>::type;
  (void)typename fruit::impl::meta::CheckIfError<E>::type();
  return storage->template get<T>();
}

template <typename... P>
template <typename T>
inline Injector<P...>::operator T() {
  return get<T>();
}

template <typename... P>
template <typename AnnotatedC>
inline const std::vector<fruit::impl::RemoveAnnotations<AnnotatedC>*>& Injector<P...>::getMultibindings() {

  using Op = fruit::impl::meta::Eval<fruit::impl::meta::CheckNormalizedTypes(
      fruit::impl::meta::Vector<fruit::impl::meta::Type<AnnotatedC>>)>;
  (void)typename fruit::impl::meta::CheckIfError<Op>::type();

  return storage->template getMultibindings<AnnotatedC>();
}

template <typename... P>
FRUIT_DEPRECATED_DEFINITION(inline void Injector<P...>::eagerlyInjectAll()) {
  // Eagerly inject normal bindings.
  void* unused[] = {reinterpret_cast<void*>(
      storage->template get<fruit::impl::meta::UnwrapType<
          fruit::impl::meta::Eval<fruit::impl::meta::AddPointerInAnnotatedType(fruit::impl::meta::Type<P>)>>>())...};
  (void)unused;

  storage->eagerlyInjectMultibindings();
}

} // namespace fruit

#endif // FRUIT_INJECTOR_DEFN_H
