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

#include <fruit/impl/meta/basics.h>

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

// Overload with 10 elements just as an optimization, not required.
template <typename... Ts>
constexpr int staticSum(int n0, int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int n9, Ts... others) {
  return n0 + n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9 + staticSum(others...);
}

struct Sum {
  template <typename... Ints>
  struct apply;
  
  template <int n, int m>
  struct apply<Int<n>, Int<m>> {
    using type = Int<n + m>;
  };
};

struct Minus {
  template <typename N, typename M>
  struct apply;
  
  template <int n, int m>
  struct apply<Int<n>, Int<m>> {
    using type = Int<n - m>;
  };
};

struct GreaterThan {
  template <typename N, typename M>
  struct apply;
  
  template <int n, int m>
  struct apply<Int<n>, Int<m>> {
    using type = Bool<(n > m)>;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit


#endif // FRUIT_META_NUMERIC_OPERATIONS_H
