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

#ifndef SEMISTATIC_MAP_H
#define SEMISTATIC_MAP_H

#include <vector>
#include <limits>
#include <climits>
#include <cstdint>

namespace fruit {
namespace impl {

/**
 * Provides a subset of the interface of std::map, and also has these additional assumptions:
 * - Key must be default constructible
 * - Value must be default constructible
 * 
 * Also, while insertion of elements after construction is supported, inserting more than O(1) elements
 * after construction will raise the cost of any further lookups to more than O(1).
 */
template <typename Key, typename Value>
class SemistaticMap {
private:
  using Unsigned = std::uintptr_t;
  using NumBits = unsigned char;
  
  static const unsigned char beta = 4;
    
  static_assert(std::numeric_limits<NumBits>::max() >= sizeof(Unsigned)*CHAR_BIT,
                "An unsigned char is not enough to contain the number of bits in your platform. Please report this issue.");
    
  struct HashFunction {
    Unsigned a;
    NumBits shift; // shift==(sizeof(Unsigned)*CHAR_BIT - num_bits)
    
    inline Unsigned hash(Unsigned x) const {
      return (Unsigned)(a * x) >> shift;
    }
  };
  
  static NumBits pickNumBits(std::size_t n) {
    NumBits result = 1;
    while ((1 << result) < n) {
      ++result;
    }
    return result;
  }

  HashFunction hash_function;
  // Given a key x, the candidate places for x are keys[lookup_table[hash_function.hash(x)]] and the following cells that hash to the same value.
  std::vector<Unsigned> lookup_table;
  std::vector<Key> keys;
  // A vector parallel to `keys', in particular it has the same size.
  std::vector<Value> values;
  
  inline Unsigned hash(const Key& key) const {
    return hash_function.hash(std::hash<typename std::remove_cv<Key>::type>()(key));
  }
  
public:
  SemistaticMap() = default;
  
  // This constructor is *not* defined in semistatic_map.templates.h, but only in semistatic_map.cc.
  // All instantiations must provide an extern template declaration and have a matching instantiation in semistatic_map.cc.
  SemistaticMap(typename std::vector<std::pair<Key, Value>>::const_iterator first,
                typename std::vector<std::pair<Key, Value>>::const_iterator last);
  
  SemistaticMap(const SemistaticMap&) = default;
  SemistaticMap(SemistaticMap&&) = default;
  
  SemistaticMap& operator=(const SemistaticMap&) = default;
  SemistaticMap& operator=(SemistaticMap&&) = default;
  
  // Precondition: `key' must exist in the map.
  // Unlike std::map::at(), this yields undefined behavior if the precondition isn't satisfied (instead of throwing).
  Value& at(Key key);
  
  // Prefer using at() when possible, this is slightly slower.
  // Returns nullptr if the key was not found.
  Value* find(Key key);
  
  std::size_t count(Key key);
  
  // Inserts (key, value). If `key' already exists, inserts (key, combine(oldValue, (*this)[key])) instead.
  template <typename Combine>
  void insert(Key key, Value value, Combine combine);
};

} // namespace impl
} // namespace fruit

#include "semistatic_map.templates.h"

#endif // SEMISTATIC_MAP_H
