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

#ifndef FRUIT_LAZY_COMPONENT_DEFN_H
#define FRUIT_LAZY_COMPONENT_DEFN_H

#include <fruit/impl/lazy_component.h>

namespace fruit {
namespace impl {

inline LazyComponent::LazyComponent(erased_fun_t erased_fun)
  : erased_fun(erased_fun) {
}

inline bool LazyComponent::operator==(const LazyComponent& other) const {
  return erased_fun == other.erased_fun && areParamsEqual(other);
}

} // namespace impl
} // namespace fruit

namespace std {

inline std::size_t hash<fruit::impl::LazyComponent>::operator()(const fruit::impl::LazyComponent& component) const {
  return component.hashCode();
}

} // namespace std

#endif // FRUIT_LAZY_COMPONENT_DEFN_H
