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

#ifndef FRUIT_GENERIC_LAZY_COMPONENT_H
#define FRUIT_GENERIC_LAZY_COMPONENT_H

#include <fruit/impl/lazy_component.h>
#include <fruit/impl/lazy_component_with_no_args.h>

namespace fruit {
namespace impl {

enum class GenericLazyComponentKind {
  COMPONENT_WITH_ARGS,
  COMPONENT_WITHOUT_ARGS,
  // These markers are used in expandLazyComponents(), see the comments there for details.
  COMPONENT_WITH_ARGS_END_MARKER,
  COMPONENT_WITHOUT_ARGS_END_MARKER,
};

/**
 * This is an abstraction over either a LazyComponent or a LazyComponentWithNoArgs.
 * If owns_pointer is true, the lazy_component object is owned by this class, otherwise it isn't.
 * owns_pointer is irrelevant when this holds a LazyComponentWithNoArgs.
 */
template <bool owns_pointer>
struct GenericLazyComponent {
  union {
    // This field is valid iff Kind is COMPONENT_WITH_ARGS or COMPONENT_WITH_ARGS_END_MARKER.
    const LazyComponent* lazy_component;

    // This field is valid iff Kind is COMPONENT_WITHOUT_ARGS or COMPONENT_WITHOUT_ARGS_END_MARKER.
    LazyComponentWithNoArgs lazy_component_with_no_args;
  };

  GenericLazyComponentKind kind;

  GenericLazyComponent() = default;

  GenericLazyComponent(GenericLazyComponent&&);
  GenericLazyComponent(const GenericLazyComponent&) = delete;

  ~GenericLazyComponent();

  GenericLazyComponent& operator=(GenericLazyComponent&&);
  GenericLazyComponent& operator=(const GenericLazyComponent&) = delete;

  // Checks the equality of the two objects.
  // WARNING: this ignores the difference between a kind and the corresponding _END_MARKER kind.
  bool operator==(const GenericLazyComponent& other) const;

  // Creates a GenericLazyComponent that will take ownership of the specified LazyComponent object.
  static GenericLazyComponent create(const LazyComponent* lazy_component);

  static GenericLazyComponent create(LazyComponentWithNoArgs lazy_component_with_no_args);

  // We could implement the copy-constructor and the assignment operator instead, but here we choose to have an explicit
  // copy() operation because copying a GenericLazyComponent is very expensive; so we want to make sure we
  // have no unnecessary implicit copies.
  template <bool other_owns_pointer>
  GenericLazyComponent<other_owns_pointer> copy() const;
};

} // namespace impl
} // namespace fruit

namespace std {

template <>
struct hash<fruit::impl::GenericLazyComponent<true>> {
  std::size_t operator()(const fruit::impl::GenericLazyComponent<true>&) const;
};

template <>
struct hash<fruit::impl::GenericLazyComponent<false>> {
  std::size_t operator()(const fruit::impl::GenericLazyComponent<false>&) const;
};

} // namespace std

#include <fruit/impl/generic_lazy_component.defn.h>

#endif // FRUIT_GENERIC_LAZY_COMPONENT_H
