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
    #include <fruit/impl/meta/metaprogramming.h>
    #include <fruit/impl/meta/proof_trees.h>
    #include <fruit/impl/meta/proof_tree_comparison.h>
    
    #include <vector>
    
    struct A1 {};
    struct B1 {};
    struct C1 {};
    struct D1 {};
    struct X1 {};
    struct Y1 {};
    
    using A = Type<A1>;
    using B = Type<B1>;
    using C = Type<C1>;
    using D = Type<D1>;
    using X = Type<X1>;
    using Y = Type<Y1>;
    
    using Proof1 = Pair<X, ToSet<A, B>>;
    using Proof1b = Pair<X, ToSet<B, A>>;
    using Proof2 = Pair<Y, ToSet<B, C>>;
    '''

class TestProofTrees(parameterized.TestCase):
    def test_IsProofTreeEqualTo(self):
        source = '''
            int main() {
                AssertNotSameProof(Pair<X, ToSet<A>>, Pair<X, ToSet<B>>);
                AssertNotSameProof(Proof1, Proof2);
                AssertSameProof(Proof1, Proof1b);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_IsForestEqualTo(self):
        source = '''
            int main() {
                AssertSameForest(Vector<>, Vector<>);
                AssertNotSameForest(Vector<Proof1>, Vector<Proof2>);
                AssertSameForest(Vector<Proof1, Proof2>, Vector<Proof2, Proof1b>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
