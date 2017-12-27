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

#ifndef FRUIT_META_TRIPLET_H
#define FRUIT_META_TRIPLET_H

#include <fruit/impl/meta/basics.h>

namespace fruit {
namespace impl {
namespace meta {

template <typename First1, typename Second1, typename Third1>
struct Triplet {
  using First = First1;
  using Second = Second1;
  using Third = Third1;
};

struct ConsTriplet {
  template <typename First, typename Second, typename Third>
  struct apply {
    using type = Triplet<First, Second, Third>;
  };
};

struct GetThird {
  template <typename T>
  struct apply {
    using type = typename T::Third;
  };
};

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_TRIPLET_H
