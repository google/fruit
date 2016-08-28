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

void ComponentStorage::addBinding(std::tuple<TypeId, BindingData> t) throw() {
  bindings.push_front(std::make_pair(std::get<0>(t), std::get<1>(t)));
}

void ComponentStorage::addCompressedBinding(std::tuple<TypeId, TypeId, BindingData> t) throw() {
  compressed_bindings.push_front(CompressedBinding{std::get<0>(t), std::get<1>(t), std::get<2>(t)});
}

void ComponentStorage::addMultibinding(std::tuple<TypeId, MultibindingData> t) throw() {
  multibindings.emplace_back(std::get<0>(t), std::get<1>(t));
}

void ComponentStorage::install(ComponentStorage&& other) throw() {
  other.bindings.splice_after(other.bindings.before_begin(), std::move(bindings));
  bindings = std::move(other.bindings);
  
  other.compressed_bindings.splice_after(other.compressed_bindings.before_begin(), std::move(compressed_bindings));
  compressed_bindings = std::move(other.compressed_bindings);
  
  // Heuristic to try saving allocations/copies by appending to the biggest vector.
  if (other.multibindings.size() > multibindings.size()) {
    std::swap(multibindings, other.multibindings);
  }
  multibindings.insert(multibindings.end(),
                       other.multibindings.begin(),
                       other.multibindings.end());
}

void ComponentStorage::clear() throw() {
  std::size_t& numInstances = getNumComponentStorageInstancesInThread();
  FruitAssert(numInstances > 0);
  numInstances--;
  // When destroying the last ComponentStorage also clear the allocator, to avoid keeping
  // allocated memory for the rest of the program (that might well not create any more
  // ComponentStorage objects).
  if (numInstances == 0) {
    // We must clear the two lists first, since they still hold data allocated by the allocator.
    bindings.clear();
    compressed_bindings.clear();

    getBindingAllocator().clear();
  }
}

ComponentStorage::ComponentStorage() throw()
    : bindings(getBindingAllocator()),
      compressed_bindings(getBindingAllocator()) {

  getNumComponentStorageInstancesInThread()++;
}

ComponentStorage::ComponentStorage(ComponentStorage&& other) throw()
    : bindings(std::move(other.bindings)),
      compressed_bindings(std::move(other.compressed_bindings)),
      multibindings(std::move(other.multibindings)) {
  FruitAssert(!other.invalid);
  other.invalid = true;
  // Note that we don't need to update the number of valid ComponentStorage instances in this thread,
  // since we mark `other' as invalid.
}

ComponentStorage::~ComponentStorage() throw() {
  // For invalid objects the compiler can often optimize out the if entirely, reducing the size of the object file.
  if (!invalid) {
    clear();
  }
}

ComponentStorage::ComponentStorage(const ComponentStorage& other) throw()
    : bindings(other.bindings),
      compressed_bindings(other.compressed_bindings),
      multibindings(other.multibindings) {
  getNumComponentStorageInstancesInThread()++;
}

} // namespace impl
} // namespace fruit
