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

#ifndef FRUIT_NORMALIZED_BINDINGS_DEFN_H
#define FRUIT_NORMALIZED_BINDINGS_DEFN_H

#include <fruit/impl/normalized_component_storage/normalized_bindings.h>

namespace fruit {
namespace impl {

inline NormalizedBinding::NormalizedBinding(ComponentStorageEntry entry) {
  switch (entry.kind) {
  case ComponentStorageEntry::Kind::BINDING_FOR_CONSTRUCTED_OBJECT:
    object = entry.binding_for_constructed_object.object_ptr;
    break;

  case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION:
  case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION:
  case ComponentStorageEntry::Kind::BINDING_FOR_OBJECT_TO_CONSTRUCT_WITH_UNKNOWN_ALLOCATION:
    create = entry.binding_for_object_to_construct.create;
    break;

  default:
#ifdef FRUIT_EXTRA_DEBUG
      std::cerr << "Unexpected kind: " << (std::size_t)entry.kind << std::endl;
#endif
    FRUIT_UNREACHABLE;
  }
}

} // namespace impl
} // namespace fruit

#endif // FRUIT_NORMALIZED_BINDINGS_DEFN_H
