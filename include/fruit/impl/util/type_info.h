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

#ifndef FRUIT_TYPE_INFO_H
#define FRUIT_TYPE_INFO_H

#include <fruit/impl/meta/vector.h>
#include <fruit/impl/util/demangle_type_name.h>
#include <typeinfo>

#include <vector>

namespace fruit {
namespace impl {

// Similar to std::type_index, but with a constexpr constructor and also storing the type size and alignment.
// Also guaranteed to be aligned, to allow storing a TypeInfo and 1 bit together in the size of a void*.
struct alignas(1) alignas(void*) TypeInfo {

  struct ConcreteTypeInfo {
    // These fields are allowed to have dummy values for abstract types.
    std::size_t type_size;
    std::size_t type_alignment;
    bool is_trivially_destructible;

#if FRUIT_EXTRA_DEBUG
    bool is_abstract;
#endif
  };

  // This should only be used if RTTI is disabled. Use the other constructor if possible.
  constexpr TypeInfo(ConcreteTypeInfo concrete_type_info);

  constexpr TypeInfo(const std::type_info& info, ConcreteTypeInfo concrete_type_info);

  std::string name() const;

  size_t size() const;

  size_t alignment() const;

  bool isTriviallyDestructible() const;

private:
  // The std::type_info struct associated with the type, or nullptr if RTTI is disabled.
  // This is only used for the type name.
  const std::type_info* info;
  ConcreteTypeInfo concrete_type_info;
};

struct TypeId {
  const TypeInfo* type_info;

  operator std::string() const;

  bool operator==(TypeId x) const;
  bool operator!=(TypeId x) const;
  bool operator<(TypeId x) const;
};

// Returns the TypeId for the type T.
// Multiple invocations for the same type return the same value.
// This has special support for types of the form Annotated<SomeAnnotation, SomeType>, it reports
// data for SomeType (except the name, that is "Annotated<SomeAnnotation, SomeType>").
template <typename T>
TypeId getTypeId();

// A convenience function that returns an std::vector of TypeId values for the given meta-vector of types.
template <typename V>
std::vector<TypeId> getTypeIdsForList();

} // namespace impl
} // namespace fruit

#if FRUIT_EXTRA_DEBUG

#include <ostream>

namespace fruit {
namespace impl {

inline std::ostream& operator<<(std::ostream& os, TypeId type);

} // namespace impl
} // namespace fruit

#endif // FRUIT_EXTRA_DEBUG

namespace std {

template <>
struct hash<fruit::impl::TypeId> {
  std::size_t operator()(fruit::impl::TypeId type) const;
};

} // namespace std

#include <fruit/impl/util/type_info.defn.h>

#endif // FRUIT_TYPE_INFO_H
