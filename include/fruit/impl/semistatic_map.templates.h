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

#include "semistatic_map.h"

namespace fruit {
namespace impl {

template <typename Combine>
void SemistaticMap::insert(BindingData value, Combine combine) {
  Key key = value.getKey();
  Unsigned raw_key = reinterpret_cast<Unsigned>(key) | 1;
  Unsigned h = hash(key);
  Unsigned old_keys_size = values.size();
  Unsigned first_candidate_index = lookup_table[h];
  Unsigned last_candidate_index = old_keys_size;
  
  {
    Unsigned i = first_candidate_index;
    for (; i != last_candidate_index; ++i) {
      if ((values[i].getRawKey() | 1) == raw_key) {
        values[i] = combine(value, values[i]);
        return;
      }
      Unsigned h1 = hash(values[i].getKey());
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
    // The copy makes sure that the references passed to push_back dont't get invalidated by resizing.
    values.push_back(BindingData(values[i]));
  }
  
  // Step 2: also insert the new value.
  values.push_back(value);
  
  // Step 3: update the index in the lookup table to point to the newly-added sequence.
  // The old sequence is no longer pointed to by any index in the lookup table, but recompacting the vectors would be too slow.
  lookup_table[h] = old_keys_size;
}

} // namespace impl
} // namespace fruit

#endif // SEMISTATIC_MAP_TEMPLATES_H
