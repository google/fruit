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

#ifndef FRUIT_BINDING_DEPS_DEFN_H
#define FRUIT_BINDING_DEPS_DEFN_H

#include <fruit/impl/component_storage/binding_deps.h>

namespace fruit {
namespace impl {

template <typename L>
struct GetBindingDepsHelper;

template <typename... Ts>
struct GetBindingDepsHelper<fruit::impl::meta::Vector<fruit::impl::meta::Type<Ts>...>> {
  inline const BindingDeps* operator()() {
    static const TypeId types[] = {getTypeId<Ts>()..., TypeId{nullptr}}; // LCOV_EXCL_BR_LINE
    static const BindingDeps deps = {types, sizeof...(Ts)};
    return &deps;
  }
};

// We specialize the "no Ts" case to avoid declaring types[] as an array of length 0.
template <>
struct GetBindingDepsHelper<fruit::impl::meta::Vector<>> {
  inline const BindingDeps* operator()() {
    static const TypeId types[] = {TypeId{nullptr}};
    static const BindingDeps deps = {types, 0};
    return &deps;
  }
};

template <typename Deps>
inline const BindingDeps* getBindingDeps() {
  return GetBindingDepsHelper<Deps>()();
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDING_DEPS_DEFN_H
