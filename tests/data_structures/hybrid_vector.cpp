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

#include <fruit/impl/data_structures/hybrid_vector.h>

#include <vector>
#include <set>
#include <cassert>

using namespace std;
using namespace fruit::impl;

using HV = HybridVector<int, 2>;
using V = vector<int>;

void assertSet(HV&& hv, std::initializer_list<int> values) {
  V v(std::move(hv));
  std::set<int> s;
  for (int x : v) {
    assert(s.count(x) == 0);
    s.insert(x);
  }
  assert(s == std::set<int>(values));
}

void test_empty() {
  assertSet(HV(), {});
}

void test_less_than_treshold() {
  HV hv1;
  hv1.push_back(3);
  HV hv2;
  hv2.push_back(7);
  hv2.insert(std::move(hv1));
  assertSet(std::move(hv2), {3, 7});
}

void test_treshold_exceeded() {
  HV hv1;
  hv1.push_back(3);
  hv1.push_back(7);
  hv1.push_back(8);
  assertSet(std::move(hv1), {3, 7, 8});
}

void test_insert_while_exceeding() {
  HV hv1;
  hv1.push_back(3);
  hv1.push_back(7);
  hv1.push_back(8);
  HV hv2;
  hv2.push_back(9);
  hv1.insert(std::move(hv2));
  assertSet(std::move(hv1), {3, 7, 8, 9});
}

void test_insert_exceeding() {
  HV hv1;
  hv1.push_back(3);
  hv1.push_back(7);
  hv1.push_back(8);
  HV hv2;
  hv2.push_back(9);
  hv2.insert(std::move(hv1));
  assertSet(std::move(hv2), {3, 7, 8, 9});
}

int main() {
  
  test_empty();
  test_less_than_treshold();
  test_treshold_exceeded();
  test_insert_while_exceeding();
  test_insert_exceeding();
  
  return 0;
}
