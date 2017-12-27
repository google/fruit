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

#include <fruit/impl/meta/basics.h>

namespace fruit {
namespace impl {
namespace meta {

template <bool... bs>
struct BoolVector {};

template <bool... bs>
using StaticAnd = Bool<std::is_same<BoolVector<bs...>, BoolVector<(true || bs)...>>::value>;

template <bool... bs>
using StaticOr = Bool<!std::is_same<BoolVector<bs...>, BoolVector<(false && bs)...>>::value>;

} // namespace meta
} // namespace impl
} // namespace fruit

#endif // FRUIT_META_LOGICAL_OPERATIONS_H
