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
#include "../test_macros.h"

#include <vector>

using namespace std;
using namespace fruit::impl;

void test_empty() {
  vector<pair<int, std::string>> values{};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  Assert(map.find(0) == nullptr);
  Assert(map.find(2) == nullptr);
  Assert(map.find(5) == nullptr);
}

void test_1_elem() {
  vector<pair<int, std::string>> values{{2, "foo"}};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  Assert(map.find(0) == nullptr);
  Assert(map.find(2) != nullptr);
  Assert(map.at(2) == "foo");
  Assert(map.find(5) == nullptr);
}

void test_1_inserted_elem() {
  vector<pair<int, std::string>> values{};
  SemistaticMap<int, std::string> old_map(values.begin(), values.size());
  vector<pair<int, std::string>> new_values{{2, "bar"}};
  SemistaticMap<int, std::string> map(old_map, std::move(new_values));
  Assert(map.find(0) == nullptr);
  Assert(map.find(2) != nullptr);
  Assert(map.at(2) == "bar");
  Assert(map.find(5) == nullptr);
}

void test_3_elem() {
  vector<pair<int, std::string>> values{{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map(values.begin(), values.size());
  Assert(map.find(0) == nullptr);
  Assert(map.find(1) != nullptr);
  Assert(map.at(1) == "foo");
  Assert(map.find(2) == nullptr);
  Assert(map.find(3) != nullptr);
  Assert(map.at(3) == "bar");
  Assert(map.find(4) != nullptr);
  Assert(map.at(4) == "baz");
  Assert(map.find(5) == nullptr);
}

void test_1_elem_2_inserted() {
  vector<pair<int, std::string>> values{{1, "foo"}};
  SemistaticMap<int, std::string> old_map(values.begin(), values.size());
  vector<pair<int, std::string>> new_values{{3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map(old_map, std::move(new_values));
  Assert(map.find(0) == nullptr);
  Assert(map.find(1) != nullptr);
  Assert(map.at(1) == "foo");
  Assert(map.find(2) == nullptr);
  Assert(map.find(3) != nullptr);
  Assert(map.at(3) == "bar");
  Assert(map.find(4) != nullptr);
  Assert(map.at(4) == "baz");
  Assert(map.find(5) == nullptr);
}

void test_3_elem_3_inserted() {
  vector<pair<int, std::string>> values{{1, "1"}, {3, "3"}, {5, "5"}};
  SemistaticMap<int, std::string> old_map(values.begin(), values.size());
  vector<pair<int, std::string>> new_values{{2, "2"}, {4, "4"}, {16, "16"}};
  SemistaticMap<int, std::string> map(old_map, std::move(new_values));
  Assert(map.find(0) == nullptr);
  Assert(map.find(1) != nullptr);
  Assert(map.at(1) == "1");
  Assert(map.find(2) != nullptr);
  Assert(map.at(2) == "2");
  Assert(map.find(3) != nullptr);
  Assert(map.at(3) == "3");
  Assert(map.find(4) != nullptr);
  Assert(map.at(4) == "4");
  Assert(map.find(5) != nullptr);
  Assert(map.at(5) == "5");
  Assert(map.find(6) == nullptr);
  Assert(map.find(16) != nullptr);
  Assert(map.at(16) == "16");
}

void test_move_constructor() {
  vector<pair<int, std::string>> values{{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map1(values.begin(), values.size());
  SemistaticMap<int, std::string> map = std::move(map1);
  Assert(map.find(0) == nullptr);
  Assert(map.find(1) != nullptr);
  Assert(map.at(1) == "foo");
  Assert(map.find(2) == nullptr);
  Assert(map.find(3) != nullptr);
  Assert(map.at(3) == "bar");
  Assert(map.find(4) != nullptr);
  Assert(map.at(4) == "baz");
  Assert(map.find(5) == nullptr);
}

void test_move_assignment() {
  vector<pair<int, std::string>> values{{1, "foo"}, {3, "bar"}, {4, "baz"}};
  SemistaticMap<int, std::string> map1(values.begin(), values.size());
  SemistaticMap<int, std::string> map;
  map = std::move(map1);
  Assert(map.find(0) == nullptr);
  Assert(map.find(1) != nullptr);
  Assert(map.at(1) == "foo");
  Assert(map.find(2) == nullptr);
  Assert(map.find(3) != nullptr);
  Assert(map.at(3) == "bar");
  Assert(map.find(4) != nullptr);
  Assert(map.at(4) == "baz");
  Assert(map.find(5) == nullptr);
}

int main() {
  
  test_empty();
  test_1_elem();
  test_1_inserted_elem();
  test_3_elem();
  test_1_elem_2_inserted();
  test_3_elem_3_inserted();
  test_move_constructor();
  test_move_assignment();
  
  return 0;
}
