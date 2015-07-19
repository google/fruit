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

constexpr bool staticAnd() {
  return true;
}

template <typename... Ts>
constexpr bool staticAnd(bool first, Ts... others) {
  return first && staticAnd(others...);
}

// Overload with 10 elements just as an optimization, not required.
template <typename... Ts>
constexpr bool staticAnd(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8, bool b9, Ts... others) {
  return b0 && b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && staticAnd(others...);
}

constexpr bool staticOr() {
  return false;
}

template <typename... Ts>
constexpr bool staticOr(bool first, Ts... others) {
  return first || staticOr(others...);
}

// Overload with 10 elements just as an optimization.
template <typename... Ts>
constexpr bool staticOr(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8, bool b9, Ts... others) {
  return b0 || b1 || b2 || b3 || b4 || b5 || b6 || b7 || b8 || b9 || staticOr(others...);
}

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_LOGICAL_OPERATIONS_H
