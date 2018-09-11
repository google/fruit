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

#if !IN_FRUIT_CPP_FILE
#error "Fruit .template.h file included in non-cpp file."
#endif

#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
#include <utility>
// This include is not necessary for GCC/Clang, but it's necessary for MSVC.
#include <numeric>

#include <fruit/impl/data_structures/semistatic_map.h>

#include <fruit/impl/data_structures/arena_allocator.h>
#include <fruit/impl/data_structures/fixed_size_vector.templates.h>
#include <fruit/impl/fruit_assert.h>

namespace fruit {
namespace impl {

template <typename Key, typename Value>
template <typename Iter>
SemistaticMap<Key, Value>::SemistaticMap(
    Iter values_begin, Iter values_end, std::size_t num_values, MemoryPool& memory_pool) {
  NumBits num_bits = pickNumBits(num_values);
  std::size_t num_buckets = size_t(1) << num_bits;

  FixedSizeVector<Unsigned, ArenaAllocator<Unsigned>> count(num_buckets, 0, ArenaAllocator<Unsigned>(memory_pool));

  hash_function.shift = (sizeof(Unsigned) * CHAR_BIT - num_bits);

  // The cast is a no-op in some systems (e.g. GCC and Clang under Linux 64bit) but it's needed in other systems (e.g.
  // MSVC).
  unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine random_generator(seed);
  std::uniform_int_distribution<Unsigned> random_distribution;

  while (1) {
    hash_function.a = random_distribution(random_generator);

    for (Iter itr = values_begin; !(itr == values_end); ++itr) {
      Unsigned& this_count = count[hash((*itr).first)];
      ++this_count;
      if (this_count == beta) {
        goto pick_another;
      }
    }
    break;

  pick_another:
    std::memset(count.data(), 0, num_buckets * sizeof(Unsigned));
  }

  values = FixedSizeVector<value_type>(num_values, value_type());

  std::partial_sum(count.begin(), count.end(), count.begin());
  lookup_table = FixedSizeVector<CandidateValuesRange>(count.size());
  for (Unsigned n : count) {
    lookup_table.push_back(CandidateValuesRange{values.data() + n, values.data() + n});
  }

  // At this point lookup_table[h] is the number of keys in [first, last) that have a hash <=h.
  // Note that even though we ensure this after construction, it is not maintained by insert() so it's not an invariant.

  Iter itr = values_begin;
  for (std::size_t i = 0; i < num_values; ++i, ++itr) {
    value_type*& first_value_ptr = lookup_table[hash((*itr).first)].begin;
    --first_value_ptr;
    FruitAssert(values.data() <= first_value_ptr);
    FruitAssert(first_value_ptr < values.data() + values.size());
    *first_value_ptr = *itr;
  }
}

template <typename Key, typename Value>
SemistaticMap<Key, Value>::SemistaticMap(const SemistaticMap<Key, Value>& map,
                                         std::vector<value_type, ArenaAllocator<value_type>>&& new_elements)
    : hash_function(map.hash_function), lookup_table(map.lookup_table, map.lookup_table.size()) {

  // Sort by hash.
  std::sort(new_elements.begin(), new_elements.end(),
            [this](const value_type& x, const value_type& y) { return hash(x.first) < hash(y.first); });

  std::size_t num_additional_values = new_elements.size();
  // Add the space needed to store copies of the old buckets.
  for (auto itr = new_elements.begin(), itr_end = new_elements.end(); itr != itr_end; /* no increment */) {
    Unsigned h = hash(itr->first);
    auto p = map.lookup_table[h];
    num_additional_values += (p.end - p.begin);
    for (; itr != itr_end && hash(itr->first) == h; ++itr) {
    }
  }

  values = FixedSizeVector<value_type>(num_additional_values);

  // Now actually perform the insertions.

  if (new_elements.empty()) {
    // This is to workaround a bug in the STL shipped with GCC <4.8.2, where calling data() on an
    // empty vector causes undefined behavior (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59829).
    return;
  }
  for (value_type *itr = new_elements.data(), *itr_end = new_elements.data() + new_elements.size(); itr != itr_end;
       /* no increment */) {
    Unsigned h = hash(itr->first);
    auto p = map.lookup_table[h];
    num_additional_values += (p.end - p.begin);
    value_type* first = itr;
    for (; itr != itr_end && hash(itr->first) == h; ++itr) {
    }
    value_type* last = itr;
    insert(h, first, last);
  }
}

template <typename Key, typename Value>
void SemistaticMap<Key, Value>::insert(std::size_t h, const value_type* elems_begin, const value_type* elems_end) {

  value_type* old_bucket_begin = lookup_table[h].begin;
  value_type* old_bucket_end = lookup_table[h].end;

  lookup_table[h].begin = values.data() + values.size();

  // Step 1: re-insert all keys with the same hash at the end (if any).
  for (value_type* p = old_bucket_begin; p != old_bucket_end; ++p) {
    values.push_back(*p);
  }

  // Step 2: also insert the new keys and values
  for (auto itr = elems_begin; itr != elems_end; ++itr) {
    values.push_back(*itr);
  }

  lookup_table[h].end = values.data() + values.size();

  // The old sequence is no longer pointed to by any index in the lookup table, but recompacting the vectors would be
  // too slow.
}

template <typename Key, typename Value>
const Value& SemistaticMap<Key, Value>::at(Key key) const {
  Unsigned h = hash(key);
  for (const value_type* p = lookup_table[h].begin; /* p!=lookup_table[h].end but no need to check */; ++p) {
    FruitAssert(p != lookup_table[h].end);
    if (p->first == key) {
      return p->second;
    }
  }
}

template <typename Key, typename Value>
const Value* SemistaticMap<Key, Value>::find(Key key) const {
  Unsigned h = hash(key);
  for (const value_type *p = lookup_table[h].begin, *p_end = lookup_table[h].end; p != p_end; ++p) {
    if (p->first == key) {
      return &(p->second);
    }
  }
  return nullptr;
}

template <typename Key, typename Value>
typename SemistaticMap<Key, Value>::NumBits SemistaticMap<Key, Value>::pickNumBits(std::size_t n) {
  NumBits result = 1;
  while ((std::size_t(1) << result) < n) {
    ++result;
  }
  return result + 1;
}

// This is here so that we don't have to include fixed_size_vector.templates.h in fruit.h.
template <typename Key, typename Value>
SemistaticMap<Key, Value>::~SemistaticMap() {}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_MAP_TEMPLATES_H
