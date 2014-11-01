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

#include "injector_storage.h"

// Redundant, but makes KDevelop happy.
#include "../provider.h"

namespace fruit {

template <typename... P>
inline Provider<P...>::Provider(fruit::impl::InjectorStorage* storage)
  : storage(storage) {
}

template <typename... P>
template <typename T>
inline T Provider<P...>::get() {
  using namespace fruit::impl;
  FruitDelegateCheck(TypeNotProvidedError<T, ApplyC<IsInList, Apply<GetClassForType, T>, typename Comp::Ps>::value>);
  return storage->template get<T>();
}

template <typename... P>
template <typename T>
inline Provider<P...>::operator T() {
  return get<T>();
}


} // namespace fruit


#endif // FRUIT_PROVIDER_DEFN_H
