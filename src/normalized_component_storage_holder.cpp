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

#define IN_FRUIT_CPP_FILE 1

#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>
#include <fruit/impl/normalized_component_storage/normalized_component_storage_holder.h>

using namespace fruit;
using namespace fruit::impl;

namespace fruit {
namespace impl {

NormalizedComponentStorageHolder::NormalizedComponentStorageHolder(
    ComponentStorage&& component, const std::vector<TypeId, ArenaAllocator<TypeId>>& exposed_types,
    MemoryPool& memory_pool, WithUndoableCompression)
    : storage(new NormalizedComponentStorage(std::move(component), exposed_types, memory_pool,
                                             NormalizedComponentStorage::WithUndoableCompression())) {}

NormalizedComponentStorageHolder::~NormalizedComponentStorageHolder() {}

} // namespace impl
} // namespace fruit
