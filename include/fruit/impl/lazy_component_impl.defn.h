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

#ifndef FRUIT_LAZY_COMPONENT_IMPL_DEFN_H
#define FRUIT_LAZY_COMPONENT_IMPL_DEFN_H

#include <fruit/impl/lazy_component_impl.h>
#include <fruit/impl/util/hash_codes.h>

namespace fruit {
namespace impl {

template <typename Component, typename... Args>
inline LazyComponentImpl<Component, Args...>::LazyComponentImpl(
    fun_t fun,
    std::tuple<Args...> args_tuple)
  : LazyComponent(reinterpret_cast<erased_fun_t>(fun)),
    args_tuple(std::move(args_tuple)) {
}

template <typename Component, typename... Args>
inline bool LazyComponentImpl<Component, Args...>::areParamsEqual(const LazyComponent& other) const {
  if (getFunTypeId() != other.getFunTypeId()) {
    return false;
  }
  const auto& casted_other = static_cast<const LazyComponentImpl<Component, Args...>&>(other);
  return args_tuple == casted_other.args_tuple;
}

template <typename Component, typename... Args>
inline void LazyComponentImpl<Component, Args...>::addBindings(ComponentStorage& storage) const {
  Component component = callWithTuple<Component, Args...>(reinterpret_cast<fun_t>(erased_fun), args_tuple);
  storage.install(std::move(component.storage));
}

template <typename Component, typename... Args>
inline std::size_t LazyComponentImpl<Component, Args...>::hashCode() const {
  std::size_t fun_hash = std::hash<fun_t>()(reinterpret_cast<fun_t>(erased_fun));
  std::size_t args_hash = hashTuple(args_tuple);
  return combineHashes(fun_hash, args_hash);
}

template <typename Component, typename... Args>
inline std::unique_ptr<LazyComponent> LazyComponentImpl<Component, Args...>::copy() const {
  return std::unique_ptr<LazyComponent>{
      new LazyComponentImpl{reinterpret_cast<fun_t>(erased_fun), args_tuple}};
}

template <typename Component, typename... Args>
inline TypeId LazyComponentImpl<Component, Args...>::getFunTypeId() const {
  return fruit::impl::getTypeId<Component(*)(Args...)>();
}


} // namespace impl
} // namespace fruit

#endif // FRUIT_LAZY_COMPONENT_IMPL_DEFN_H
