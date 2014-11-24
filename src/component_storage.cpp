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

#define IN_FRUIT_CPP_FILE

#include <cstdlib>
#include <memory>
#include <functional>
#include <vector>
#include <iostream>
#include "fruit/impl/util/demangle_type_name.h"
#include "fruit/impl/util/type_info.h"

#include "fruit/impl/storage/component_storage.h"

using std::cout;
using std::endl;

namespace fruit {
namespace impl {

void ComponentStorage::install(ComponentStorage other) {
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.bindings.capacity() > bindings.capacity()) {
    std::swap(bindings, other.bindings);
  }
  for (size_t i = 0; i < other.bindings_array_numUsed; i++) {
    if (bindings_array_numUsed < max_num_immediate_bindings) {
      bindings_array[bindings_array_numUsed] = other.bindings_array[i];
      ++bindings_array_numUsed;
    } else {
      bindings.push_back(other.bindings_array[i]);
    }
  }
  bindings.insert(bindings.end(),
                  other.bindings.begin(),
                  other.bindings.end());
  
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.multibindings.capacity() > multibindings.capacity()) {
    std::swap(multibindings, other.multibindings);
  }
  multibindings.insert(multibindings.end(),
                       other.multibindings.begin(),
                       other.multibindings.end());
  
  
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.compressed_bindings.capacity() > compressed_bindings.capacity()) {
    swap(other.compressed_bindings, compressed_bindings);
  }
  compressed_bindings.insert(compressed_bindings.end(), other.compressed_bindings.begin(), other.compressed_bindings.end());
}

ComponentStorage& ComponentStorage::flushBindings() {
  for (size_t i = 0; i < bindings_array_numUsed; i++) {
    bindings.push_back(bindings_array[i]);
  }
  return *this;
}

} // namespace impl
} // namespace fruit
