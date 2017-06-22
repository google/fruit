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

#ifndef FRUIT_LAZY_COMPONENT_WITH_NO_ARGS_DEFN_H
#define FRUIT_LAZY_COMPONENT_WITH_NO_ARGS_DEFN_H

#include <fruit/impl/bindings/lazy_component_with_no_args.h>
#include <fruit/impl/util/hash_codes.h>
#include <fruit/impl/util/hash_codes.h>

namespace fruit {
namespace impl {

template <typename Component>
void LazyComponentWithNoArgs::addBindings(erased_fun_t erased_fun, ComponentStorage& component_storage) {
  Component component = reinterpret_cast<Component(*)()>(erased_fun)();
  component_storage.install(std::move(component.storage));
}

template <typename Component>
inline LazyComponentWithNoArgs LazyComponentWithNoArgs::create(Component(*fun)()) {
  FruitAssert(fun != nullptr);
  LazyComponentWithNoArgs result;
  result.erased_fun = reinterpret_cast<erased_fun_t>(fun);
  result.type_id = getTypeId<Component(*)()>();
  result.add_bindings_fun = LazyComponentWithNoArgs::addBindings<Component>;
  return result;
}

inline LazyComponentWithNoArgs LazyComponentWithNoArgs::createInvalid() {
  LazyComponentWithNoArgs result;
  result.erased_fun = nullptr;
  result.type_id = TypeId{nullptr};
  result.add_bindings_fun = nullptr;
  return result;
}

inline bool LazyComponentWithNoArgs::isValid() const {
  return erased_fun != nullptr;
}

inline bool LazyComponentWithNoArgs::operator==(const LazyComponentWithNoArgs& other) const {
  if (erased_fun == other.erased_fun) {
    // These must be equal in this case, no need to compare them.
    FruitAssert(type_id == other.type_id);
    FruitAssert(add_bindings_fun == other.add_bindings_fun);
    return true;
  } else {
    // type_id and add_bindings_fun may or may not be different from the ones in `other`.
    return false;
  }
}

inline void LazyComponentWithNoArgs::addBindings(ComponentStorage& storage) const {
  FruitAssert(isValid());
  add_bindings_fun(erased_fun, storage);
}

inline std::size_t LazyComponentWithNoArgs::hashCode() const {
  // We only need to hash this field (for the same reason that we only compare this field in operator==).
  return std::hash<erased_fun_t>()(erased_fun);
}

inline TypeId LazyComponentWithNoArgs::getFunTypeId() const {
  FruitAssert(isValid());
  return type_id;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_LAZY_COMPONENT_WITH_NO_ARGS_DEFN_H
