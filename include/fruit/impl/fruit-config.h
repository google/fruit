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

#ifndef FRUIT_CONFIG_H
#define FRUIT_CONFIG_H

#include <fruit/impl/fruit-config-base.h>

#if FRUIT_HAS_STD_IS_TRIVIALLY_COPYABLE
#if FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T)                                                                                 \
  (std::is_trivially_copyable<T>::value || (std::is_empty<T>::value && std::is_trivially_copy_constructible<T>::value))
#else // !FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) (std::is_trivially_copyable<T>::value)
#endif // FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#elif FRUIT_HAS_IS_TRIVIALLY_COPYABLE
#if FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T)                                                                                 \
  (__is_trivially_copyable(T) || (std::is_empty<T>::value && std::is_trivially_copy_constructible<T>::value))
#else // !FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) (__is_trivially_copyable(T))
#endif // FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#elif FRUIT_HAS_HAS_TRIVIAL_COPY
// The compiler doesn't support __is_trivially_copyable (nor is std::is_trivially_copyable
// supported by the library). We use this check as a proxy, but it's not exactly the same thing.
#if FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T)                                                                                 \
  (__has_trivial_copy(T) || (std::is_empty<T>::value && std::is_trivially_copy_constructible<T>::value))
#else // !FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) (__has_trivial_copy(T))
#endif // FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#else
// We use the standard one, but most likely it won't work.
#if FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T)                                                                                 \
  (std::is_trivially_copyable<T>::value || (std::is_empty<T>::value && std::is_trivially_copy_constructible<T>::value))
#else // !FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) (std::is_trivially_copyable<T>::value)
#endif // FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#endif

#if FRUIT_HAS_ALWAYS_INLINE_ATTRIBUTE
#define FRUIT_ALWAYS_INLINE __attribute__((always_inline))
#elif FRUIT_HAS_FORCEINLINE
#define FRUIT_ALWAYS_INLINE __forceinline
#else
#define FRUIT_ALWAYS_INLINE
#endif

#if FRUIT_HAS_GCC_ATTRIBUTE_DEPRECATED
#define FRUIT_DEPRECATED_DECLARATION(...) __VA_ARGS__ __attribute__((deprecated))
// Marking the declaration is enough.
#define FRUIT_DEPRECATED_DEFINITION(...) __VA_ARGS__
#elif FRUIT_HAS_DECLSPEC_DEPRECATED
#define FRUIT_DEPRECATED_DECLARATION(...) __declspec(deprecated) __VA_ARGS__
#define FRUIT_DEPRECATED_DEFINITION(...) __declspec(deprecated) __VA_ARGS__
// We use this only if the above two are not supported, because some compilers "support" this syntax (i.e., it compiles)
// but they just ignore the attribute.
#elif FRUIT_HAS_ATTRIBUTE_DEPRECATED
#define FRUIT_DEPRECATED_DECLARATION(...) [[deprecated]] __VA_ARGS__
#define FRUIT_DEPRECATED_DEFINITION(...) [[deprecated]] __VA_ARGS__
#else
#define FRUIT_DEPRECATED_DECLARATION(...) __VA_ARGS__
#define FRUIT_DEPRECATED_DEFINITION(...) __VA_ARGS__
#endif

#if FRUIT_HAS_MSVC_ASSUME
#define FRUIT_UNREACHABLE                                                                                              \
  FruitAssert(false);                                                                                                  \
  __assume(0)
#elif FRUIT_HAS_BUILTIN_UNREACHABLE
#define FRUIT_UNREACHABLE                                                                                              \
  FruitAssert(false);                                                                                                  \
  __builtin_unreachable()
#endif

#endif // FRUIT_CONFIG_H
