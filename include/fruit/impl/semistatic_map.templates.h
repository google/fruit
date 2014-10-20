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

#ifndef SEMISTATIC_MAP_TEMPLATES_H
#define SEMISTATIC_MAP_TEMPLATES_H

#include <cassert>
#include <chrono>

#include "semistatic_map.h"

namespace fruit {
namespace impl {

template <typename Key, typename Value>
template <typename Combine>
void SemistaticMap<Key, Value>::insert(Key key, Value value, Combine combine) {
  Unsigned h = hash(key);
  Unsigned old_keys_size = values.size();
  Unsigned first_candidate_index = lookup_table[h];
  Unsigned last_candidate_index = old_keys_size;
  
  {
    Unsigned i = first_candidate_index;
    for (; i != last_candidate_index; ++i) {
      if (values[i].first == key) {
        values[i].second = combine(value, values[i].second);
        return;
      }
      Unsigned h1 = hash(values[i].first);
      if (h1 != h) {
        break;
      }
    }
    last_candidate_index = i;
    // Now [first_candidate_index, last_candidate_index) contains only keys that hash to h.
  }
  
  // `key' is not in `keys'.
  
  // Step 1: re-insert all keys with the same hash at the end (if any).
  for (Unsigned i = first_candidate_index; i != last_candidate_index; ++i) {
    // The copies make sure that the references passed to push_back dont't get invalidated by resizing.
    values.emplace_back(Key(values[i].first), Value(values[i].second));
  }
  
  // Step 2: also insert the new key and value
  values.emplace_back(key, value);
  
  // Step 3: update the index in the lookup table to point to the newly-added sequence.
  // The old sequence is no longer pointed to by any index in the lookup table, but recompacting the vectors would be too slow.
  lookup_table[h] = old_keys_size;
}

template <typename Key, typename Value>
template <typename Iter>
SemistaticMap<Key, Value>::SemistaticMap(Iter valuesBegin, std::size_t num_values) {
  NumBits num_bits = pickNumBits(num_values);
  std::size_t num_buckets = (1 << num_bits);
  
  std::vector<Unsigned> count;
  count.reserve(num_buckets);
  
  hash_function.shift = (sizeof(Unsigned)*CHAR_BIT - num_bits);
  
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine random_generator(seed);
  std::uniform_int_distribution<Unsigned> random_distribution;
  
  while (1) {
    hash_function.a = random_distribution(random_generator);
    count.assign(num_buckets, 0);
    
    Iter itr = valuesBegin;
    for (std::size_t i = 0; i < num_values; ++i, ++itr) {
      Unsigned& thisCount = count[hash((*itr).first)];
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
  values.resize(num_values);
  
  // At this point lookup_table[h] is the number of keys in [first, last) that have a hash <=h.
  // Note that even though we ensure this after construction, it is not maintained by insert() so it's not an invariant.
  
  Iter itr = valuesBegin;
  for (std::size_t i = 0; i < num_values; ++i, ++itr) {
    Unsigned& cell = lookup_table[hash((*itr).first)];
    --cell;
    assert(cell < num_values);
    values[cell] = *itr;
  }
}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_MAP_TEMPLATES_H
