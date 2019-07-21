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
    #include <fruit/impl/meta/vector.h>
    #include <fruit/impl/meta/metaprogramming.h>
    
    struct A1 {};
    struct B1 {};
    struct C1 {};
    
    using A = A1;
    using B = B1;
    using C = C1;
    '''

class TestVector(parameterized.TestCase):
    def test_IsInVector(self):
        source = '''
            int main() {
                AssertNot(IsInVector(A, Vector<>));
                AssertNot(IsInVector(A, Vector<B>));
                Assert(IsInVector(A, Vector<A>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_IsSameVector(self):
        source = '''
            int main() {
                AssertNotSameType(Vector<A, B>, Vector<B, A>);
                AssertNotSameType(Vector<A>, Vector<>);
                AssertNotSameType(Vector<>, Vector<A>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_VectorSize(self):
        source = '''
            int main() {
                AssertSameType(Id<VectorSize(Vector<>)>, Int<0>);
                AssertSameType(Id<VectorSize(Vector<A>)>, Int<1>);
                AssertSameType(Id<VectorSize(Vector<A, B>)>, Int<2>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_ConcatVectors(self):
        source = '''
            int main() {
                AssertSameType(Id<ConcatVectors(Vector<>, Vector<>)>, Vector<>);
                AssertSameType(Id<ConcatVectors(Vector<>, Vector<A, B>)>, Vector<A, B>);
                AssertSameType(Id<ConcatVectors(Vector<A, B>, Vector<>)>, Vector<A, B>);
                AssertSameType(Id<ConcatVectors(Vector<A>, Vector<A, B>)>, Vector<A, A, B>);
                AssertSameType(Id<ConcatVectors(Vector<A, B>, Vector<A, C>)>, Vector<A, B, A, C>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_VectorEndsWith(self):
        source = '''
            int main() {
                Assert(VectorEndsWith(Vector<A, B>, B));
                AssertNot(VectorEndsWith(Vector<A, B>, A));
                AssertNot(VectorEndsWith(Vector<>, A));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_VectorRemoveFirstN(self):
        source = '''
            int main() {
                AssertSameType(Id<VectorRemoveFirstN(Vector<>, Int<0>)>, Vector<>);
                AssertSameType(Id<VectorRemoveFirstN(Vector<A>, Int<0>)>, Vector<A>);
                AssertSameType(Id<VectorRemoveFirstN(Vector<A>, Int<1>)>, Vector<>);
                AssertSameType(Id<VectorRemoveFirstN(Vector<A, B, C>, Int<0>)>, Vector<A, B, C>);
                AssertSameType(Id<VectorRemoveFirstN(Vector<A, B, C>, Int<1>)>, Vector<B, C>);
                AssertSameType(Id<VectorRemoveFirstN(Vector<A, B, C>, Int<2>)>, Vector<C>);
                AssertSameType(Id<VectorRemoveFirstN(Vector<A, B, C>, Int<3>)>, Vector<>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
