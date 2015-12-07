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

#define IN_FRUIT_CPP_FILE

#include "fruit/impl/util/lambda_invoker.h"

namespace fruit {
namespace impl {

#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ * 10 + __GNUC_MINOR__) >= 49)
  alignas(std::max_align_t) char LambdaInvoker::buf[1] = {0};
#else
  // In GCC 4.8.x, we need a non-standard max_align_t.
  alignas(::max_align_t) char LambdaInvoker::buf[1] = {0};
#endif

} // namespace impl
} // namespace fruit
