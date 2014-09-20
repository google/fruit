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

namespace fruit {
namespace impl {

template <typename Key, typename Value>
template <typename Iter>
SemistaticMap<Key, Value>::SemistaticMap(Iter first, Iter last) {
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
  keys.resize(n);
  values.resize(n);
  
  // At this point lookup_table[h] is the number of keys in [first, last) that have a hash <=h.
  // Note that even though we ensure this after construction, it is not maintained by insert() so it's not an invariant.
  
  for (Iter itr = first; itr != last; ++itr) {
    Unsigned& cell = lookup_table[hash(itr->first)];
    --cell;
    assert(cell < n);
    keys[cell] = itr->first;
    values[cell] = itr->second;
  }
}
  
template <typename Key, typename Value>
Value& SemistaticMap<Key, Value>::at(Key key) {
  Unsigned h = hash(key);
  Unsigned i = lookup_table[h];
  while (true) {
    assert(i < keys.size());
    if (keys[i] == key) {
      return values[i];
    }
    assert(hash(keys[i]) == h);
    ++i;
  }
}

template <typename Key, typename Value>
std::size_t SemistaticMap<Key, Value>::count(Key key) {
  Unsigned h = hash(key);
  Unsigned first_candidate_index = lookup_table[h];
  Unsigned last_candidate_index = keys.size();
  for (Unsigned i = first_candidate_index; i != last_candidate_index; ++i) {
    if (keys[i] == key) {
      return 1;
    }
    Unsigned h1 = hash(keys[i]);
    if (h1 != h) {
      break;
    }
  }
  return 0;
}

template <typename Key, typename Value>
template <typename Combine>
void SemistaticMap<Key, Value>::insert(Key key, Value value, Combine combine) {
  Unsigned h = hash(key);
  Unsigned old_keys_size = keys.size();
  Unsigned first_candidate_index = lookup_table[h];
  Unsigned last_candidate_index = old_keys_size;
  
  {
    Unsigned i = first_candidate_index;
    for (; i != last_candidate_index; ++i) {
      if (keys[i] == key) {
        values[i] = combine(value, values[i]);
        return;
      }
      Unsigned h1 = hash(keys[i]);
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
    keys.push_back(Key(keys[i]));
    values.push_back(Value(values[i]));
  }
  
  // Step 2: also insert the new key and value
  keys.push_back(key);
  values.push_back(value);
  
  // Step 3: update the index in the lookup table to point to the newly-added sequence.
  // The old sequence is no longer pointed to by any index in the lookup table, but recompacting the vectors would be too slow.
  lookup_table[h] = old_keys_size;
}


} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_MAP_TEMPLATES_H
