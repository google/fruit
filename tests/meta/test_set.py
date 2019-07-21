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
    #include <fruit/impl/meta/set.h>
    #include <fruit/impl/meta/metaprogramming.h>
    
    struct A1 {};
    struct B1 {};
    struct C1 {};
    
    using A = Type<A1>;
    using B = Type<B1>;
    using C = Type<C1>;
    
    struct Square {
        template <typename N>
        struct apply {
            using type = Int<N::value * N::value>;
        };
    };
    '''

class TestSet(parameterized.TestCase):
    def test_EmptySet(self):
        source = '''
            int main() {
                AssertNot(IsInSet(A, EmptySet));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_ToSet1(self):
        source = '''
            int main() {
                Assert(IsInSet(A, ToSet1<A>));
                AssertNot(IsInSet(A, ToSet1<B>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_ToSet2(self):
        source = '''
            int main() {
                Assert(IsInSet(A, ToSet2<A, B>));
                Assert(IsInSet(B, ToSet2<A, B>));
                AssertNot(IsInSet(C, ToSet2<A, B>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_IsSameSet(self):
        source = '''
            int main() {
                AssertSameSet(EmptySet, EmptySet);
                AssertSameSet(ToSet<A, B>, ToSet<B, A>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_FoldSet(self):
        source = '''
            int main() {
                AssertSameType(Id<FoldSet(ToSet<>, Sum, Int<3>)>, Int<3>);
                AssertSameType(Id<FoldSet(ToSet<Int<2>>, Sum, Int<3>)>, Int<5>);
                AssertSameType(Id<FoldSet(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>, Sum, Int<7>)>, Int<39>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_FoldSetWithCombine(self):
        source = '''
            int main() {
                AssertSameType(Id<FoldSetWithCombine(ToSet<>, Square, Sum, Int<0>)>, Int<0>);
                AssertSameType(Id<FoldSetWithCombine(ToSet<Int<2>>, Square, Sum, Int<0>)>, Int<4>);
                AssertSameType(Id<FoldSetWithCombine(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>, Square, Sum, Int<0>)>, Int<288>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_AddToSet(self):
        source = '''
            int main() {
                AssertSameSet(Id<AddToSet(EmptySet, A)>, ToSet<A>);
                AssertSameSet(Id<AddToSet(ToSet<A, B>, A)>, ToSet<A, B>);
                AssertSameSet(Id<AddToSet(ToSet<C, B>, A)>, ToSet<A, C, B>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_TransformSet(self):
        source = '''
            int main() {
                AssertSameSet(Id<TransformSet(ToSet<>, Square)>, ToSet<>);
                AssertSameSet(Id<TransformSet(ToSet<Int<2>>, Square)>, ToSet<Int<4>>);
                AssertSameSet(Id<TransformSet(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>, Square)>, ToSet<Int<9>, Int<4>, Int<25>, Int<81>, Int<169>>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_SetSize(self):
        source = '''
            int main() {
                AssertSameType(Id<SetSize(ToSet<>)>, Int<0>);
                AssertSameType(Id<SetSize(ToSet<Int<2>>)>, Int<1>);
                AssertSameType(Id<SetSize(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>)>, Int<5>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_IsEmptySet(self):
        source = '''
            int main() {
                Assert(IsEmptySet(ToSet<>));
                AssertNot(IsEmptySet(ToSet<Int<2>>));
                AssertNot(IsEmptySet(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_SetDifference(self):
        source = '''
            int main() {
                AssertSameSet(Id<SetDifference(ToSet<>, ToSet<>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<>, ToSet<A>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<>, ToSet<A, B>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<>, ToSet<A, B, C>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<A>, ToSet<>)>, ToSet<A>);
                AssertSameSet(Id<SetDifference(ToSet<A>, ToSet<A>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<A>, ToSet<A, B>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<A>, ToSet<A, B, C>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<B>, ToSet<>)>, ToSet<B>);
                AssertSameSet(Id<SetDifference(ToSet<B>, ToSet<A>)>, ToSet<B>);
                AssertSameSet(Id<SetDifference(ToSet<B>, ToSet<A, B>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<B>, ToSet<A, B, C>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<B, C>, ToSet<>)>, ToSet<B, C>);
                AssertSameSet(Id<SetDifference(ToSet<B, C>, ToSet<A>)>, ToSet<B, C>);
                AssertSameSet(Id<SetDifference(ToSet<B, C>, ToSet<A, B>)>, ToSet<C>);
                AssertSameSet(Id<SetDifference(ToSet<B, C>, ToSet<A, B, C>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<A, B>, ToSet<A, B>)>, ToSet<>);
                AssertSameSet(Id<SetDifference(ToSet<A>, ToSet<A, B>)>, EmptySet);
                AssertSameSet(Id<SetDifference(ToSet<A, B, C>, ToSet<A>)>, ToSet<B, C>);
                AssertSameSet(Id<SetDifference(ToSet<A, B, C>, ToSet<B>)>, ToSet<A, C>);
                AssertSameSet(Id<SetDifference(ToSet<A, B, C>, ToSet<C>)>, ToSet<A, B>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_SetIntersection(self):
        source = '''
            int main() {
                AssertSameSet(Id<SetIntersection(ToSet<A, B>, ToSet<A, B>)>, ToSet<A, B>);
                AssertSameSet(Id<SetIntersection(ToSet<A>, ToSet<A, B>)>, ToSet<A>);
                AssertSameSet(Id<SetIntersection(ToSet<A, B>, ToSet<A>)>, ToSet<A>);
                AssertSameSet(Id<SetIntersection(ToSet<A>, ToSet<B>)>, ToSet<>);
                AssertSameSet(Id<SetIntersection(ToSet<>, ToSet<A, B>)>, ToSet<>);
                AssertSameSet(Id<SetIntersection(ToSet<C>, ToSet<A, B>)>, ToSet<>);
                AssertSameSet(Id<SetIntersection(ToSet<A, B>, ToSet<>)>, ToSet<>);
                AssertSameSet(Id<SetIntersection(ToSet<A, B>, ToSet<C>)>, ToSet<>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_IsContained(self):
        source = '''
            int main() {
                Assert(IsContained(ToSet<>, ToSet<>));
                Assert(IsContained(ToSet<>, ToSet<A>));
                Assert(IsContained(ToSet<A, B>, ToSet<A, B>));
                Assert(IsContained(ToSet<A, B>, ToSet<B, A>));
                Assert(IsContained(ToSet<A>, ToSet<A, B, C>));
                Assert(IsContained(ToSet<B>, ToSet<A, B, C>));
                Assert(IsContained(ToSet<C>, ToSet<A, B, C>));
                AssertNot(IsContained(ToSet<A, B, C>, ToSet<A, B>));
                AssertNot(IsContained(ToSet<A, B, C>, ToSet<A, C>));
                AssertNot(IsContained(ToSet<A, B, C>, ToSet<B, C>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_IsDisjoint(self):
        source = '''
            int main() {
                AssertNot(IsDisjoint(ToSet<A, B>, ToSet<A, B>));
                AssertNot(IsDisjoint(ToSet<A>, ToSet<A, B>));
                AssertNot(IsDisjoint(ToSet<A, B>, ToSet<A>));
                Assert(IsDisjoint(ToSet<A>, ToSet<B>));
                Assert(IsDisjoint(ToSet<>, ToSet<A, B>));
                Assert(IsDisjoint(ToSet<C>, ToSet<A, B>));
                Assert(IsDisjoint(ToSet<A, B>, ToSet<>));
                Assert(IsDisjoint(ToSet<A, B>, ToSet<C>));
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

    def test_SetUnion(self):
        source = '''
            int main() {
                AssertSameSet(Id<SetUnion(ToSet<A, B>, ToSet<A, B>)>, ToSet<A, B>);
                AssertSameSet(Id<SetUnion(ToSet<A>, ToSet<A, B>)>, ToSet<A, B>);
                AssertSameSet(Id<SetUnion(ToSet<A, B>, ToSet<A>)>, ToSet<A, B>);
                AssertSameSet(Id<SetUnion(ToSet<A>, ToSet<B>)>, ToSet<A, B>);
                AssertSameSet(Id<SetUnion(ToSet<>, ToSet<A, B>)>, ToSet<A, B>);
                AssertSameSet(Id<SetUnion(ToSet<C>, ToSet<A, B>)>, ToSet<A, B, C>);
                AssertSameSet(Id<SetUnion(ToSet<A, B>, ToSet<>)>, ToSet<A, B>);
                AssertSameSet(Id<SetUnion(ToSet<A, B>, ToSet<C>)>, ToSet<A, B, C>);
            }
            '''
        expect_success(
            COMMON_DEFINITIONS,
            source,
            locals())

if __name__ == '__main__':
    absltest.main()
