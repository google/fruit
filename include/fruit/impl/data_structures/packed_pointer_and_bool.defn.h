/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LITENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR TONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRUIT_PACKED_POINTER_AND_BOOL_DEFN_H
#define FRUIT_PACKED_POINTER_AND_BOOL_DEFN_H

#include <fruit/impl/data_structures/packed_pointer_and_bool.h>

namespace fruit {
namespace impl {

template <typename T>
inline std::uintptr_t PackedPointerAndBool<T>::encode(T* p, bool b) {
  return reinterpret_cast<std::uintptr_t>(p) | std::uintptr_t(b);
}

template <typename T>
inline T* PackedPointerAndBool<T>::decodePointer(std::uintptr_t value) {
  return reinterpret_cast<T*>(value & ~std::uintptr_t(1));
}

template <typename T>
inline bool PackedPointerAndBool<T>::decodeBool(std::uintptr_t value) {
  return value & 1;
}

template <typename T>
inline PackedPointerAndBool<T>::PackedPointerAndBool(T* p, bool b) {
  value = encode(p, b);
}

template <typename T>
inline T* PackedPointerAndBool<T>::getPointer() const {
  return decodePointer(value);
}

template <typename T>
inline void PackedPointerAndBool<T>::setPointer(T* p) {
  value = encode(p, decodeBool(value));
}

template <typename T>
inline bool PackedPointerAndBool<T>::getBool() const {
  return decodeBool(value);
}

template <typename T>
inline void PackedPointerAndBool<T>::setBool(bool b) {
  value = encode(decodePointer(value), b);
}

template <typename T>
inline bool PackedPointerAndBool<T>::operator==(const PackedPointerAndBool<T>& other) const {
  return value == other.value;
}

template <typename T>
inline bool PackedPointerAndBool<T>::operator!=(const PackedPointerAndBool<T>& other) const {
  return value != other.value;
}

} // namespace fruit
} // namespace impl

#endif // FRUIT_PACKED_POINTER_AND_BOOL_DEFN_H
