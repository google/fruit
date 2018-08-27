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

#ifndef FRUIT_ASSERT_H
#define FRUIT_ASSERT_H

#if FRUIT_EXTRA_DEBUG
#include <cassert>
// Usage: FruitStaticAssert(MetaExpr)
#define FruitStaticAssert(...) static_assert(fruit::impl::meta::Eval<__VA_ARGS__>::value, "")
#define FruitAssert(...) assert(__VA_ARGS__)

#else
// We still define this, otherwise some compilers (e.g. Clang 3.9) would complain that there's a stray ';' when this is
// used inside a struct/class definition.
#define FruitStaticAssert(...) static_assert(true, "")
#define FruitAssert(...)

#endif

#define FruitDelegateCheck(...) static_assert(true || sizeof(fruit::impl::meta::Eval<__VA_ARGS__>), "")
#define FruitDisplayErrorForType(...) static_assert(false && sizeof(fruit::impl::meta::Eval<__VA_ARGS__>), "")

#endif // FRUIT_ASSERT_H
