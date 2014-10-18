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

#include "fruit/impl/semistatic_map.h"
#include "fruit/impl/normalized_component_storage.h"

#include <algorithm>
#include <random>
#include <utility>

using namespace fruit::impl;

template <typename Key, typename Value>
SemistaticMap<Key, Value>::SemistaticMap(typename std::vector<std::pair<Key, Value>>::const_iterator first,
                                         typename std::vector<std::pair<Key, Value>>::const_iterator last) {
  using Iter = typename std::vector<std::pair<Key, Value>>::const_iterator;
  std::size_t n = last - first;
  NumBits num_bits = pickNumBits(n);
  std::size_t num_buckets = (1 << num_bits);
  
  std::vector<Unsigned> count;
  count.reserve(num_buckets);
  
  hash_function.shift = (sizeof(Unsigned)*CHAR_BIT - num_bits);
  
  std::default_random_engine random_generator;
  std::uniform_int_distribution<Unsigned> random_distribution;
  
  while (1) {
    hash_function.a = random_distribution(random_generator);
    count.assign(num_buckets, 0);
    
    for (Iter itr = first; itr != last; ++itr) {
      Unsigned& thisCount = count[hash(itr->first)];
      ++thisCount;
      if (thisCount == beta) {
        goto pick_another;
      }
    }
    break;
    
    pick_another:;
  }
  
  std::partial_sum(count.begin(), count.end(), count.begin());
  lookup_table = std::move(count);
  values.resize(n);
  
  // At this point lookup_table[h] is the number of keys in [first, last) that have a hash <=h.
  // Note that even though we ensure this after construction, it is not maintained by insert() so it's not an invariant.
  
  for (Iter itr = first; itr != last; ++itr) {
    Unsigned& cell = lookup_table[hash(itr->first)];
    --cell;
    assert(cell < n);
    values[cell] = *itr;
  }
}

template SemistaticMap<const TypeInfo*, NormalizedComponentStorage::BindingData>::SemistaticMap(
  typename std::vector<std::pair<const TypeInfo*, NormalizedComponentStorage::BindingData>>::const_iterator first,
  typename std::vector<std::pair<const TypeInfo*, NormalizedComponentStorage::BindingData>>::const_iterator last);
