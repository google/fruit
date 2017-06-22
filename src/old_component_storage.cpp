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
#include <algorithm>
#include <fruit/impl/util/demangle_type_name.h>
#include <fruit/impl/util/type_info.h>

#include <fruit/impl/storage/to_port/old_component_storage.h>

using std::cout;
using std::endl;

namespace fruit {
namespace impl {

OldComponentStorage& OldComponentStorage::operator=(const OldComponentStorage& other) {
  bindings = other.bindings;
  compressed_bindings = other.compressed_bindings;
  multibindings = other.multibindings;

  lazy_components.resize(other.lazy_components.size());
  std::transform(other.lazy_components.begin(), other.lazy_components.end(), lazy_components.begin(),
      [](const OwningGenericLazyComponent& lazy_component) {
        return lazy_component.copy<true /* owning_pointer */>();
      });

  return *this;
}

void OldComponentStorage::install(OldComponentStorage&& other) throw() {
  bindings.insert(
      bindings.end(), other.bindings.begin(), other.bindings.end());
  compressed_bindings.insert(
      compressed_bindings.end(), other.compressed_bindings.begin(), other.compressed_bindings.end());
  multibindings.insert(multibindings.end(), other.multibindings.begin(), other.multibindings.end());

  lazy_components.insert(
      lazy_components.end(),
      std::make_move_iterator(other.lazy_components.begin()),
      std::make_move_iterator(other.lazy_components.end()));
}

OldComponentStorage::~OldComponentStorage() {
}

} // namespace impl
} // namespace fruit
