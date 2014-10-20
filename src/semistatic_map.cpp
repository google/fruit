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
#include "fruit/impl/static_graph.h"
#include "fruit/impl/binding_data.h"

#include <algorithm>
#include <random>
#include <utility>
#include <chrono>
#include <cassert>

using namespace fruit::impl;

template <typename Key, typename Value>
Value& SemistaticMap<Key, Value>::at(Key key) {
  Unsigned h = hash(key);
  Unsigned i = lookup_table[h];
  while (true) {
    assert(i < values.size());
    if (values[i].first == key) {
      return values[i].second;
    }
    assert(hash(values[i].first) == h);
    ++i;
  }
}

template <typename Key, typename Value>
Value* SemistaticMap<Key, Value>::find(Key key) {
  Unsigned h = hash(key);
  Unsigned first_candidate_index = lookup_table[h];
  Unsigned last_candidate_index = values.size();
  for (Unsigned i = first_candidate_index; i != last_candidate_index; ++i) {
    if (values[i].first == key) {
      return &(values[i].second);
    }
    Unsigned h1 = hash(values[i].first);
    if (h1 != h) {
      break;
    }
  }
  return nullptr;
}

template class SemistaticMap<TypeId, std::size_t>;
template class SemistaticMap<TypeId, std::vector<StaticGraph<TypeId, NormalizedBindingData>::NodeData>::iterator>;
