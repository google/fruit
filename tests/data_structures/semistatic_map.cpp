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

#include <fruit/impl/data_structures/semistatic_map.h>
#include <fruit/impl/data_structures/semistatic_map.templates.h>

#include <vector>

using namespace std;
using namespace fruit::impl;

void test_empty() {
  vector<pair<int, std::string>> values = {};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  assert(map.find(0) == nullptr);
  assert(map.find(2) == nullptr);
  assert(map.find(5) == nullptr);
}

void test_1_elem() {
  vector<pair<int, std::string>> values = {{2, "foo"}};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  assert(map.find(0) == nullptr);
  assert(map.find(2) != nullptr);
  assert(map.at(2) == "foo");
  assert(map.find(5) == nullptr);
}

void test_1_inserted_elem() {
  vector<pair<int, std::string>> values = {};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  map.insert(2, "bar", [](std::string x, std::string){ return x; });
  assert(map.find(0) == nullptr);
  assert(map.find(2) != nullptr);
  assert(map.at(2) == "bar");
  assert(map.find(5) == nullptr);
}

void test_1_elem_combine() {
  vector<pair<int, std::string>> values = {{2, "foo"}};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  map.insert(2, "bar", [](std::string x, std::string y){ return x + y; });
  assert(map.find(0) == nullptr);
  assert(map.find(2) != nullptr);
  assert(map.at(2) == "foobar");
  assert(map.find(5) == nullptr);
}

void test_3_elem() {
  vector<pair<int, std::string>> values = {{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  assert(map.find(0) == nullptr);
  assert(map.find(1) != nullptr);
  assert(map.at(1) == "foo");
  assert(map.find(2) == nullptr);
  assert(map.find(3) != nullptr);
  assert(map.at(3) == "bar");
  assert(map.find(4) != nullptr);
  assert(map.at(4) == "baz");
  assert(map.find(5) == nullptr);
}

void test_1_elem_2_inserted() {
  vector<pair<int, std::string>> values = {{1, "foo"}};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  auto combine = [](std::string x, std::string y){ return x + y; };
  map.insert(3, "bar", combine);
  map.insert(4, "baz", combine);
  assert(map.find(0) == nullptr);
  assert(map.find(1) != nullptr);
  assert(map.at(1) == "foo");
  assert(map.find(2) == nullptr);
  assert(map.find(3) != nullptr);
  assert(map.at(3) == "bar");
  assert(map.find(4) != nullptr);
  assert(map.at(4) == "baz");
  assert(map.find(5) == nullptr);
}

void test_3_elem_3_inserted() {
  vector<pair<int, std::string>> values = {{1, "1"}, {3, "3"}, {5, "5"}};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  auto combine = [](std::string x, std::string y){ return x + y; };
  map.insert(2, "2", combine);
  map.insert(4, "4", combine);
  map.insert(16, "16", combine);
  assert(map.find(0) == nullptr);
  assert(map.find(1) != nullptr);
  assert(map.at(1) == "1");
  assert(map.find(2) != nullptr);
  assert(map.at(2) == "2");
  assert(map.find(3) != nullptr);
  assert(map.at(3) == "3");
  assert(map.find(4) != nullptr);
  assert(map.at(4) == "4");
  assert(map.find(5) != nullptr);
  assert(map.at(5) == "5");
  assert(map.find(6) == nullptr);
  assert(map.find(16) != nullptr);
  assert(map.at(16) == "16");
}

void test_move_constructor() {
  vector<pair<int, std::string>> values = {{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map1(values.begin(), values.size());
  SemistaticMap<int, std::string> map = std::move(map1);
  assert(map.find(0) == nullptr);
  assert(map.find(1) != nullptr);
  assert(map.at(1) == "foo");
  assert(map.find(2) == nullptr);
  assert(map.find(3) != nullptr);
  assert(map.at(3) == "bar");
  assert(map.find(4) != nullptr);
  assert(map.at(4) == "baz");
  assert(map.find(5) == nullptr);
}

void test_copy_constructor() {
  vector<pair<int, std::string>> values = {{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map1(values.begin(), values.size());
  SemistaticMap<int, std::string> map = map1;
  assert(map.find(0) == nullptr);
  assert(map.find(1) != nullptr);
  assert(map.at(1) == "foo");
  assert(map.find(2) == nullptr);
  assert(map.find(3) != nullptr);
  assert(map.at(3) == "bar");
  assert(map.find(4) != nullptr);
  assert(map.at(4) == "baz");
  assert(map.find(5) == nullptr);
}

void test_move_assignment() {
  vector<pair<int, std::string>> values = {{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map1(values.begin(), values.size());
  SemistaticMap<int, std::string> map;
  map = std::move(map1);
  assert(map.find(0) == nullptr);
  assert(map.find(1) != nullptr);
  assert(map.at(1) == "foo");
  assert(map.find(2) == nullptr);
  assert(map.find(3) != nullptr);
  assert(map.at(3) == "bar");
  assert(map.find(4) != nullptr);
  assert(map.at(4) == "baz");
  assert(map.find(5) == nullptr);
}

void test_copy_assignment() {
  vector<pair<int, std::string>> values = {{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map1(values.begin(), values.size());
  SemistaticMap<int, std::string> map;
  map = map1;
  assert(map.find(0) == nullptr);
  assert(map.find(1) != nullptr);
  assert(map.at(1) == "foo");
  assert(map.find(2) == nullptr);
  assert(map.find(3) != nullptr);
  assert(map.at(3) == "bar");
  assert(map.find(4) != nullptr);
  assert(map.at(4) == "baz");
  assert(map.find(5) == nullptr);
}

int main() {
  
  test_empty();
  test_1_elem();
  test_1_inserted_elem();
  test_1_elem_combine();
  test_3_elem();
  test_1_elem_2_inserted();
  test_3_elem_3_inserted();
  test_move_constructor();
  test_copy_constructor();
  test_move_assignment();
  test_copy_assignment();
  
  return 0;
}
