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

#include <fruit/fruit_forward_decls.h>
#include <fruit/impl/data_structures/arena_allocator.h>
#include <fruit/impl/data_structures/memory_pool.h>
#include <fruit/impl/fruit-config.h>
#include <fruit/impl/fruit_assert.h>

namespace fruit {
namespace impl {

template <typename T, bool is_abstract = std::is_abstract<T>::value>
struct GetConcreteTypeInfo {
  constexpr TypeInfo::ConcreteTypeInfo operator()() const {
    return TypeInfo::ConcreteTypeInfo{
        sizeof(T), alignof(T), std::is_trivially_destructible<T>::value,
#if FRUIT_EXTRA_DEBUG
        false /* is_abstract */,
#endif
    };
  }
};

// For abstract types we don't need the real information.
// Also, some compilers might report compile errors in this case, for example alignof(T) doesn't work in Visual Studio
// when T is an abstract type.
template <typename T>
struct GetConcreteTypeInfo<T, true> {
  constexpr TypeInfo::ConcreteTypeInfo operator()() const {
    return TypeInfo::ConcreteTypeInfo{
        0 /* type_size */, 0 /* type_alignment */, false /* is_trivially_destructible */,
#if FRUIT_EXTRA_DEBUG
        true /* is_abstract */,
#endif
    };
  }
};

// This should only be used if RTTI is disabled. Use the other constructor if possible.
inline constexpr TypeInfo::TypeInfo(ConcreteTypeInfo concrete_type_info)
    : info(nullptr), concrete_type_info(concrete_type_info) {}

inline constexpr TypeInfo::TypeInfo(const std::type_info& info, ConcreteTypeInfo concrete_type_info)
    : info(&info), concrete_type_info(concrete_type_info) {}

inline std::string TypeInfo::name() const {
  if (info != nullptr) // LCOV_EXCL_BR_LINE
    return demangleTypeName(info->name());
  else
    return "<unknown> (type name not accessible because RTTI is disabled)"; // LCOV_EXCL_LINE
}

inline size_t TypeInfo::size() const {
#if FRUIT_EXTRA_DEBUG
  FruitAssert(!concrete_type_info.is_abstract);
#endif
  return concrete_type_info.type_size;
}

inline size_t TypeInfo::alignment() const {
#if FRUIT_EXTRA_DEBUG
  FruitAssert(!concrete_type_info.is_abstract);
#endif
  return concrete_type_info.type_alignment;
}

inline bool TypeInfo::isTriviallyDestructible() const {
#if FRUIT_EXTRA_DEBUG
  FruitAssert(!concrete_type_info.is_abstract);
#endif
  return concrete_type_info.is_trivially_destructible;
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
#if FRUIT_HAS_TYPEID
    return TypeInfo(typeid(T), GetConcreteTypeInfo<T>()());
#else
    return TypeInfo(GetConcreteTypeInfo<T>()());
#endif
  };
};

template <typename Annotation, typename T>
struct GetTypeInfoForType<fruit::Annotated<Annotation, T>> {
  constexpr TypeInfo operator()() const {
#if FRUIT_HAS_TYPEID
    return TypeInfo(typeid(fruit::Annotated<Annotation, T>), GetConcreteTypeInfo<T>()());
#else
    return TypeInfo(GetConcreteTypeInfo<T>()());
#endif
  };
};

template <typename T>
inline TypeId getTypeId() {
#if FRUIT_HAS_TYPEID && !FRUIT_HAS_CONSTEXPR_TYPEID
  // We can't use constexpr here because TypeInfo contains a `const std::type_info&` and that's not constexpr with the
  // current compiler/STL.
  static TypeInfo info = GetTypeInfoForType<T>()();
#else
  // Usual case. The `constexpr' ensures compile-time evaluation.
  static constexpr TypeInfo info = GetTypeInfoForType<T>()();
#endif
  return TypeId{&info};
}

template <typename L>
struct GetTypeIdsForListHelper;

template <typename... Ts>
struct GetTypeIdsForListHelper<fruit::impl::meta::Vector<Ts...>> {
  std::vector<TypeId, ArenaAllocator<TypeId>> operator()(MemoryPool& memory_pool) {
    return std::vector<TypeId, ArenaAllocator<TypeId>>(std::initializer_list<TypeId>{getTypeId<Ts>()...}, memory_pool);
  }
};

template <typename L>
std::vector<TypeId, ArenaAllocator<TypeId>> getTypeIdsForList(MemoryPool& memory_pool) {
  return GetTypeIdsForListHelper<L>()(memory_pool);
}

#if FRUIT_EXTRA_DEBUG

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
