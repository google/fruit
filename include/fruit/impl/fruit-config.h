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

#if FRUIT_HAS_STD_MAX_ALIGN_T
#define FRUIT_MAX_ALIGN_T std::max_align_t
#elif FRUIT_HAS_MAX_ALIGN_T
// In e.g. GCC 4.8.x, we need a non-standard max_align_t.
#define FRUIT_MAX_ALIGN_T ::max_align_t
#else
// We use the standard one, but most likely it won't work.
#define FRUIT_MAX_ALIGN_T std::max_align_t
#endif

#if FRUIT_HAS_STD_IS_TRIVIALLY_COPYABLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) std::is_trivially_copyable<T>::value
#elif FRUIT_HAS_IS_TRIVIALLY_COPYABLE
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) __is_trivially_copyable(T)
#elif FRUIT_HAS_HAS_TRIVIAL_COPY
// The compiler doesn't support __is_trivially_copyable (nor is std::is_trivially_copyable
// supported by the library). We use this check as a proxy, but it's not exactly the same thing.
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) __has_trivial_copy(T)
#else
// We use the standard one, but most likely it won't work.
#define FRUIT_IS_TRIVIALLY_COPYABLE(T) std::is_trivially_copyable<T>::value
#endif

#endif // FRUIT_CONFIG_H
