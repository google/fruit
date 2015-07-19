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
#include <fruit/impl/meta/set.h>
#include <fruit/impl/meta/metaprogramming.h>

struct A1 {};
struct B1 {};
struct C1 {};

using A = Type<A1>;
using B = Type<B1>;
using C = Type<C1>;

void test_EmptySet() {
  AssertNot(IsInSet(A, EmptySet));
}

void test_ToSet1() {
  Assert(IsInSet(A, ToSet1<A>));
  AssertNot(IsInSet(A, ToSet1<B>));
}

void test_ToSet2() {
  Assert(IsInSet(A, ToSet2<A, B>));
  Assert(IsInSet(B, ToSet2<A, B>));
  AssertNot(IsInSet(C, ToSet2<A, B>));
}

void test_IsSameSet() {
  AssertSameSet(EmptySet, EmptySet);
  AssertSameSet(ToSet<A, B>, ToSet<B, A>);
}

void test_FoldSet() {
  AssertSameType(FoldSet(ToSet<>, Sum, Int<3>), Int<3>);
  AssertSameType(FoldSet(ToSet<Int<2>>, Sum, Int<3>), Int<5>);
  AssertSameType(FoldSet(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>, Sum, Int<7>), Int<39>);
}

struct Square {
  template <typename N>
  struct apply {
    using type = Int<N::value * N::value>;
  };
};

void test_FoldSetWithCombine() {
  AssertSameType(FoldSetWithCombine(ToSet<>, Square, Sum, Int<0>), Int<0>);
  AssertSameType(FoldSetWithCombine(ToSet<Int<2>>, Square, Sum, Int<0>), Int<4>);
  AssertSameType(FoldSetWithCombine(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>, Square, Sum, Int<0>), Int<288>);
}

void test_AddToSet() {
  AssertSameSet(AddToSet(EmptySet, A), ToSet<A>);
  AssertSameSet(AddToSet(ToSet<A, B>, A), ToSet<A, B>);
  AssertSameSet(AddToSet(ToSet<C, B>, A), ToSet<A, C, B>);
}

void test_TransformSet() {
  AssertSameSet(TransformSet(ToSet<>, Square), ToSet<>);
  AssertSameSet(TransformSet(ToSet<Int<2>>, Square), ToSet<Int<4>>);
  AssertSameSet(TransformSet(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>, Square), ToSet<Int<9>, Int<4>, Int<25>, Int<81>, Int<169>>);
}

void test_SetSize() {
  AssertSameType(SetSize(ToSet<>), Int<0>);
  AssertSameType(SetSize(ToSet<Int<2>>), Int<1>);
  AssertSameType(SetSize(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>), Int<5>);
}

void test_IsEmptySet() {
  Assert(IsEmptySet(ToSet<>));
  AssertNot(IsEmptySet(ToSet<Int<2>>));
  AssertNot(IsEmptySet(ToSet<Int<3>, Int<2>, Int<5>, Int<9>, Int<13>>));
}

void test_SetDifference() {
  AssertSameSet(SetDifference(ToSet<>, ToSet<>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<>, ToSet<A>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<>, ToSet<A, B>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<>, ToSet<A, B, C>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<A>, ToSet<>), ToSet<A>);
  AssertSameSet(SetDifference(ToSet<A>, ToSet<A>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<A>, ToSet<A, B>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<A>, ToSet<A, B, C>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<B>, ToSet<>), ToSet<B>);
  AssertSameSet(SetDifference(ToSet<B>, ToSet<A>), ToSet<B>);
  AssertSameSet(SetDifference(ToSet<B>, ToSet<A, B>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<B>, ToSet<A, B, C>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<B, C>, ToSet<>), ToSet<B, C>);
  AssertSameSet(SetDifference(ToSet<B, C>, ToSet<A>), ToSet<B, C>);
  AssertSameSet(SetDifference(ToSet<B, C>, ToSet<A, B>), ToSet<C>);
  AssertSameSet(SetDifference(ToSet<B, C>, ToSet<A, B, C>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<A, B>, ToSet<A, B>), ToSet<>);
  AssertSameSet(SetDifference(ToSet<A>, ToSet<A, B>), EmptySet);
  AssertSameSet(SetDifference(ToSet<A, B, C>, ToSet<A>), ToSet<B, C>);
  AssertSameSet(SetDifference(ToSet<A, B, C>, ToSet<B>), ToSet<A, C>);
  AssertSameSet(SetDifference(ToSet<A, B, C>, ToSet<C>), ToSet<A, B>);
}

void test_SetIntersection() {
  AssertSameSet(SetIntersection(ToSet<A, B>, ToSet<A, B>), ToSet<A, B>);
  AssertSameSet(SetIntersection(ToSet<A>, ToSet<A, B>), ToSet<A>);
  AssertSameSet(SetIntersection(ToSet<A, B>, ToSet<A>), ToSet<A>);
  AssertSameSet(SetIntersection(ToSet<A>, ToSet<B>), ToSet<>);
  AssertSameSet(SetIntersection(ToSet<>, ToSet<A, B>), ToSet<>);
  AssertSameSet(SetIntersection(ToSet<C>, ToSet<A, B>), ToSet<>);
  AssertSameSet(SetIntersection(ToSet<A, B>, ToSet<>), ToSet<>);
  AssertSameSet(SetIntersection(ToSet<A, B>, ToSet<C>), ToSet<>);
}

void test_IsContained() {
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

void test_IsDisjoint() {
  AssertNot(IsDisjoint(ToSet<A, B>, ToSet<A, B>));
  AssertNot(IsDisjoint(ToSet<A>, ToSet<A, B>));
  AssertNot(IsDisjoint(ToSet<A, B>, ToSet<A>));
  Assert(IsDisjoint(ToSet<A>, ToSet<B>));
  Assert(IsDisjoint(ToSet<>, ToSet<A, B>));
  Assert(IsDisjoint(ToSet<C>, ToSet<A, B>));
  Assert(IsDisjoint(ToSet<A, B>, ToSet<>));
  Assert(IsDisjoint(ToSet<A, B>, ToSet<C>));
}


void test_SetUnion() {
  AssertSameSet(SetUnion(ToSet<A, B>, ToSet<A, B>), ToSet<A, B>);
  AssertSameSet(SetUnion(ToSet<A>, ToSet<A, B>), ToSet<A, B>);
  AssertSameSet(SetUnion(ToSet<A, B>, ToSet<A>), ToSet<A, B>);
  AssertSameSet(SetUnion(ToSet<A>, ToSet<B>), ToSet<A, B>);
  AssertSameSet(SetUnion(ToSet<>, ToSet<A, B>), ToSet<A, B>);
  AssertSameSet(SetUnion(ToSet<C>, ToSet<A, B>), ToSet<A, B, C>);
  AssertSameSet(SetUnion(ToSet<A, B>, ToSet<>), ToSet<A, B>);
  AssertSameSet(SetUnion(ToSet<A, B>, ToSet<C>), ToSet<A, B, C>);
}

int main() {
  
  test_EmptySet();
  test_ToSet1();
  test_ToSet2();
  test_IsSameSet();
  test_FoldSet();
  test_FoldSetWithCombine();
  test_AddToSet();
  test_TransformSet();
  test_SetSize();
  test_IsEmptySet();
  test_SetDifference();
  test_SetIntersection();
  test_IsContained();
  test_IsDisjoint();
  test_SetUnion();
  
  return 0;
}


