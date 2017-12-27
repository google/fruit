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

#ifndef FRUIT_HASH_CODES_DEFN_H
#define FRUIT_HASH_CODES_DEFN_H

#include <fruit/impl/util/hash_codes.h>

namespace fruit {
namespace impl {

template <typename Tuple, int index>
inline std::size_t hashTupleElement(const Tuple& x) {
  const auto& value = std::get<index>(x);
  using T = typename std::remove_const<typename std::remove_reference<decltype(value)>::type>::type;
  return std::hash<T>()(value);
}

template <typename Tuple, int last_index>
struct HashTupleHelper;

template <int last_index>
struct HashTupleHelper<std::tuple<>, last_index> {
  std::size_t operator()(const std::tuple<>&) {
    return 0;
  }
};

template <typename... Args>
struct HashTupleHelper<std::tuple<Args...>, 1> {
  std::size_t operator()(const std::tuple<Args...>& x) {
    return hashTupleElement<std::tuple<Args...>, 0>(x);
  }
};

template <typename... Args, int last_index>
struct HashTupleHelper<std::tuple<Args...>, last_index> {
  std::size_t operator()(const std::tuple<Args...>& x) {
    return combineHashes(hashTupleElement<std::tuple<Args...>, last_index - 1>(x),
                         HashTupleHelper<std::tuple<Args...>, last_index - 1>()(x));
  }
};

template <typename... Args>
inline std::size_t hashTuple(const std::tuple<Args...>& x) {
  return HashTupleHelper<std::tuple<Args...>, sizeof...(Args)>()(x);
}

inline std::size_t combineHashes(std::size_t h1, std::size_t h2) {
  h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
  return h1;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_HASH_CODES_DEFN_H
