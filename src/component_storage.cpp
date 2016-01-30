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
#include <fruit/impl/util/demangle_type_name.h>
#include <fruit/impl/util/type_info.h>

#include <fruit/impl/storage/component_storage.h>

using std::cout;
using std::endl;

namespace fruit {
namespace impl {

void ComponentStorage::install(ComponentStorage other) {
  bindings.insert(std::move(other.bindings));
  
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.multibindings.capacity() > multibindings.capacity()) {
    std::swap(multibindings, other.multibindings);
  }
  multibindings.insert(multibindings.end(),
                       other.multibindings.begin(),
                       other.multibindings.end());
  
  compressed_bindings.insert(std::move(other.compressed_bindings));
}

} // namespace impl
} // namespace fruit
