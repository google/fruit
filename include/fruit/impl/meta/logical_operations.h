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

#ifndef FRUIT_META_LOGICAL_OPERATIONS_H
#define FRUIT_META_LOGICAL_OPERATIONS_H

namespace fruit {
namespace impl {
namespace meta {

// TODO: Consider providing an optimized version of each (e.g. with 5 + n params).

// General case: empty.
template <bool... bs>
struct StaticAnd {
  static constexpr bool value = true;
};

template <bool b, bool... bs>
struct StaticAnd<b, bs...> {
  static constexpr bool value = b && StaticAnd<bs...>::value;  
};

template <bool... bs>
struct StaticOr {
  static constexpr bool value = false;
};

template <bool b, bool... bs>
struct StaticOr<b, bs...> {
  static constexpr bool value = b || StaticOr<bs...>::value;  
};

// General case: nothing to sum.
template <int... is>
struct StaticSum {
  static constexpr int value = 0;
};

template <int i, int... is>
struct StaticSum<i, is...> {
  static constexpr int value = i + StaticSum<is...>::value;
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_LOGICAL_OPERATIONS_H
