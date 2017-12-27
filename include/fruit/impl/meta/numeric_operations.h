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

struct Sum {
  template <typename... Ints>
  struct apply;

  template <int n, int m>
  struct apply<Int<n>, Int<m>> {
    using type = Int<n + m>;
  };
};

struct SumAll {
  template <typename... Ints>
  struct apply {
    using type = Int<0>;
  };

  template <typename N1, typename... Ints>
  struct apply<N1, Ints...> {
    using type = Int<N1::value + apply<Ints...>::type::value>;
  };

  // Optimization, not required for correctness.
  template <typename N0, typename N1, typename N2, typename N3, typename N4, typename... Ints>
  struct apply<N0, N1, N2, N3, N4, Ints...> {
    using type = Int<N0::value + N1::value + N2::value + N3::value + N4::value + apply<Ints...>::type::value>;
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
