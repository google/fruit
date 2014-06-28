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

#ifndef FRUIT_BASIC_UTILS_H
#define FRUIT_BASIC_UTILS_H

#include <memory>

namespace fruit {
namespace impl {

// General case, if none of the following apply.
// When adding a specialization here, make sure that the ComponentStorage
// can actually get<> the specified type when the class was registered.
template <typename T>
struct GetClassForTypeHelper {using type = T;};

template <typename T>
struct GetClassForTypeHelper<const T> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<T*> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<T&> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<const T*> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<const T&> {using type = T;};

template <typename T>
struct GetClassForTypeHelper<std::shared_ptr<T>> {using type = T;};

template <typename T>
using GetClassForType = typename GetClassForTypeHelper<T>::type;

template <typename T>
struct NopDeleter {
  void operator()(T*) {
  }
};


} // namespace impl
} // namespace fruit


#endif // FRUIT_BASIC_UTILS_H
