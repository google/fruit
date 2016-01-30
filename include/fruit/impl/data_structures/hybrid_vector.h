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

#ifndef FRUIT_HYBRID_VECTOR_H
#define FRUIT_HYBRID_VECTOR_H

#include <vector>

namespace fruit {
namespace impl {

// Similar to std::vector<T>, but stores up to num_immediate_values in an array to avoid a memory allocation if the vector is
// non-empty but small.
template <typename T, int num_immediate_values>
class HybridVector {
private:
  // The first num_immediate_values values are stored here, to avoid a memory allocation if the vector is small.
  T values_array[num_immediate_values];
  std::size_t values_array_num_used = 0;
  
  std::vector<T> values;
  
public:
  void push_back(T x);
  
  // Inserts all the values in `other' into this object, in an unspecified position.
  void insert(HybridVector&& other);
  
  operator std::vector<T>() &&;
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/data_structures/hybrid_vector.defn.h>

#endif // FRUIT_HYBRID_VECTOR_H
