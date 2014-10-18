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

#include <type_traits>
#include <functional>

namespace fruit {
namespace impl {

class LambdaInvoker {
private:
  // We reinterpret-cast a reference to x to avoid de-referencing nullptr, which would be technically be undefined behavior (even
  // though we would not access any data there anyway).
  static const int x;
  
public:
  template <typename F, typename... Args>
  static auto invoke(Args... args) -> decltype(reinterpret_cast<const F&>(x)(args...)) {
    // TODO: Move to injection_errors.
    static_assert(std::is_empty<F>::value,
                  "Error: only lambdas with no captures are supported, and those should satisfy is_empty. If this error happens for a lambda with no captures, please file a bug at https://github.com/google/fruit/issues .");
    // Since `F' is empty, a valid value of type F is already stored starting at &x.
    return reinterpret_cast<const F&>(x)(args...);
  }
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_LAMBDA_INVOKER_H
