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

#ifndef FRUIT_PROVIDER_DEFN_H
#define FRUIT_PROVIDER_DEFN_H

#include <fruit/impl/injector/injector_storage.h>

// Redundant, but makes KDevelop happy.
#include <fruit/provider.h>

namespace fruit {

template <typename C>
inline Provider<C>::Provider(fruit::impl::InjectorStorage* storage,
                             fruit::impl::InjectorStorage::Graph::node_iterator itr)
    : storage(storage), itr(itr) {}

template <typename C>
inline C* Provider<C>::get() {
  return get<C*>();
}

namespace impl {
namespace meta {

template <typename C>
struct ProviderImplHelper {

  template <typename T>
  using CheckGet = Eval<PropagateError(
      CheckInjectableType(RemoveAnnotations(Type<T>)),
      If(Not(IsSame(GetClassForType(Type<T>), RemoveConstFromType(Type<C>))),
         ConstructError(Id<TypeNotProvidedErrorTag>, Type<T>),
         If(And(TypeInjectionRequiresNonConstBinding(Type<T>), Not(IsSame(Id<GetClassForType(Type<T>)>, Type<C>))),
            ConstructError(TypeProvidedAsConstOnlyErrorTag, Type<T>), None)))>;
};

} // namespace meta
} // namespace impl

template <typename C>
template <typename T>
inline T Provider<C>::get() {
  using E = typename fruit::impl::meta::ProviderImplHelper<C>::template CheckGet<T>;
  (void)typename fruit::impl::meta::CheckIfError<E>::type();
  return storage->template get<T>(itr);
}

template <typename C>
template <typename T>
inline Provider<C>::operator T() {
  return get<T>();
}

} // namespace fruit

#endif // FRUIT_PROVIDER_DEFN_H
