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

#ifndef FRUIT_NORMALIZED_COMPONENT_STORAGE_DEFN_H
#define FRUIT_NORMALIZED_COMPONENT_STORAGE_DEFN_H

#include <fruit/impl/normalized_component_storage/normalized_component_storage.h>

namespace fruit {
namespace impl {

inline NormalizedComponentStorage& NormalizedComponentStorage::operator=(NormalizedComponentStorage&& other) {
  bindings = std::move(other.bindings);
  multibindings = std::move(other.multibindings);
  fixed_size_allocator_data = std::move(other.fixed_size_allocator_data);

  // We must destroy `bindingCompressionInfoMap` before its memory pool, so we clear it explicitly.
  bindingCompressionInfoMap = std::unique_ptr<BindingCompressionInfoMap>();
  bindingCompressionInfoMap = std::move(other.bindingCompressionInfoMap);

  bindingCompressionInfoMapMemoryPool = std::move(other.bindingCompressionInfoMapMemoryPool);

  return *this;
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_NORMALIZED_COMPONENT_STORAGE_DEFN_H
