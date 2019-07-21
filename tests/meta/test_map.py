#!/usr/bin/env python3
#  Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from absl.testing import parameterized
from fruit_test_common import *

COMMON_DEFINITIONS = '''
    #define IN_FRUIT_CPP_FILE 1
    
    #include "meta/common.h"
    #include <fruit/impl/meta/map.h>
    #include <fruit/impl/meta/metaprogramming.h>
    '''

class TestMap(parameterized.TestCase):
    def test_FindInMap(self):
        source = '''
            int main() {
              AssertSameType(Id<FindInMap(ToSet<>, Int<2>)>, None);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<1>, Int<2>>>, Int<7>)>, None);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<1>, Int<2>>>, Int<2>)>, None);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<2>, Int<1>>>, Int<2>)>, Int<1>);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<7>)>, None);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<2>)>, None);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<20>)>, None);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<1>)>, Int<2>);
              AssertSameType(Id<FindInMap(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>, Int<10>)>, Int<20>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_MapContainsKey(self):
        source = '''
            int main() {
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
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_GetMapKeys(self):
        source = '''
            int main() {
              AssertSameSet(Id<GetMapKeys(ToSet<>)>, ToSet<>);
              AssertSameSet(Id<GetMapKeys(ToSet<Pair<Int<1>, Int<2>>>)>, ToSet<Int<1>>);
              AssertSameSet(Id<GetMapKeys(ToSet<Pair<Int<1>, Int<2>>, Pair<Int<10>, Int<20>>>)>, ToSet<Int<1>, Int<10>>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
