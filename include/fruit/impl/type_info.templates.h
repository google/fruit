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

#ifndef FRUIT_TYPE_INFO_TEMPLATES_H
#define FRUIT_TYPE_INFO_TEMPLATES_H

namespace fruit {
namespace impl {

template <typename T>
inline const TypeInfo* getTypeInfo() noexcept {
  // The `constexpr' ensures compile-time evaluation.
#ifdef __GXX_RTTI
  static constexpr TypeInfo info = TypeInfo(typeid(T));
#else
  static constexpr TypeInfo info = TypeInfo();
#endif
  return &info;
}


} // namespace impl
} // namespace fruit

#endif // FRUIT_TYPE_INFO_TEMPLATES_H
