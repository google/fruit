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

#include "checked_adder.h"

#include <climits>
#include <iostream>

class CheckedAdder : public Adder {
private:
  bool add_overflows(int x, int y) {
    if (y > x)
      std::swap(x, y);
    // Now y <= x.
    const int half_max = INT_MAX / 2;
    const int half_min = INT_MIN / 2;
    if (x > half_max) {
      // We can't have negative overflow, but might have positive overflow.
      if (y > half_max)
        return true;
      if (y <= 0)
        return false;
      // x <= INT_MAX && y <= half_max,
      // so: x + y <= INT_MAX + half_max
      // so: x - half_max + y <= INT_MAX
      // so: (x - half_max + y) doesn't overflow.
      // (x + y) > INT_MAX iff (x - half_max + y) > (INT_MAX - half_max)
      return (x - half_max + y) > (INT_MAX - half_max);
    }
    // y <= x <= half_max, can't have positive overflow.
    if (y < half_min) {
      // We can't have positive overflow, but might have negative overflow.
      if (x < half_min)
        return true;
      if (x >= 0)
        return false;
      // y >= INT_MIN && x >= half_min,
      // so: y + x >= INT_MIN + half_min
      // so: y - half_min + x >= INT_MAX
      // so: (y - half_min + x) doesn't overflow.
      // (y + x) < INT_MIN iff (y - half_min + x) < (INT_MIN - half_min)
      return (y - half_min + x) < (INT_MIN - half_min);
    }
    // Neither negative nor positive overflow.
    return false;
  }

public:
  INJECT(CheckedAdder()) = default;

  virtual int add(int x, int y) override {
    if (add_overflows(x, y)) {
      std::cerr << "CheckedAdder: detected overflow during addition of " << x << " and " << y << std::endl;
      abort();
    }
    return x + y;
  }
};

fruit::Component<Adder> getCheckedAdderComponent() {
  return fruit::createComponent().bind<Adder, CheckedAdder>();
}
