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

#ifndef FRUIT_TYPE_INFO_DEFN_H
#define FRUIT_TYPE_INFO_DEFN_H

#include <fruit/impl/util/type_info.h>

#include <fruit/impl/fruit-config.h>
#include <fruit/fruit_forward_decls.h>

namespace fruit {
namespace impl {
  
// This should only be used if RTTI is disabled. Use the other constructor if possible.
inline constexpr TypeInfo::TypeInfo(std::size_t type_size, std::size_t type_alignment, bool is_trivially_destructible)
: info(nullptr), type_size(type_size), type_alignment(type_alignment), is_trivially_destructible(is_trivially_destructible) {
}

inline constexpr TypeInfo::TypeInfo(const std::type_info& info, std::size_t type_size, std::size_t type_alignment, 
                    bool is_trivially_destructible)
: info(&info), type_size(type_size), type_alignment(type_alignment), is_trivially_destructible(is_trivially_destructible) {
}

inline std::string TypeInfo::name() const {
  if (info != nullptr)
    return demangleTypeName(info->name());
  else
    return "<unknown> (type name not accessible due to -fno-rtti)";
}

inline size_t TypeInfo::size() const {
  return type_size;
}  

inline size_t TypeInfo::alignment() const {
  return type_alignment;
}  

inline bool TypeInfo::isTriviallyDestructible() const {
  return is_trivially_destructible;
}
  
inline TypeId::operator std::string() const {
  return type_info->name();
}

inline bool TypeId::operator==(TypeId x) const {
  return type_info == x.type_info;
}

inline bool TypeId::operator!=(TypeId x) const {
  return type_info != x.type_info;
}

inline bool TypeId::operator<(TypeId x) const {
  return type_info < x.type_info;
}

template <typename T>
struct GetTypeInfoForType {
  constexpr TypeInfo operator()() const {
#ifdef FRUIT_HAS_TYPEID
    return TypeInfo(typeid(T), sizeof(T), alignof(T), std::is_trivially_destructible<T>::value);
#else
    return TypeInfo(sizeof(T), alignof(T), std::is_trivially_destructible<T>::value);
#endif
  };
};

template <typename Annotation, typename T>
struct GetTypeInfoForType<fruit::Annotated<Annotation, T>> {
  constexpr TypeInfo operator()() const {
#ifdef FRUIT_HAS_TYPEID
    return TypeInfo(typeid(fruit::Annotated<Annotation, T>), sizeof(T), alignof(T), std::is_trivially_destructible<T>::value);
#else
    return TypeInfo(sizeof(T), alignof(T), std::is_trivially_destructible<T>::value);
#endif
  };
};

template <typename T>
inline TypeId getTypeId() {
  // The `constexpr' ensures compile-time evaluation.
  static constexpr TypeInfo info = GetTypeInfoForType<T>()();
  return TypeId{&info};
}

template <typename L>
struct GetTypeIdsForListHelper;

template <typename... Ts>
struct GetTypeIdsForListHelper<fruit::impl::meta::Vector<Ts...>> {
  std::vector<TypeId> operator()() {
    return std::vector<TypeId>{getTypeId<Ts>()...};
  }
};

template <typename L>
std::vector<TypeId> getTypeIdsForList() {
  return GetTypeIdsForListHelper<L>()();
}

#ifdef FRUIT_EXTRA_DEBUG

inline std::ostream& operator<<(std::ostream& os, TypeId type) {
  return os << std::string(type);
}

#endif // FRUIT_EXTRA_DEBUG

} // namespace impl
} // namespace fruit

namespace std {
  
inline std::size_t hash<fruit::impl::TypeId>::operator()(fruit::impl::TypeId type) const {
  return hash<const fruit::impl::TypeInfo*>()(type.type_info);
}

} // namespace std

#endif // FRUIT_TYPE_INFO_DEFN_H
