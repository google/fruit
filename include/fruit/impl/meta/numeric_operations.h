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

#ifndef FRUIT_META_NUMERIC_OPERATIONS_H
#define FRUIT_META_NUMERIC_OPERATIONS_H

#include "basics.h"

namespace fruit {
namespace impl {
namespace meta {

constexpr int staticSum() {
  return 0;
}

template <typename... Ts>
constexpr int staticSum(int n, Ts... others) {
  return n + staticSum(others...);
}

struct Sum {
  template <typename... Ints>
  struct apply;
  
  template <int... ns>
  struct apply<Int<ns>...> {
    using type = Int<staticSum(ns...)>;
  };
};

template <int...>
struct IntVector {};


struct GenerateIntSequenceEvenHelper {
  template <typename Half>
  struct apply;
  
  template <int... ns>
  struct apply<IntVector<ns...>> {
    using type = IntVector<ns..., (sizeof...(ns) + ns)...>;
  };
};

struct GenerateIntSequenceOddHelper {
  template <typename Half>
  struct apply;
  
  template <int... ns>
  struct apply<IntVector<ns...>> {
    using type = IntVector<ns..., sizeof...(ns), (sizeof...(ns) + 1 + ns)...>;
  };
};

template <int length>
struct GenerateIntSequenceImpl {
  using type = typename std::conditional<(length % 2) == 0,
                                          GenerateIntSequenceEvenHelper,
                                          GenerateIntSequenceOddHelper
                                        >::type::template apply<
                                          typename GenerateIntSequenceImpl<length/2>::type
                                        >::type;
};

template <>
struct GenerateIntSequenceImpl<0> {
  using type = IntVector<>;
};

template <>
struct GenerateIntSequenceImpl<1> {
  using type = IntVector<0>;
};

template <int n>
using GenerateIntSequence = typename GenerateIntSequenceImpl<n>::type;


} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_NUMERIC_OPERATIONS_H
