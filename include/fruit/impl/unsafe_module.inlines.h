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

#ifndef FRUIT_UNSAFE_MODULE_INLINES_H
#define FRUIT_UNSAFE_MODULE_INLINES_H

namespace fruit {
namespace impl {

inline UnsafeModule::UnsafeModule(const UnsafeModule& other)
  : typeRegistry(other.typeRegistry) {
  // Can't copy the module once it starts owning resources (singleton instances).
  check(other.createdSingletons.empty(), "Attempting to copy a module that has already started creating instances");
}

inline UnsafeModule::UnsafeModule(UnsafeModule&& other) {
  swap(other);
}

inline void UnsafeModule::swap(UnsafeModule& other) {
  std::swap(typeRegistry, other.typeRegistry);
  std::swap(createdSingletons, other.createdSingletons);
}

inline void UnsafeModule::check(bool b, const char* message) {
  check(b, [=](){return std::string(message);});
}

} // namespace impl
} // namespace fruit


#endif // FRUIT_UNSAFE_MODULE_INLINES_H