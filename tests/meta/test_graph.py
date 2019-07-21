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
    #include <fruit/impl/meta/graph.h>
    
    struct A1 {};
    struct B1 {};
    struct C1 {};
    struct D1 {};
    struct E1 {};
    
    using A = Type<A1>;
    using B = Type<B1>;
    using C = Type<C1>;
    using D = Type<D1>;
    using E = Type<E1>;
    '''

class TestGraph(parameterized.TestCase):
    def test_GraphFindLoop(self):
        source = '''
            int main() {
              // A -> B, D
              // C -> D
              AssertSameType(Id<GraphFindLoop(Vector<Pair<A, Vector<B, D>>, Pair<C, Vector<D>>, Pair<B, Vector<>>>)>, None);
            
              // A -> B
              // B -> B
              // C -> B
              AssertSameType(Id<GraphFindLoop(Vector<Pair<A, Vector<B>>, Pair<B, Vector<B>>, Pair<C, Vector<B>>>)>, Vector<B>);
            
              // A -> D, B
              // B -> C
              // C -> A
              // The order in the result here *does* matter, but rotations of the correct (A,B,C) sequence are also ok.
              // Fix this test as appropriate.
              AssertSameType(Id<GraphFindLoop(Vector<Pair<A, Vector<D, B>>, Pair<B, Vector<C>>, Pair<C, Vector<A>>>)>, Vector<B, C, A>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
