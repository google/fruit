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

#include "common.h"
#include <fruit/impl/meta/map.h>
#include <fruit/impl/meta/metaprogramming.h>

void test_FindInMap() {
  AssertSameType(FindInMap(ToSet<>, Int<2>), None);
  AssertSameType(FindInMap(ToSet<Pair<Int<1>, Int<2>>>, Int<7>), None);
  AssertSameType(FindInMap(ToSet<Pair<Int<1>, Int<2>>>, Int<2>), None);
  AssertSameType(FindInMap(ToSet<Pair<Int<2>, Int<1>>>, Int<2>), Int<1>);
  AssertSameType(FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<7>), None);
  AssertSameType(FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<2>), None);
  AssertSameType(FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<20>), None);
  AssertSameType(FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<1>), Int<2>);
  AssertSameType(FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<10>), Int<20>);
}

void test_MapContainsKey() {
  AssertNot(MapContainsKey(ToSet<>, Int<2>));
  AssertNot(MapContainsKey(ToSet<Pair<Int<1>, Int<2>>>, Int<7>));
  AssertNot(MapContainsKey(ToSet<Pair<Int<1>, Int<2>>>, Int<2>));
  Assert(MapContainsKey(ToSet<Pair<Int<2>, Int<1>>>, Int<2>));
  AssertNot(MapContainsKey(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<7>));
  AssertNot(MapContainsKey(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<2>));
  AssertNot(MapContainsKey(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<20>));
  Assert(MapContainsKey(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<1>));
  Assert(MapContainsKey(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<10>));
}

void test_GetMapKeys() {
  AssertSameSet(GetMapKeys(ToSet<>), ToSet<>);
  AssertSameSet(GetMapKeys(ToSet<Pair<Int<1>, Int<2>>>), ToSet<Int<1>>);
  AssertSameSet(GetMapKeys(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>), ToSet<Int<1>, Int<10>>);
}



int main() {
  
  test_FindInMap();
  test_MapContainsKey();
  test_GetMapKeys();
  
  return 0;
}
