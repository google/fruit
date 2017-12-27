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

#ifndef FRUIT_CLASS_CONSTRUCTION_TRACKER_H
#define FRUIT_CLASS_CONSTRUCTION_TRACKER_H

#include <cstddef>

/**
 * This class is useful to keep track of how many instances of a given type are created during the entire program
 * execution.
 *
 * Example use:
 *   class Foo : public ConstructionTracker<Foo> {
 *      ...
 *   };
 *
 *   int main() {
 *     ...
 *     assert(Foo::num_objects_constructed == 3);
 *   }
 */
template <typename T>
struct ConstructionTracker {
  static std::size_t num_objects_constructed;

  ConstructionTracker() {
    ++num_objects_constructed;
  }
};

template <typename T>
std::size_t ConstructionTracker<T>::num_objects_constructed = 0;

#endif // FRUIT_CLASS_CONSTRUCTION_TRACKER_H
