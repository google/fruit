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

#ifndef FRUIT_NORMALIZED_BINDINGS_H
#define FRUIT_NORMALIZED_BINDINGS_H

#include <fruit/impl/component_storage/component_storage_entry.h>
#include <memory>

namespace fruit {
namespace impl {

/** A single normalized binding (not a multibinding). */
struct NormalizedBinding {
  union {
    // Valid iff this is a terminal node (in the SemistaticGraph that contains this NormalizedBinding object).
    ComponentStorageEntry::BindingForConstructedObject::object_ptr_t object;

    // Valid iff this is not a terminal node  (in the SemistaticGraph that contains this NormalizedBinding object).
    ComponentStorageEntry::BindingForObjectToConstruct::create_t create;
  };

#if FRUIT_EXTRA_DEBUG
  bool is_nonconst;
#endif

  NormalizedBinding() = default;

  // Converts a ComponentStorageEntry to a NormalizedBinding.
  // This is only supported for entries with these kinds:
  // * BINDING_FOR_CONSTRUCTED_OBJECT,
  // * BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_ALLOCATION,
  // * BINDING_FOR_OBJECT_TO_CONSTRUCT_THAT_NEEDS_NO_ALLOCATION,
  explicit NormalizedBinding(ComponentStorageEntry entry);
};

/** A single normalized multibinding. */
struct NormalizedMultibinding {

  bool is_constructed;

  union {
    // Valid iff is_constructed==true.
    ComponentStorageEntry::MultibindingForConstructedObject::object_ptr_t object;

    // Valid iff is_constructed==false.
    ComponentStorageEntry::MultibindingForObjectToConstruct::create_t create;
  };
};

/** This stores all multibindings for a given type_id. */
struct NormalizedMultibindingSet {

  // Can be empty, but only if v is present and non-empty.
  std::vector<NormalizedMultibinding> elems;

  // TODO: Check this comment.
  // Returns the std::vector<T*> of instances, or nullptr if none.
  // Caches the result in the `v' member.
  ComponentStorageEntry::MultibindingVectorCreator::get_multibindings_vector_t get_multibindings_vector;

  // A (casted) pointer to the std::vector<T*> of objects, or nullptr if the vector hasn't been constructed yet.
  // Can't be empty.
  std::shared_ptr<char> v;
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/normalized_component_storage/normalized_bindings.defn.h>

#endif // FRUIT_NORMALIZED_BINDINGS_H
