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

#ifndef FRUIT_PACKED_POINTER_AND_BOOL_H
#define FRUIT_PACKED_POINTER_AND_BOOL_H

namespace fruit {
namespace impl {

/**
 * This class stores a T* and a bool in the space of a pointer.
 * alignof(T) must be at least 2.
 */
template <typename T>
class PackedPointerAndBool {
private:
  static_assert(alignof(T) >= 2, "alignof(T) must be at least 2 for the packing to be possible");

  std::uintptr_t value;

  static std::uintptr_t encode(T* p, bool b);
  static T* decodePointer(std::uintptr_t value);
  static bool decodeBool(std::uintptr_t value);

public:
  PackedPointerAndBool(T* p, bool b);

  PackedPointerAndBool() = default;

  PackedPointerAndBool(const PackedPointerAndBool&) = default;
  PackedPointerAndBool(PackedPointerAndBool&&) = default;

  PackedPointerAndBool& operator=(const PackedPointerAndBool&) = default;
  PackedPointerAndBool& operator=(PackedPointerAndBool&&) = default;

  T* getPointer() const;
  void setPointer(T* p);

  bool getBool() const;
  void setBool(bool b);

  bool operator==(const PackedPointerAndBool<T>& other) const;
  bool operator!=(const PackedPointerAndBool<T>& other) const;
};

} // namespace impl
} // namespace fruit

#include <fruit/impl/data_structures/packed_pointer_and_bool.defn.h>

#endif // FRUIT_PACKED_POINTER_AND_BOOL_H
