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

#ifndef FRUIT_COMPONENT_STORAGE_INLINES_H
#define FRUIT_COMPONENT_STORAGE_INLINES_H

// Redundant, but makes KDevelop happy.
#include "component_storage.h"

namespace fruit {
namespace impl {

inline ComponentStorage::operator InjectorStorage() && {
  return InjectorStorage(std::move(typeRegistry), std::move(typeRegistryForMultibindings));
}

inline void ComponentStorage::check(bool b, const char* message) {
  check(b, [=](){return std::string(message);});
}

} // namespace impl
} // namespace fruit


#endif // FRUIT_COMPONENT_STORAGE_INLINES_H
