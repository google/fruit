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

#ifndef FRUIT_BINDING_NORMALIZATION_H
#define FRUIT_BINDING_NORMALIZATION_H

#ifndef IN_FRUIT_CPP_FILE
// We don't want to include it in public headers to save some compile time.
#error "binding_normalization.h included in non-cpp file."
#endif

#include <fruit/impl/util/hash_helpers.h>

namespace fruit {
namespace impl {

/**
 * This struct contains helper functions used for binding normalization.
 * They are wrapped in a struct so that Fruit classes can easily declare to be friend
 * of all these.
 */
struct BindingNormalization {
  
  struct BindingCompressionInfo {
    TypeId iTypeId;
    BindingData iBinding;
    BindingData cBinding;
  };
  // Stores an element of the form (cTypeId, (iTypeId, iBinding, cBinding)) for each binding compression that was performed.
  // These are used to undo binding compression after applying it (if necessary).
  using BindingCompressionInfoMap = HashMap<TypeId, BindingCompressionInfo>;
  
  // bindingCompressionInfoMap is an output parameter. This function will store
  // information on all performed binding compressions
  // in that map, to allow them to be undone later, if necessary.
  static std::vector<std::pair<TypeId, BindingData>> normalizeBindings(
      const std::vector<std::pair<TypeId, BindingData>>& bindings_vector,
      FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
      std::vector<CompressedBinding>&& compressed_bindings_vector,
      const std::vector<std::pair<TypeId, MultibindingData>>& multibindings,
      const std::vector<TypeId>& exposed_types,
      BindingCompressionInfoMap& bindingCompressionInfoMap);

  static void addMultibindings(std::unordered_map<TypeId, NormalizedMultibindingData>& multibindings,
                               FixedSizeAllocator::FixedSizeAllocatorData& fixed_size_allocator_data,
                               const std::vector<std::pair<TypeId, MultibindingData>>& multibindings_vector);
  
};

} // namespace impl
} // namespace fruit

#endif // FRUIT_BINDING_NORMALIZATION_H
