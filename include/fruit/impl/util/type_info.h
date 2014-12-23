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

#include <typeinfo>
#include "demangle_type_name.h"
#include "../meta/list.h"

namespace fruit {
namespace impl {

// Similar to std::type_index, but with a constexpr constructor and also storing the type size and alignment.
// Also guaranteed to be aligned, to allow storing a TypeInfo and 1 bit together in the size of a void*.
struct alignas(1) alignas(void*) TypeInfo {
  // This should only be used if RTTI is disabled. Use the other constructor if possible.
  constexpr TypeInfo(std::size_t type_size, std::size_t type_alignment)
  : info(nullptr), type_size(type_size), type_alignment(type_alignment) {
  }

  constexpr TypeInfo(const std::type_info& info, std::size_t type_size, std::size_t type_alignment)
  : info(&info), type_size(type_size), type_alignment(type_alignment) {
  }

  std::string name() const {
    if (info != nullptr)
      return demangleTypeName(info->name());
    else
      return "<unknown> (type name not accessible due to -fno-rtti)";
  }

  size_t size() const {
    return type_size;
  }  

  size_t alignment() const {
    return type_alignment;
  }  

private:
  // The std::type_info struct associated with the type, or nullptr if RTTI is disabled.
  const std::type_info* info;
  std::size_t type_size;
  std::size_t type_alignment;
};

struct TypeId {
  const TypeInfo* type_info;
  
  operator std::string() const {
    return type_info->name();
  }
  
  bool operator==(TypeId x) const {
    return type_info == x.type_info;
  }
  
  bool operator!=(TypeId x) const {
    return type_info != x.type_info;
  }
  
  bool operator<(TypeId x) const {
    return type_info < x.type_info;
  }
};

// Returns the TypeId for the type T.
// Multiple invocations for the same type return the same value.
template <typename T>
TypeId getTypeId();

// A convenience function that returns an initializer_list of TypeId values for the given type list.
template <typename L>
std::initializer_list<TypeId> getTypeIdsForList();

} // namespace impl
} // namespace fruit

#ifdef FRUIT_EXTRA_DEBUG

#include <ostream>

namespace fruit {
namespace impl {

inline std::ostream& operator<<(std::ostream& os, TypeId type) {
  return os << std::string(type);
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_EXTRA_DEBUG

namespace std {
  
template <>
struct hash<fruit::impl::TypeId> {
  std::size_t operator()(fruit::impl::TypeId type) const {
    return hash<const fruit::impl::TypeInfo*>()(type.type_info);
  }
};

} // namespace std

#include "type_info.defn.h"

#endif // FRUIT_TYPE_INFO_H
