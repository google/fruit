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

#ifndef FRUIT_CONFIG_BASE_H
#define FRUIT_CONFIG_BASE_H

// Needed for all Clang versions (as of January 2016), not needed for GCC.
// This can also be defined for GCC, but it slightly slows down compile time of code using Fruit.
#define FRUIT_HAS_CLANG_ARBITRARY_OVERLOAD_RESOLUTION_BUG 1

// Whether the compiler defines std::max_align_t.
#define FRUIT_HAS_STD_MAX_ALIGN_T 1

// Whether the compiler defines ::max_align_t.
// Ignored if FRUIT_HAS_STD_MAX_ALIGN_T is set.
#define FRUIT_HAS_MAX_ALIGN_T 1

// Whether the compiler defines std::is_trivially_copyable.
#define FRUIT_HAS_STD_IS_TRIVIALLY_COPYABLE 1

// Whether the compiler defines __has_trivial_copy.
// Ignored if FRUIT_HAS_STD_IS_TRIVIALLY_COPYABLE is set.
#define FRUIT_HAS_HAS_TRIVIAL_COPY 1

// Whether the compiler defines __is_trivially_copyable.
// Ignored if FRUIT_HAS_STD_IS_TRIVIALLY_COPYABLE is set.
#define FRUIT_HAS_IS_TRIVIALLY_COPYABLE 1

// Whether the compiler defines std::is_trivially_copy_constructible.
#define FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE 1

// Whether typeid() is available. Typically, it is unless RTTI is disabled.
#define FRUIT_HAS_TYPEID 1

// Whether typeid() is constexpr. Typically, it is except in MSVC.
#define FRUIT_HAS_CONSTEXPR_TYPEID 1

// Whether abi::__cxa_demangle() is available after including cxxabi.h.
#define FRUIT_HAS_CXA_DEMANGLE 1

#define FRUIT_USES_BOOST 1

#define FRUIT_HAS_ALWAYS_INLINE_ATTRIBUTE 1

#define FRUIT_HAS_FORCEINLINE 0

#define FRUIT_HAS_ATTRIBUTE_DEPRECATED 0

#define FRUIT_HAS_GCC_ATTRIBUTE_DEPRECATED 1

#define FRUIT_HAS_DECLSPEC_DEPRECATED 0

#define FRUIT_HAS_MSVC_ASSUME 0

#define FRUIT_HAS_BUILTIN_UNREACHABLE 1

#endif // FRUIT_CONFIG_BASE_H
