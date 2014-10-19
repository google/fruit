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
#include <chrono>

using namespace fruit::impl;

SemistaticMap::SemistaticMap(const std::vector<BindingData>& values1) {
  std::size_t n = values1.size();
  NumBits num_bits = pickNumBits(n);
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
    
    for (const BindingData& value : values1) {
      Unsigned& thisCount = count[hash(value.getKey())];
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
  
  for (const BindingData& value : values1) {
    Unsigned& cell = lookup_table[hash(value.getKey())];
    --cell;
    assert(cell < n);
    values[cell] = value;
  }
}

BindingData& SemistaticMap::at(Key key) {
  Unsigned raw_key = reinterpret_cast<Unsigned>(key) | 1;
  Unsigned h = hash(key);
  Unsigned i = lookup_table[h];
  while (true) {
    assert(i < values.size());
    if ((values[i].getRawKey() | 1) == raw_key) {
      return values[i];
    }
    assert(hash(values[i].getKey()) == h);
    ++i;
  }
}

BindingData* SemistaticMap::find(Key key) {
  Unsigned raw_key = reinterpret_cast<Unsigned>(key) | 1;
  Unsigned h = hash(key);
  Unsigned first_candidate_index = lookup_table[h];
  Unsigned last_candidate_index = values.size();
  for (Unsigned i = first_candidate_index; i != last_candidate_index; ++i) {
    if ((values[i].getRawKey() | 1) == raw_key) {
      return &(values[i]);
    }
    Unsigned h1 = hash(values[i].getKey());
    if (h1 != h) {
      break;
    }
  }
  return nullptr;
}
