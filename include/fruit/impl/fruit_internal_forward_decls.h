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

#ifndef FRUIT_FRUIT_INTERNAL_FORWARD_DECLS_H
#define FRUIT_FRUIT_INTERNAL_FORWARD_DECLS_H

#include <memory>
#include <vector>

namespace fruit {
namespace impl {

class ComponentStorage;
class NormalizedComponentStorage;
class InjectorStorage;
struct TypeId;
struct ComponentStorageEntry;
struct NormalizedBinding;
struct NormalizedMultibinding;
struct NormalizedMultibindingSet;
struct InjectorAccessorForTests;

template <typename Component, typename... Args>
class LazyComponentImpl;

namespace meta {
template <typename... PreviousBindings>
struct OpForComponent;
}

} // namespace impl

} // namespace fruit

#endif // FRUIT_FRUIT_INTERNAL_FORWARD_DECLS_H
