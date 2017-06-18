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

#ifndef FRUIT_GENERIC_LAZY_COMPONENT_DEFN_H
#define FRUIT_GENERIC_LAZY_COMPONENT_DEFN_H

#include <fruit/impl/generic_lazy_component.h>

namespace fruit {
namespace impl {

template <>
inline GenericLazyComponent<true>::~GenericLazyComponent() {
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    delete lazy_component;
  }
}

template <>
inline GenericLazyComponent<false>::~GenericLazyComponent() {
}

template <>
inline GenericLazyComponent<true>::GenericLazyComponent(GenericLazyComponent&& other) {
  kind = other.kind;
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    lazy_component = other.lazy_component;
    // This prevents other's destructor from deleting the pointer; this object has ownership now.
    other.kind = GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS;
  } else {
    lazy_component_with_no_args = other.lazy_component_with_no_args;
  }
}

template <>
inline GenericLazyComponent<false>::GenericLazyComponent(GenericLazyComponent&& other) {
  kind = other.kind;
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    lazy_component = other.lazy_component;
  } else {
    lazy_component_with_no_args = other.lazy_component_with_no_args;
  }
}

template <>
inline GenericLazyComponent<true>& GenericLazyComponent<true>::operator=(GenericLazyComponent&& other) {
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    delete lazy_component;
  }

  kind = other.kind;
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    lazy_component = other.lazy_component;
    // This prevents other's destructor from deleting the pointer; this object has ownership now.
    other.kind = GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS;
  } else {
    lazy_component_with_no_args = other.lazy_component_with_no_args;
  }

  return *this;
}

template <>
inline GenericLazyComponent<false>& GenericLazyComponent<false>::operator=(GenericLazyComponent&& other) {
  kind = other.kind;
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    lazy_component = other.lazy_component;
  } else {
    lazy_component_with_no_args = other.lazy_component_with_no_args;
  }

  return *this;
}



// Creates a GenericLazyComponent that will take ownership of the specified LazyComponent object.
template <bool owns_pointer>
inline GenericLazyComponent<owns_pointer> GenericLazyComponent<owns_pointer>::create(
    const LazyComponent* lazy_component) {
  GenericLazyComponent result;
  result.lazy_component = lazy_component;
  result.kind = GenericLazyComponentKind::COMPONENT_WITH_ARGS;
  return std::move(result);
}

template <bool owns_pointer>
inline GenericLazyComponent<owns_pointer> GenericLazyComponent<owns_pointer>::create(
    LazyComponentWithNoArgs lazy_component_with_no_args) {
  GenericLazyComponent result;
  result.lazy_component_with_no_args = lazy_component_with_no_args;
  result.kind = GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS;
  return std::move(result);
}

template <bool owns_pointer>
inline bool GenericLazyComponent<owns_pointer>::operator==(const GenericLazyComponent& other) const {
  switch (kind) {
  case GenericLazyComponentKind::COMPONENT_WITH_ARGS:
  case GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER:
    switch (other.kind) {
    case GenericLazyComponentKind::COMPONENT_WITH_ARGS:
    case GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER:
      return *lazy_component == *other.lazy_component;

    case GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS:
    case GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS_END_MARKER:
      return false;
    }

  case GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS:
  case GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS_END_MARKER:
    switch (other.kind) {
    case GenericLazyComponentKind::COMPONENT_WITH_ARGS:
    case GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER:
      return false;

    case GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS:
    case GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS_END_MARKER:
      return lazy_component_with_no_args == other.lazy_component_with_no_args;
    }
  }

  FruitAssert(false);
  return false;
}

template <>
template <>
inline GenericLazyComponent<false> GenericLazyComponent<true>::copy<false>() const {
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    return GenericLazyComponent<false>::create(lazy_component);
  } else {
    return GenericLazyComponent<false>::create(lazy_component_with_no_args);
  }
}

template <>
template <>
inline GenericLazyComponent<false> GenericLazyComponent<false>::copy<false>() const {
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    return GenericLazyComponent<false>::create(lazy_component);
  } else {
    return GenericLazyComponent<false>::create(lazy_component_with_no_args);
  }
}

template <>
template <>
inline GenericLazyComponent<true> GenericLazyComponent<true>::copy<true>() const {
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    return GenericLazyComponent<true>::create(lazy_component->copy());
  } else {
    return GenericLazyComponent<true>::create(lazy_component_with_no_args);
  }
}

template <>
template <>
inline GenericLazyComponent<true> GenericLazyComponent<false>::copy<true>() const {
  if (kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS
      || kind == GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER) {
    return GenericLazyComponent<true>::create(lazy_component->copy());
  } else {
    return GenericLazyComponent<true>::create(lazy_component_with_no_args);
  }
}

} // namespace impl
} // namespace fruit

namespace std {

inline std::size_t hash<fruit::impl::GenericLazyComponent<true>>::operator() (
    const fruit::impl::GenericLazyComponent<true>& lazy_component) const {
  switch (lazy_component.kind) {
  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITH_ARGS:
  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER:
    return lazy_component.lazy_component->hashCode();

  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS:
  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS_END_MARKER:
    return lazy_component.lazy_component_with_no_args.hashCode();
  }

  FruitAssert(false);
  return 0;
}

inline std::size_t hash<fruit::impl::GenericLazyComponent<false>>::operator() (
    const fruit::impl::GenericLazyComponent<false>& lazy_component) const {
  switch (lazy_component.kind) {
  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITH_ARGS:
  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITH_ARGS_END_MARKER:
    return lazy_component.lazy_component->hashCode();

  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS:
  case fruit::impl::GenericLazyComponentKind::COMPONENT_WITHOUT_ARGS_END_MARKER:
    return lazy_component.lazy_component_with_no_args.hashCode();
  }

  FruitAssert(false);
  return 0;
}

} // namespace std

#endif // FRUIT_GENERIC_LAZY_COMPONENT_DEFN_H
