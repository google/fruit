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

#ifndef FRUIT_HYBRID_VECTOR_DEFN_H
#define FRUIT_HYBRID_VECTOR_DEFN_H

#include <fruit/impl/data_structures/hybrid_vector.h>

namespace fruit {
namespace impl {

template <typename T, int num_immediate_values>
inline void HybridVector<T, num_immediate_values>::push_back(T x) {
  if (values_array_num_used < num_immediate_values) {
    values_array[values_array_num_used] = std::move(x);
    ++values_array_num_used;
  } else {
    values.push_back(std::move(x));
  }
}
  
template <typename T, int num_immediate_values>
inline void HybridVector<T, num_immediate_values>::insert(HybridVector&& other) {
  for (std::size_t i = 0; i < other.values_array_num_used; i++) {
    push_back(other.values_array[i]);
  }
  // Heuristic to try saving an allocation by appending to the largest vector.
  if (other.values.capacity() > values.capacity()) {
    std::swap(other.values, values);
  }
  values.insert(values.end(),
                other.values.begin(),
                other.values.end());
}

template <typename T, int num_immediate_values>
inline HybridVector<T, num_immediate_values>::operator std::vector<T>() && {
  for (std::size_t i = 0; i < values_array_num_used; i++) {
    values.push_back(values_array[i]);
  }
  return std::move(values);
}


} // namespace impl
} // namespace fruit

#endif // FRUIT_HYBRID_VECTOR_DEFN_H
