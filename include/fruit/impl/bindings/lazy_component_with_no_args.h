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

#ifndef FRUIT_LAZY_COMPONENT_WITH_NO_ARGS_H
#define FRUIT_LAZY_COMPONENT_WITH_NO_ARGS_H

#include <fruit/impl/storage/component_storage.h>

namespace fruit {
namespace impl {

/**
 * Similar to LazyComponent, but only supports component functions that have no args.
 * This is significantly more efficient though.
 */
class LazyComponentWithNoArgs {
private:
  // An arbitrary function type, used as type for the field `erased_fun`.
  // Note that we can't use void* here, since data pointers might not have the same size as function pointers.
  using erased_fun_t = void(*)();

  // The function that will be invoked to create the Component.
  // Here we don't know the type, it's only known at construction time.
  erased_fun_t erased_fun;

  // The type ID of the (non-erased) fun.
  TypeId type_id;

  // The function that allows to add this component's bindings to the given ComponentStorage.
  using add_bindings_fun_t = void(*)(erased_fun_t, ComponentStorage&);
  add_bindings_fun_t add_bindings_fun;

  template <typename Component>
  static void addBindings(erased_fun_t erased_fun, ComponentStorage& component_storage);

public:
  template <typename Component>
  static LazyComponentWithNoArgs create(Component(*fun)());

  static LazyComponentWithNoArgs createInvalid();

  bool operator==(const LazyComponentWithNoArgs& other) const;

  void addBindings(ComponentStorage& storage) const;

  std::size_t hashCode() const;

  bool isValid() const;

  /**
   * Returns the type ID of the original `fun` object (before erasure).
   * We use this instead of the `typeid` operator so that we don't require RTTI.
   */
  TypeId getFunTypeId() const;
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/bindings/lazy_component_with_no_args.defn.h>

#endif // FRUIT_LAZY_COMPONENT_WITH_NO_ARGS_H
