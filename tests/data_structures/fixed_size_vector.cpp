// expect-success
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

#define IN_FRUIT_CPP_FILE

#include <fruit/impl/data_structures/fixed_size_vector.h>

#include <vector>

using namespace std;
using namespace fruit::impl;

void test_empty_capacity_0() {
  FixedSizeVector<int> v;
  const FixedSizeVector<int>& const_v = v;
  assert(v.size() == 0);
  assert(v.data() == nullptr);
  assert(v.begin() == v.end());
  assert(const_v.data() == nullptr);
  assert(const_v.begin() == const_v.end());
}

void test_empty_capacity_nonzero() {
  FixedSizeVector<int> v(15);
  const FixedSizeVector<int>& const_v = v;
  assert(v.size() == 0);
  assert(v.data() != nullptr);
  assert(v.begin() == v.end());
  assert(const_v.data() != nullptr);
  assert(const_v.begin() == const_v.end());
}

void test_push_back() {
  FixedSizeVector<int> v(2);
  v.push_back(1000);
  v.push_back(2000);
  const FixedSizeVector<int>& const_v = v;
  assert(v.size() == 2);
  assert(v.data() == &(v[0]));
  assert(v.end() - v.begin() == 2);
  assert(&*(v.begin()) == &(v[0]));
  assert(&*(v.begin() + 1) == &(v[1]));
  assert(const_v.data() != nullptr);
  assert(const_v.begin() == &(const_v[0]));
  assert(const_v.end() - const_v.begin() == 2);
  assert(&*(const_v.begin()) == &(const_v[0]));
  assert(&*(const_v.begin() + 1) == &(const_v[1]));
  assert(v[0] == 1000);
  assert(v[1] == 2000);
}

void test_2arg_constructor() {
  FixedSizeVector<int> v(2, 1000);
  const FixedSizeVector<int>& const_v = v;
  assert(v.size() == 2);
  assert(v.data() == &(v[0]));
  assert(v.end() - v.begin() == 2);
  assert(&*(v.begin()) == &(v[0]));
  assert(&*(v.begin() + 1) == &(v[1]));
  assert(const_v.data() != nullptr);
  assert(const_v.begin() == &(const_v[0]));
  assert(const_v.end() - const_v.begin() == 2);
  assert(&*(const_v.begin()) == &(const_v[0]));
  assert(&*(const_v.begin() + 1) == &(const_v[1]));
  assert(v[0] == 1000);
  assert(v[1] == 1000);
}

void test_move_constructor() {
  FixedSizeVector<int> v1(2);
  v1.push_back(1000);
  v1.push_back(2000);
  FixedSizeVector<int> v = std::move(v1);
  assert(v.size() == 2);
  assert(v.data() == &(v[0]));
  assert(v.end() - v.begin() == 2);
  assert(&*(v.begin()) == &(v[0]));
  assert(&*(v.begin() + 1) == &(v[1]));
  assert(v[0] == 1000);
  assert(v[1] == 2000);  
}

void test_copy_constructor() {
  FixedSizeVector<int> v1(2);
  v1.push_back(1000);
  v1.push_back(2000);
  FixedSizeVector<int> v(v1, 3);
  assert(v.size() == 2);
  assert(v.data() == &(v[0]));
  assert(v.end() - v.begin() == 2);
  assert(&*(v.begin()) == &(v[0]));
  assert(&*(v.begin() + 1) == &(v[1]));
  assert(v[0] == 1000);
  assert(v[1] == 2000);  
}

void test_move_assignment() {
  FixedSizeVector<int> v1(2);
  v1.push_back(1000);
  v1.push_back(2000);
  FixedSizeVector<int> v;
  v = std::move(v1);
  assert(v.size() == 2);
  assert(v.data() == &(v[0]));
  assert(v.end() - v.begin() == 2);
  assert(&*(v.begin()) == &(v[0]));
  assert(&*(v.begin() + 1) == &(v[1]));
  assert(v[0] == 1000);
  assert(v[1] == 2000);  
}

void test_swap() {
  FixedSizeVector<int> v1(2);
  v1.push_back(1000);
  v1.push_back(2000);
  FixedSizeVector<int> v2(1);
  v2.push_back(3000);
  std::swap(v1, v2);
  assert(v1.size() == 1);
  assert(v1[0] == 3000);
  assert(v2.size() == 2);
  assert(v2[0] == 1000);
  assert(v2[1] == 2000);
}

void test_clear() {
  FixedSizeVector<int> v(2);
  v.push_back(1000);
  v.push_back(2000);
  v.clear();
  assert(v.size() == 0);
  assert(v.data() != nullptr);
  assert(v.begin() == v.end());
  // This must not blow up, clear() must preserve the capacity.
  v.push_back(1000);
}

int main() {
  test_empty_capacity_0();
  test_empty_capacity_nonzero();
  test_push_back();
  test_2arg_constructor();
  test_move_constructor();
  test_copy_constructor();
  test_move_assignment();
  test_swap();
  test_clear();
  
  return 0;
}
