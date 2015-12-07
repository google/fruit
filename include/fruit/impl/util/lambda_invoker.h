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

#ifndef FRUIT_LAMBDA_INVOKER_H
#define FRUIT_LAMBDA_INVOKER_H

#include "../injection_errors.h"
#include "../meta/errors.h"
#include "../meta/wrappers.h"

#include <type_traits>
#include <functional>
#include <cstddef>

namespace fruit {
namespace impl {

class LambdaInvoker {
private:
  // We reinterpret-cast a char[] to avoid de-referencing nullptr, which would technically be
  // undefined behavior (even though we would not access any data there anyway).
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ * 10 + __GNUC_MINOR__) >= 49)
  alignas(std::max_align_t) static char buf[1];
#else
  // In GCC 4.8.x, we need a non-standard max_align_t.
  alignas(::max_align_t) static char buf[1];
#endif
  
  
public:
  template <typename F, typename... Args>
  static auto invoke(Args... args) -> decltype(std::declval<const F&>()(args...)) {
    FruitStaticAssert(fruit::impl::meta::IsEmpty(fruit::impl::meta::Type<F>));
    FruitStaticAssert(fruit::impl::meta::IsTriviallyCopyable(fruit::impl::meta::Type<F>));
    // Since `F' is empty, a valid value of type F is already stored at the beginning of buf.
    F* __attribute__((__may_alias__)) f = reinterpret_cast<F*>(buf);
    return (*f)(args...);
  }
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_LAMBDA_INVOKER_H
