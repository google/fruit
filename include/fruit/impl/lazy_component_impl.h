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

#ifndef FRUIT_LAZY_COMPONENT_IMPL_H
#define FRUIT_LAZY_COMPONENT_IMPL_H

#include <fruit/impl/lazy_component.h>

namespace fruit {
namespace impl {

template <typename Component, typename... Args>
class LazyComponentImpl : public LazyComponent {
private:
  using fun_t = Component(*)(Args...);
  std::tuple<Args...> args_tuple;

public:
  LazyComponentImpl(fun_t fun, std::tuple<Args...> args_tuple);

  bool areParamsEqual(const LazyComponent& other) const final;
  void addBindings(ComponentStorage& component_storage) const final;
  std::size_t hashCode() const final;
  LazyComponent* copy() const final;
  TypeId getFunTypeId() const final;
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/lazy_component_impl.defn.h>

#endif // FRUIT_LAZY_COMPONENT_IMPL_H
