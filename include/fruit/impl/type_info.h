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

#ifndef FRUIT_TYPE_INFO
#define FRUIT_TYPE_INFO

#include <typeinfo>

namespace fruit {
namespace impl {

// Similar to std::type_index, but with a constexpr constructor.
struct TypeIndex
{
  constexpr TypeIndex(const std::type_info& impl) noexcept
  : impl(&impl) { }

  bool
  operator==(const TypeIndex& __rhs) const noexcept
  { return *impl == *__rhs.impl; }

  bool
  operator!=(const TypeIndex& __rhs) const noexcept
  { return *impl != *__rhs.impl; }

  std::size_t
  hash_code() const noexcept
  { return impl->hash_code(); }

  const char*
  name() const
  { return impl->name(); }

private:
  const std::type_info* impl;
};

template <typename T>
TypeIndex getTypeIndex() noexcept;

} // namespace impl
} // namespace fruit

namespace std {

template <typename _Tp>
struct hash;

/// std::hash specialization for TypeIndex.
template<>
struct hash<fruit::impl::TypeIndex>
{
  typedef std::size_t        result_type;
  typedef fruit::impl::TypeIndex  argument_type;

  std::size_t
  operator()(const fruit::impl::TypeIndex& typeIndex) const noexcept
  { return typeIndex.hash_code(); }
};

} // namespace std

#include "type_info.templates.h"

#endif // FRUIT_TYPE_INFO
