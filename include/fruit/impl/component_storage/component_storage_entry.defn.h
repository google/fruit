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

#ifndef FRUIT_COMPONENT_STORAGE_ENTRY_DEFN_H
#define FRUIT_COMPONENT_STORAGE_ENTRY_DEFN_H

#include <fruit/impl/component_storage/component_storage_entry.h>
#include <fruit/impl/util/call_with_tuple.h>
#include <fruit/impl/util/hash_codes.h>
#include <fruit/component_function.h>

namespace fruit {
namespace impl {

// We use a custom method instead of a real copy constructor so that all copies are explicit (since copying is a
// fairly expensive operation).
inline ComponentStorageEntry ComponentStorageEntry::copy() const {
  FruitAssert(kind != Kind::INVALID);
  ComponentStorageEntry result;
  switch (kind) {
  case Kind::LAZY_COMPONENT_WITH_ARGS:
  case Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS:
  case Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    result.kind = kind;
    result.type_id = type_id;
    result.lazy_component_with_args = lazy_component_with_args.copy();
    break;

  default:
    result = *this;
  }
  return result;
}

// We use a custom method instead of a real destructor, so that we can hold these in a std::vector but still destroy
// them when desired.
inline void ComponentStorageEntry::destroy() const {
  FruitAssert(kind != Kind::INVALID);
  switch (kind) {
  case Kind::LAZY_COMPONENT_WITH_ARGS:
  case Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS:
  case Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS:
    lazy_component_with_args.destroy();
#if FRUIT_EXTRA_DEBUG
    kind = Kind::INVALID;
#endif
    break;

  default:
    break;
  }
}

inline ComponentStorageEntry::LazyComponentWithArgs::ComponentInterface::ComponentInterface(erased_fun_t erased_fun)
    : erased_fun(erased_fun) {}

template <typename Component, typename... Args>
class ComponentInterfaceImpl : public ComponentStorageEntry::LazyComponentWithArgs::ComponentInterface {
private:
  using ComponentInterface = ComponentStorageEntry::LazyComponentWithArgs::ComponentInterface;

  using fun_t = Component (*)(Args...);
  std::tuple<Args...> args_tuple;

public:
  inline ComponentInterfaceImpl(fun_t fun, std::tuple<Args...> args_tuple)
      : ComponentInterface(reinterpret_cast<erased_fun_t>(fun)), args_tuple(std::move(args_tuple)) {}

  inline bool
  areParamsEqual(const ComponentStorageEntry::LazyComponentWithArgs::ComponentInterface& other) const final {
    if (getFunTypeId() != other.getFunTypeId()) {
      return false;
    }
    const auto& casted_other = static_cast<const ComponentInterfaceImpl<Component, Args...>&>(other);
    return args_tuple == casted_other.args_tuple;
  }

  inline void addBindings(entry_vector_t& entries) const final {
    Component component = callWithTuple<Component, Args...>(reinterpret_cast<fun_t>(erased_fun), args_tuple);
    FixedSizeVector<ComponentStorageEntry> component_entries = std::move(component.storage).release();
    entries.insert(entries.end(), component_entries.begin(), component_entries.end());
  }

  inline std::size_t hashCode() const final {
    std::size_t fun_hash = std::hash<fun_t>()(reinterpret_cast<fun_t>(erased_fun));
    std::size_t args_hash = hashTuple(args_tuple);
    return combineHashes(fun_hash, args_hash);
  }

  inline ComponentInterface* copy() const final {
    return new ComponentInterfaceImpl{reinterpret_cast<fun_t>(erased_fun), args_tuple};
  }

  inline TypeId getFunTypeId() const final {
    return fruit::impl::getTypeId<Component (*)(Args...)>();
  }
};

template <typename Component, typename... Args>
inline ComponentStorageEntry ComponentStorageEntry::LazyComponentWithArgs::create(Component (*fun)(Args...),
                                                                                  std::tuple<Args...> args_tuple) {
  ComponentStorageEntry result;
  result.type_id = getTypeId<Component (*)(Args...)>();
  result.kind = ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_ARGS;
  result.lazy_component_with_args.component =
      new ComponentInterfaceImpl<Component, Args...>(fun, std::move(args_tuple));
  return result;
}

template <typename Component, typename Arg, typename... Args>
inline ComponentStorageEntry ComponentStorageEntry::LazyComponentWithArgs::create(
    fruit::ComponentFunction<Component, Arg, Args...> component_function) {
  return LazyComponentWithArgs::create(component_function.getComponent, component_function.args_tuple);
}

template <typename Component, typename... Args>
inline ComponentStorageEntry
ComponentStorageEntry::LazyComponentWithArgs::createReplacedComponentEntry(Component (*fun)(Args...),
                                                                           std::tuple<Args...> args_tuple) {
  ComponentStorageEntry result;
  result.type_id = getTypeId<Component (*)(Args...)>();
  result.kind = ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_ARGS;
  result.lazy_component_with_args.component =
      new ComponentInterfaceImpl<Component, Args...>(fun, std::move(args_tuple));
  return result;
}

template <typename Component, typename... Args>
inline ComponentStorageEntry
ComponentStorageEntry::LazyComponentWithArgs::createReplacementComponentEntry(Component (*fun)(Args...),
                                                                              std::tuple<Args...> args_tuple) {
  ComponentStorageEntry result;
  result.type_id = getTypeId<Component (*)(Args...)>();
  result.kind = ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_ARGS;
  result.lazy_component_with_args.component =
      new ComponentInterfaceImpl<Component, Args...>(fun, std::move(args_tuple));
  return result;
}

inline ComponentStorageEntry::LazyComponentWithArgs ComponentStorageEntry::LazyComponentWithArgs::copy() const {
  LazyComponentWithArgs result;
  result.component = component->copy();
  return result;
}

inline void ComponentStorageEntry::LazyComponentWithArgs::destroy() const {
  delete component;
}

inline bool ComponentStorageEntry::LazyComponentWithArgs::ComponentInterface::
operator==(const ComponentInterface& other) const {
  return erased_fun == other.erased_fun && areParamsEqual(other);
}

template <typename Component>
void ComponentStorageEntry::LazyComponentWithNoArgs::addBindings(erased_fun_t erased_fun, entry_vector_t& entries) {
  Component component = reinterpret_cast<Component (*)()>(erased_fun)();
  FixedSizeVector<ComponentStorageEntry> component_entries = std::move(component.storage).release();
  entries.insert(entries.end(), component_entries.begin(), component_entries.end());
}

template <typename Component>
inline ComponentStorageEntry ComponentStorageEntry::LazyComponentWithNoArgs::create(Component (*fun)()) {
  FruitAssert(fun != nullptr);
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::LAZY_COMPONENT_WITH_NO_ARGS;
  result.type_id = getTypeId<Component (*)()>();
  result.lazy_component_with_no_args.erased_fun = reinterpret_cast<erased_fun_t>(fun);
  result.lazy_component_with_no_args.add_bindings_fun = LazyComponentWithNoArgs::addBindings<Component>;
  return result;
}

template <typename Component>
inline ComponentStorageEntry ComponentStorageEntry::LazyComponentWithNoArgs::create(
        fruit::ComponentFunction<Component> component_function) {
  return LazyComponentWithNoArgs::create(component_function.getComponent);
}

template <typename Component>
inline ComponentStorageEntry
ComponentStorageEntry::LazyComponentWithNoArgs::createReplacedComponentEntry(Component (*fun)()) {
  FruitAssert(fun != nullptr);
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::REPLACED_LAZY_COMPONENT_WITH_NO_ARGS;
  result.type_id = getTypeId<Component (*)()>();
  result.lazy_component_with_no_args.erased_fun = reinterpret_cast<erased_fun_t>(fun);
  result.lazy_component_with_no_args.add_bindings_fun = LazyComponentWithNoArgs::addBindings<Component>;
  return result;
}

template <typename Component>
inline ComponentStorageEntry
ComponentStorageEntry::LazyComponentWithNoArgs::createReplacementComponentEntry(Component (*fun)()) {
  FruitAssert(fun != nullptr);
  ComponentStorageEntry result;
  result.kind = ComponentStorageEntry::Kind::REPLACEMENT_LAZY_COMPONENT_WITH_NO_ARGS;
  result.type_id = getTypeId<Component (*)()>();
  result.lazy_component_with_no_args.erased_fun = reinterpret_cast<erased_fun_t>(fun);
  result.lazy_component_with_no_args.add_bindings_fun = LazyComponentWithNoArgs::addBindings<Component>;
  return result;
}

inline bool ComponentStorageEntry::LazyComponentWithNoArgs::isValid() const {
  return erased_fun != nullptr;
}

inline bool ComponentStorageEntry::LazyComponentWithNoArgs::
operator==(const ComponentStorageEntry::LazyComponentWithNoArgs& other) const {
  if (erased_fun == other.erased_fun) {
    // These must be equal in this case, no need to compare them.
    FruitAssert(add_bindings_fun == other.add_bindings_fun);
    return true;
  } else {
    // type_id and add_bindings_fun may or may not be different from the ones in `other`.
    return false;
  }
}

inline void ComponentStorageEntry::LazyComponentWithNoArgs::addBindings(entry_vector_t& entries) const {
  FruitAssert(isValid());
  add_bindings_fun(erased_fun, entries);
}

inline std::size_t ComponentStorageEntry::LazyComponentWithNoArgs::hashCode() const {
  // We only need to hash this field (for the same reason that we only compare this field in operator==).
  return std::hash<erased_fun_t>()(erased_fun);
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_COMPONENT_STORAGE_ENTRY_DEFN_H
