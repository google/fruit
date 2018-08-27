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

#ifndef FRUIT_TEST_MACROS_H
#define FRUIT_TEST_MACROS_H

#include <iostream>

#define Assert(...)                                                                                                    \
  do {                                                                                                                 \
    if (!(__VA_ARGS__)) {                                                                                              \
      std::cerr << __FILE__ << ":" << __LINE__ << ": " << __func__ << ": Assertion \"" << #__VA_ARGS__ << "\" failed." \
                << std::endl;                                                                                          \
      abort();                                                                                                         \
    }                                                                                                                  \
  } while (false)

#define InstantiateType(...)                                                                                           \
  void f() {                                                                                                           \
    (void)sizeof(__VA_ARGS__);                                                                                         \
  }

#endif // FRUIT_TEST_MACROS_H
